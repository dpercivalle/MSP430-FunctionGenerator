/*
 * MSP430_lcd.c
 *
 *  Created on: Apr 8, 2013
 *      Author: donaldpercivalle
 */
#include <msp430g2553.h>
#include <MSP430_lcd_nibble.h>

/**
  * void lcd_init()
  *    Initializes the display by writing
  *   the start-up instructions as specified
  *    in the datasheet.  Modified for 8Mhz
  *    operation on 4/29/13.
  *
  * Parameters:
  *    char entryMode: The display mode desired
  *                    as specified by the
  *                    datasheet.
  *
  * Returns:
  *    void
  **/
void lcd_init(char entryMode){
   //
   // Initialize lines low, then set direction of
   // data bits to output in order to write to LCD.
   //
   LCD_DATA_OUT &= ~LCD_DATA_MASK;
   LCD_CTRL_OUT &= ~LCD_CTRL_MASK;
   LCD_DATA_DIR |= LCD_DATA_MASK;
   LCD_CTRL_DIR |= LCD_CTRL_MASK;

   //
   // Wait for LCD to power up since the uController
   // powers up faster than it
   //
   __delay_cycles(320000);
   //
   // Execute start-up routine of as specified
   //
   lcd_writeData(SET_FUNCTION, FALSE);
   __delay_cycles(1600);
   lcd_writeData(DISPLAY_SET, FALSE);
   __delay_cycles(1600);
   lcd_clearDisplay();
   __delay_cycles(1600);
   lcd_writeData(entryMode, FALSE);
   __delay_cycles(1600);
}

void lcd_customCharacterInit(unsigned char *ptr){
   int i;
   for (i = 0; i < 8; i++){
         lcd_writeData(ptr[i], TRUE);
      }
}

/**
  * void lcd_strobe()
  *    Strobes the enable line in order
  *    to signal that data is ready to be
  *    read by the controller and written
  *    to the display.
  *
  * Parameters:
  *    int direction: UP for E line high,
  *                   DOWN for E line low.
  *
  * Returns
  *    void
  **/
void lcd_strobe(int direction){
   //
   //pull EN line high
   //
   if(direction == UP){
      LCD_CTRL_OUT = LCD_PIN_EN;
   }

   //
   //pull EN line back to low
   //
   else{
      //
      // Delay write data setup  time
      //
      __delay_cycles(1600);
      LCD_CTRL_OUT = ~LCD_PIN_EN;
   }
}

/**
  * void lcd_writeData(char data, int isData)
  *    Writes the given data to out to the
  *    display controller.  Also writes
  *    instructions, setting the control
  *    lines appropriately.
  *
  * Parameters:
  *    char data:   The data to be written to
  *                 the display.
  *    int  isData: TRUE if the data is display data.
  *                 FALSE if it is controller
  *                 instructions.
  *
  * Return:
  *    void
  **/
void lcd_writeData(char data, int isData){
   //
   // Clear all data lines
   //
   LCD_DATA_OUT &= ~LCD_DATA_MASK;
   LCD_CTRL_OUT &= ~LCD_CTRL_MASK;

   //
   // Set data lines to outputs in order
   // to write to LCD
   //
   LCD_DATA_DIR |= LCD_DATA_MASK;

   //
   // Set control lines depending on if the
   // data is data or instructions
   //
   if(isData == TRUE){
      LCD_CTRL_OUT |= LCD_PIN_RS;
   }
   else{
      LCD_CTRL_OUT &= ~LCD_PIN_RS;
   }

   //
   // Wait for R/W delay time
   //
   __delay_cycles(1600);

   //
   // Set write strobe up
   //
   lcd_strobe(UP);

   //
   // Set data line voltages to 4 MSBs
   //
   LCD_DATA_MASK |= (data & 0xF0);

   //
   // Strobe down, latching data, strobe up for next 4 bits
   //
   lcd_strobe(DOWN);
   lcd_strobe(UP);

   //
   // Set data line voltages to 4 LSBs
   //
   LCD_DATA &= ~LCD_DATA_MASK;
   LCD_DATA |= ((data & 0x0F) << 4);

   //
   // Strobe down, latching data
   //
   lcd_strobe(DOWN);

   //
   // Delay write data hold time
   //
   __delay_cycles(56000);

   //
   // Clear all pins after the write, leaving
   // the data lines as outputs to preserve health
   // of logic translater circuit (LCD is 5V).
   //
   LCD_DATA_OUT &= (~LCD_DATA_MASK);
   LCD_CTRL_OUT &= (~LCD_CTRL_MASK);
}

/**
  * void lcd_clearDisplay()
  *    Writes the display clear function to the
  *    display, clearing its contents.
  *
  * Parameters:
  *    none
  *
  * Returns:
  *    void
  **/
void lcd_clearDisplay(){
   lcd_writeData(CLEAR_DISPLAY, FALSE);
   return;
}

