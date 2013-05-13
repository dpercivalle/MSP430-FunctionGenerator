/**
  * CPE 329 - 07/08
  * Spring 2013
  *
  * Project 2 - Function Generator
  * Engineers:  Donny Percivalle
  *             Alex Lin
  **/

#include "MSP430_FunctionGenerator.h"
/////////////////////////
//
// main()
//
/////////////////////////
int main(void){
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

   //
   // Calibrate clock for 16Mhz
   //
   if (CALBC1_16MHZ == 0XFF){
      while(1);
   }
   DCOCTL = 0;
   BCSCTL1 = CALBC1_16MHZ;
   DCOCTL = CALDCO_16MHZ;

   //
   // Configure a timer with interrupts to drive the 20ms period square wave
   //
   TACTL = TASSEL_2 + MC_2 + ID_2;     // Set timer to count SMCLK
                                       // Set mode to count continuous to CCR0
                                       // Divide the 16MHz clock by 1

   CCTL0 = CCIE;                       // Enable timer interrupts
   CCR0 = TIMER_FREQ;

   //
   // Configure ports for SPI on USIB
   //
   P1DIR |= BIT4;
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
   // Infinite loop, update DAC value
   //
   _enable_interrupt();
   while (1) {
      setDACOutput(DAC_VALUE);
   }
   return 0;
}

/////////////////////////
//
// Function definitions
//
/////////////////////////

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

   P1OUT &= ~BIT4;                               // Software driven /CS line on
                                                // P1.4 driven low for SPI
                                                // communication

   UCB0TXBUF = (DAC_word >> 8);                 // Shift upper byte of DAC_word
                                                // 8-bits to the right

   while (!(IFG2 & UCB0TXIFG));                 // Wait for first byte to be
                                                // sent

   UCB0TXBUF = (unsigned char)(DAC_word & 0x00FF);
                                                // Transmit lower byte

   while (!(IFG2 & UCB0TXIFG));
   __delay_cycles(200);                         // Wait for transfer to finish
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
// Timer A ISR
//
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerAInterrupt(){
   static volatile int toggle = 0;
   //
   // Square wave interrupt
   //
   if (WAVE == SQUARE){
      if (WAVE_DUTY){
         DAC_VALUE = SQUARE_SAW_HIGH;
         WAVE_DUTY--;
      }
      else{
         DAC_VALUE = SQUARE_SAW_LOW;
         WAVE_DUTY = WAVE_DUTY_SET;
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
      if (i == 180){
         i = 0;
      }
      DAC_VALUE = SIN_LUT[(i++)];

      // SIN wave logic
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
   __delay_cycles(400000);
   //
   // Wave select
   //
   if (P2IFG & BIT2){
      DAC_VALUE = SQUARE_SAW_LOW;
      WAVE = ((WAVE+1) % 3);
      P2IFG &= ~BIT2;
      //
      // Sawtooth wave set, 100Hz
      //
      if (WAVE == SAWTOOTH){
         TACTL = TASSEL_2 + MC_2;
         TIMER_FREQ = 525;
      }
      //
      // Square wave set, 100Hz, 50%
      //
      else if (WAVE == SQUARE){
         square_count = 0;
         TACTL = TASSEL_2 + MC_2 + ID_2;
         TIMER_FREQ = 20000;
      }
      //
      // Sin wave set, 100Hz
      //
      else if (WAVE == SIN){
         // SIN WAVE TIMER_FREQ SET
         TACTL = TASSEL_2 + MC_2;
         TIMER_FREQ = 875;
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
         if (square_count == 4){
            square_count = 0;
         }
         else{
            square_count++;
         }
         TIMER_FREQ = SQ_FREQ[square_count];
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
         if (sin_count == 4){
            sin_count = 0;
         }
         else{
            sin_count++;
         }
         TIMER_FREQ = SIN_FREQ[sin_count];
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
   }
   //
   // Something weird happened
   //
   else{
      P2IFG &= ~0xFF;
   }
   _enable_interrupt();
}
