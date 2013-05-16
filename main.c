/**
  * CPE 329 - 07/08
  * Spring 2013
  *
  * Project 2 - Function Generator
  * Engineers:  Donny Percivalle
  *             Alex Lin
  **/
#include "MSP430_FunctionGenerator.h"
/////////////////////////////////
//
// main()
//
/////////////////////////////////
void main(void){
   WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
   // Calibrate Timer A
   TACTL = TASSEL_2 + MC_2 + ID_2;     // Set timer to count SMCLK
                                       // Set mode to count continuous to CCR0
                                       // Divide the 16MHz clock by 1 = 16Mhz
   CCTL0 = CCIE;                       // Enable timer interrupts
   CCR0 = 20000;
   //
   // Configure ports for 3-wire SPI on USCIB
   //
   P1DIR |= BIT4 | BIT0 | BIT6;
   P1OUT &= ~BIT6;
   P1OUT |= BIT0;
   P1SEL = BIT7 | BIT5;
   P1SEL2 = BIT7 | BIT5;
   P2IE = BIT0 | BIT1 | BIT2;
   P2IES = BIT0 | BIT1 | BIT2;
   P2IFG &= (~BIT0 | ~BIT1 | ~BIT2);
   //
   // Configure SPI on USIB
   // Clock inactive state = low, MSP first, 8-bit SPI
   // master, 4-pin active low STE, synchronous, SMCLK driven
   //
   UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC;
   UCB0CTL1 |= UCSSEL_2;
   //
   // Configue UCB cock to use SMCLK at 16Mhz
   //
   UCB0BR0 |= 0x00;    // (low divider for byte)
   UCB0BR1 |= 0x00;    // (high divider for byte)
   //
   // Initialize the USCI state machine, starting the SPI
   //
   UCB0CTL1 &= ~UCSWRST;
   //
   // Run DCO at 16Mhz
   //
   DCOCTL = 0;
   BCSCTL1 = CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;
   //
   // Enable interrupts, enter infinite loop
   // that continuously updates DAC configuration
   //
   _enable_interrupt();
   while (1) {
     setDACOutput(DAC_VALUE);
   }
}  
/////////////////////////////////
//
// Function definitions
//
/////////////////////////////////
//
// SPI output to DAC
//
void setDACOutput (unsigned int level){
   _disable_interrupt();
   unsigned int DAC_word = 0;
   DAC_word = (0x1000) | (level & 0x0FFF);      // 0x1000 sets DAC for a write
                                                // instruction, gain = 2,
                                                // /SHDN = 1, level = 12 bit
                                                // BCD value for voltage config.
   P1OUT &= ~BIT4;                              // Software driven /CS line on
                                                // P1.4 driven low for SPI
                                                // communication
   UCB0TXBUF = (DAC_word >> 8);                 // Shift upper byte of DAC_word
                                                // 8-bits to the right
   while (!(IFG2 & UCB0TXIFG));                 // Wait for first byte to be
                                                // sent
   UCB0TXBUF = (unsigned char)(DAC_word & 0x00FF);
                                                // Transmit lower byte
   while (!(IFG2 & UCB0TXIFG));
   __delay_cycles(16);                          // Wait for transfer to finish
                                                // before driving /CS high and
                                                // finishing communication
   P1OUT |= BIT4;
   _enable_interrupt();
   return;
}
/////////////////////////////////
//
// Interrupt vectors
//
/////////////////////////////////
//
// Timer A ISR, update value being output to DAC
//
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerAInterrupt(){
   static volatile int toggle = 0;
   //
   // Square wave interrupt
   //
   if (WAVE == SQUARE){
      if (DAC_VALUE == SQUARE_SAW_HIGH){
         DAC_VALUE = SQUARE_SAW_LOW;
         TIMER_FREQ = WAVE_DUTY_LOW;
      }
      else{
         DAC_VALUE = SQUARE_SAW_HIGH;
         TIMER_FREQ = WAVE_DUTY_HIGH;
      }
   }
   //
   // Sawtooth wave interrupt
   //
   else if (WAVE == SAWTOOTH){
      if (DAC_VALUE == SQUARE_SAW_HIGH){
            toggle = -1;
         }
      if (toggle == -1){
         DAC_VALUE = SQUARE_SAW_LOW;
         toggle = 0;
      }
      else{
         DAC_VALUE+= SAW_STEP;
      }
   }
   //
   // Sin wave interrupt
   //
    else if (WAVE == SIN){
       static volatile int i = 0;
       if (i+SIN_STEP > 179){
          i = 0;
       }
       DAC_VALUE = SIN_LUT[(i+=SIN_STEP)];
    }
   //
   // Exponential Pulse wave interrupt
   //
   else if (WAVE == EXP_PULSE){
      if (((exp_index + 1) > 108  && !exp_toggle) || ((exp_index - 1) < 0 && exp_toggle)){
         exp_toggle = !exp_toggle;
         if (!exp_toggle){
            exp_hold = exp_hold_time;
         }
      }
      if (!exp_toggle){
         DAC_VALUE = EXP_LUT[exp_index++];         
      }
      else if (exp_hold > 0){
         exp_hold --;
      }
      else{
         DAC_VALUE = EXP_LUT[exp_index--];
      }
   }
   CCR0 += TIMER_FREQ;
}
//
// Port 2 ISR, Waveform select, Frequency select, duty cycle
// buttons
//
#pragma vector = PORT2_VECTOR
__interrupt void Port2Interrupt(){
   _disable_interrupt();
   __delay_cycles(4000000);
   //
   // Wave select
   //
   if (P2IFG & BIT2){
      DAC_VALUE = SQUARE_SAW_LOW;
      WAVE++;
      if (WAVE == 5) {WAVE = SQUARE;}
      P2IFG &= ~BIT2;
      //
      // Sawtooth wave set, 100Hz
      //
      if (WAVE == SAWTOOTH){
         DCOCTL = 0;
         BCSCTL1 = CALBC1_16MHZ;
         DCOCTL = CALDCO_16MHZ;
         P1OUT &= ~(BIT0 | BIT6);
         P1OUT |= (BIT0 | BIT6);
         TACTL = TASSEL_2 + MC_2;
         TIMER_FREQ = SAW_FREQ;
      }
      //
      // Square wave set, 100Hz, 50% duty
      //
      else if (WAVE == SQUARE){
         DCOCTL = 0;
         BCSCTL1 = CALBC1_16MHZ;
         DCOCTL = CALDCO_16MHZ;
         P1OUT &= ~(BIT0 | BIT6);
         P1OUT |= BIT0;
         TIMER_FREQ = WAVE_DUTY_HIGH = WAVE_DUTY_LOW = 20000;
         duty = 50;
         freq = 100;
         TACTL = TASSEL_2 + MC_2 + ID_2;
      }
      //
      // Sin wave set, 100Hz, either the sinwave functionality or 
      // the exponential pulse functionalit, including LUTs,
      // has to be omitted in order for program to fit on MSP430
      //
      else if (WAVE == SIN){
         // SIN WAVE TIMER_FREQ SET
         DCOCTL = 0;
         BCSCTL1 = CALBC1_16MHZ;
         DCOCTL = CALDCO_16MHZ;

         P1OUT &= ~(BIT0 | BIT6);
         P1OUT |= BIT6;
         TACTL = TASSEL_2 + MC_2;
         SIN_STEP = 1;
         TIMER_FREQ = SIN_FREQ;
      }
      //
      //Exponential pulse set
      //
      else if (WAVE == EXP_PULSE){
         DCOCTL = 0;
         BCSCTL1 = CALBC1_16MHZ;
         DCOCTL = CALDCO_16MHZ;
         exp_toggle = 0;
         exp_index = 0;
         exp_hold = exp_hold_time = 50;
         P1OUT &= ~(BIT0 | BIT6);
         TACTL = TASSEL_2 + MC_2 + ID_3;
         TIMER_FREQ = 60000;
      }
   }
   //
   // Frequency select
   //
   else if (P2IFG & BIT1){
      P2IFG &= ~BIT1;
      //
      // Square wave frequency select
      //
      if (WAVE == SQUARE){
         if (freq == 500){
            freq = 100;
         }
         else{
            freq += 100;
         }
         duty = 50;
         if (freq == 100){
            WAVE_DUTY_HIGH = WAVE_DUTY_LOW = 20000;
         }
         else if (freq == 200){
            WAVE_DUTY_HIGH = WAVE_DUTY_LOW = 10000;
         }
         else if (freq == 300){
            WAVE_DUTY_HIGH = 6667;
            WAVE_DUTY_LOW = 6666;
         }
         else if (freq == 400){
            WAVE_DUTY_HIGH = WAVE_DUTY_LOW = 5000;
         }
         else if (freq == 500){
            WAVE_DUTY_HIGH = WAVE_DUTY_LOW = 4000;
         }

      }
      //
      // Sawtooth wave frequency select
      //
      else if (WAVE == SAWTOOTH){
         if (SAW_STEP == 50){
            DAC_VALUE = SQUARE_SAW_LOW;
            SAW_STEP = 10;
         }
         else {
            DAC_VALUE = SQUARE_SAW_LOW;
            SAW_STEP += 10;
         }
      }
      //
      // Sin wave frequency select
      //
      else{ // WAVE == SIN
         if (SIN_STEP ==  5){
            SIN_STEP = 1;
         }
         else{
            SIN_STEP+=1;
         }
      }
   }
   //
   // Duty cycle select
   //
   else if (P2IFG & BIT0){
      P2IFG &= ~BIT0;
      if (WAVE != SQUARE){
         return;
      }
      else{
         // 100 Hz Frequency Duty
         if (freq == 100){            
            if (duty == 10){
               WAVE_DUTY_HIGH = 4000;
               WAVE_DUTY_LOW = 36000;
               duty += 10;
            }
            else if (duty == 20){
               WAVE_DUTY_HIGH = 8000;
               WAVE_DUTY_LOW = 32000;
               duty += 10;
            }
            else if (duty == 30){
               WAVE_DUTY_HIGH = 12000;
               WAVE_DUTY_LOW = 28000;
               duty += 10;
            }
            else if (duty == 40){
               WAVE_DUTY_HIGH = 16000;
               WAVE_DUTY_LOW = 24000;
               duty += 10;
            }
            else if (duty == 50){
               WAVE_DUTY_HIGH = 20000;
               WAVE_DUTY_LOW = 20000;
               duty += 10;
            }
            else if (duty == 60){
               WAVE_DUTY_HIGH = 24000;
               WAVE_DUTY_LOW = 16000;
               duty += 10;
            }
            else if (duty == 70){
               WAVE_DUTY_HIGH = 27999;
               WAVE_DUTY_LOW = 12000;
               duty += 10;
            }
            else if (duty == 80){
               WAVE_DUTY_HIGH = 32000;
               WAVE_DUTY_LOW = 8000;
               duty += 10;
            }
            else if (duty == 90){
               WAVE_DUTY_HIGH = 36000;
               WAVE_DUTY_LOW = 4000;
               duty = 10;
            }
         }
         // 200 Hz Frequency Duty
         else if (freq == 200){
            if (duty == 10){
               WAVE_DUTY_HIGH = 2000;
               WAVE_DUTY_LOW = 18000;
               duty += 10;
            }
            else if (duty == 20){
               WAVE_DUTY_HIGH = 4000;
               WAVE_DUTY_LOW = 16000;
               duty += 10;
            }
            else if (duty == 30){
               WAVE_DUTY_HIGH = 6000;
               WAVE_DUTY_LOW = 14000;
               duty += 10;
            }
            else if (duty == 40){
               WAVE_DUTY_HIGH = 8000;
               WAVE_DUTY_LOW = 12000;
               duty += 10;
            }
            else if (duty == 50){
               WAVE_DUTY_HIGH = 10000;
               WAVE_DUTY_LOW = 10000;
               duty += 10;
            }
            else if (duty == 60){
               WAVE_DUTY_HIGH = 12000;
               WAVE_DUTY_LOW = 8000;
               duty += 10;
            }
            else if (duty == 70){
               WAVE_DUTY_HIGH = 14000;
               WAVE_DUTY_LOW = 6000;
               duty += 10;
            }
            else if (duty == 80){
               WAVE_DUTY_HIGH = 16000;
               WAVE_DUTY_LOW = 4000;
               duty += 10;
            }
            else if (duty == 90){
               WAVE_DUTY_HIGH = 18000;
               WAVE_DUTY_LOW = 2000;
               duty = 10;
            }
         }
         // 300 Hz Frequency Duty
         else if (freq == 300){
            if (duty == 10){
               WAVE_DUTY_HIGH = 1333;
               WAVE_DUTY_LOW = 12000;
               duty += 10;
            }
            else if (duty == 20){
               WAVE_DUTY_HIGH = 2667;
               WAVE_DUTY_LOW = 10666;
               duty += 10;
            }
            else if (duty == 30){
               WAVE_DUTY_HIGH = 4000;
               WAVE_DUTY_LOW = 9333;
               duty += 10;
            }
            else if (duty == 40){
               WAVE_DUTY_HIGH = 5333;
               WAVE_DUTY_LOW = 8000;
               duty += 10;
            }
            else if (duty == 50){
               WAVE_DUTY_HIGH = 6667;
               WAVE_DUTY_LOW = 6666;
               duty += 10;
            }
            else if (duty == 60){
               WAVE_DUTY_HIGH = 8000;
               WAVE_DUTY_LOW = 5333;
               duty += 10;
            }
            else if (duty == 70){
               WAVE_DUTY_HIGH = 9333;
               WAVE_DUTY_LOW = 4000;
               duty += 10;
            }
            else if (duty == 80){
               WAVE_DUTY_HIGH = 10666;
               WAVE_DUTY_LOW = 2667;
               duty += 10;
            }
            else if (duty == 90){
               WAVE_DUTY_HIGH = 12000;
               WAVE_DUTY_LOW = 1333;
               duty = 10;
            }
         }
         // 400 Hz Frequency Duty
         else if (freq == 400){
            if (duty == 10){
               WAVE_DUTY_HIGH = 1000;
               WAVE_DUTY_LOW = 9000;
               duty += 10;
            }
            else if (duty == 20){
               WAVE_DUTY_HIGH = 2000;
               WAVE_DUTY_LOW = 8000;
               duty += 10;
            }
            else if (duty == 30){
               WAVE_DUTY_HIGH = 3000;
               WAVE_DUTY_LOW = 7000;
               duty += 10;
            }
            else if (duty == 40){
               WAVE_DUTY_HIGH = 4000;
               WAVE_DUTY_LOW = 6000;
               duty += 10;
            }
            else if (duty == 50){
               WAVE_DUTY_HIGH = 5000;
               WAVE_DUTY_LOW = 5000;
               duty += 10;
            }
            else if (duty == 60){
               WAVE_DUTY_HIGH = 6000;
               WAVE_DUTY_LOW = 4000;
               duty += 10;
            }
            else if (duty == 70){
               WAVE_DUTY_HIGH = 7000;
               WAVE_DUTY_LOW = 3000;
               duty += 10;
            }
            else if (duty == 80){
               WAVE_DUTY_HIGH = 8000;
               WAVE_DUTY_LOW = 2000;
               duty += 10;
            }
            else if (duty == 90){
               WAVE_DUTY_HIGH = 9000;
               WAVE_DUTY_LOW = 1000;
               duty = 10;
            }
         }
         // 500 Hz Frequency Duty
         else if (freq == 500){
            if (duty == 10){
               WAVE_DUTY_HIGH = 800;
               WAVE_DUTY_LOW = 7200;
               duty += 10;
            }
            else if (duty == 20){
               WAVE_DUTY_HIGH = 1600;
               WAVE_DUTY_LOW = 6400;
               duty += 10;
            }
            else if (duty == 30){
               WAVE_DUTY_HIGH = 2400;
               WAVE_DUTY_LOW = 5600;
               duty += 10;
            }
            else if (duty == 40){
               WAVE_DUTY_HIGH = 3200;
               WAVE_DUTY_LOW = 4800;
               duty += 10;
            }
            else if (duty == 50){
               WAVE_DUTY_HIGH = 4000;
               WAVE_DUTY_LOW = 4000;
               duty += 10;
            }
            else if (duty == 60){
               WAVE_DUTY_HIGH = 4800;
               WAVE_DUTY_LOW = 3200;
               duty += 10;
            }
            else if (duty == 70){
               WAVE_DUTY_HIGH = 5600;
               WAVE_DUTY_LOW = 2400;
               duty += 10;
            }
            else if (duty == 80){
               WAVE_DUTY_HIGH = 6400;
               WAVE_DUTY_LOW = 1600;
               duty += 10;
            }
            else if (duty == 90){
               WAVE_DUTY_HIGH = 7200;
               WAVE_DUTY_LOW = 800;
               duty = 10;
            }
         }
      }
   }
   //
   // Something weird happened
   //
   else{
      P2IFG &= ~0xFF;
   }
   _enable_interrupt();
}
//////////////////////////////////////
//
// Trap all other interrupt vectors
//
//////////////////////////////////////
#pragma vector = PORT1_VECTOR
__interrupt void port1ISR(){while(1);}
#pragma vector = ADC10_VECTOR
__interrupt void adcISR(){while(1);}
#pragma vector = COMPARATORA_VECTOR
__interrupt void compAISR(){while(1);}
#pragma vector = NMI_VECTOR
__interrupt void nmiISR(){while(1);}
#pragma vector = TIMER0_A1_VECTOR
__interrupt void timerA01ISR(){while(1);}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void timer1A0ISR(){while(1);}
#pragma vector = TIMER1_A1_VECTOR
__interrupt void timer1A1ISR(){while(1);}
#pragma vector = USCIAB0RX_VECTOR
__interrupt void usciAB0RXISR(){while(1);}
#pragma vector = USCIAB0TX_VECTOR
__interrupt void usciAB0TXISR(){while(1);}
#pragma vector = WDT_VECTOR
__interrupt void wdtISR(){while(1);}