/**
  * void lcd_goHome()
  *    Returns the cursor to the "home" (top-left)
  *    segment of the display.
  *
  * Parameters:
  *    none
  *
  * Returns:
  *    void
  **/
void lcd_goHome(){
   //
   // Clear all data lines
   //
   LCD_DATA_OUT &= (~LCD_DATA_MASK);
   LCD_CTRL_OUT &= (~LCD_CTRL_MASK);

   //
   // Set data lines to outputs in order
   // to write to LCD
   //
   LCD_DATA_DIR |= LCD_DATA_MASK;

   //
   // Wait for R/W delay time
   //
   __delay_cycles(8600);

   //
   // Set write strobe up
   //
   lcd_strobe(UP);

   //
   // Set data line voltages
   //
   LCD_DATA_OUT = HOME_ADDRESS;

   //
   // Set control lines depending on if the
   // data is data or instructions
   //
   LCD_CTRL_OUT &= ~LCD_PIN_RS;


   //
   // Set write strobe down
   //
   lcd_strobe(DOWN);

   //
   // Delay write data hold time
   //
   __delay_cycles(65000);

   //
   // Clear all pins after the write, leaving
   // the data lines as outputs to preserve health
   // of logic translater circuit (LCD is 5V).
   //
   LCD_DATA_OUT &= (~LCD_DATA_MASK);
   LCD_CTRL_OUT &= (~LCD_CTRL_MASK);
}

/**
  * void lcd_printString(char *string)
  *    Writes a string out to the display.
  *
  * Parameters:
  *    char *string:  A pointer to the null terminated
  *                   string to print on the display.
  *    int   delay:   TRUE to delay between printing of each
  *                   character of the string, else FALSE.
  *
  * Returns:
  *    void
  **/
void lcd_printString(char *string, int delay, int line){
   //
   // Move cursor to home position on given line
   //
   if (line == TOP_LINE){
      lcd_goHome();
   }
   else if (line == BOTTOM_LINE){
      lcd_writeData(0x41, FALSE);
   }
   else{
      __delay_cycles(1000);
   }

   //
   // Set up pointer to string to iteratie through and print
   //
   char *string_pointer;
   string_pointer = string;

   //
   // Iterate through string and print each character
   //
   while ((string_pointer != 0) && (*string_pointer !=0)){
      lcd_printChar(string_pointer);
      string_pointer++;
      //
      // If a typewriter-mimic is desired (delay is true) then
      // delay between printing each character.
      //
      if (delay){
         __delay_cycles(8000);
      }
   }
   return;
}

/**
  * void lcd_printChar(char *letter)
  *   Writes a letter to the current curso position
  *
  * Parameters:
  *    char *letter: A pointer to the letter to print
  *
  * Returns:
  *    void
  **/
void lcd_printChar(char *letter){
   char *letter_pointer;
   letter_pointer = letter;
   lcd_writeData(*letter_pointer, TRUE);
}

/**
  * void lcd_shift(int shift)
  *    Shifts the aspect of the display in the direction
  *    desired.
  *
  * Parameters:
  *    int shift:  Determines what thing and direction
  *                to shift the
  *                   DISPLAY_RIGHT: shift display right one
  *                   DISPLAY_LEFT:  shift display left one
  *                   CURSOR_RIGHT:  shift cursor right one
  *                   CURSOR_LEFT:   shift cursor left one
  *
  * Returns:
  *    void
  **/
void lcd_shift(int shift){
   lcd_writeData(shift, FALSE);
}

/**
  * char lcd_readCursorData()
  *    Reads the data from the current cursor location
  *
  * Parameters:
  *   none
  *
  * Returns:
  *   char:    The data read at the location
  **/
unsigned char lcd_readCursorData(){
   //
   // CLear data lines and set direction to input.
   //
   LCD_DATA_OUT &= ~LCD_DATA_MASK;
   LCD_CTRL_OUT &= ~LCD_CTRL_MASK;
   LCD_DATA_DIR = ~LCD_DATA_MASK;

   //
   // Set R/W and RS lines to high.
   // Then, delay RS, R/W delay time.
   //
   LCD_CTRL_OUT |= (LCD_PIN_RW | LCD_PIN_RS);
   __delay_cycles(800000);

   //
   // Strobe enable line high, allowing output from LCD,
   // delay read delay output time.
   //
   lcd_strobe(UP);
   __delay_cycles(800000);

   //
   // Save data from LCD output to a char, read_data.
   //
   volatile unsigned char read_data = P1IN;

   //
   // Drop enable line back to low, disabling output,
   // delay read data hold time.
   //
   lcd_strobe(DOWN);
   __delay_cycles(3000);

   //
   // Clear control lines, and set pins data pins to
   // outputs again.  Then return the data saved from the LCD.
   //
   LCD_DATA_DIR |= LCD_DATA_MASK;
   LCD_CTRL_OUT &= ~LCD_CTRL_MASK;
   return read_data;
}
