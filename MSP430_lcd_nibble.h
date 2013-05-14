/*
 * MSP430_lcd.h
 *
 *	Created on: Apr 8, 2013
 *		Author: donaldpercivalle
 */
#ifndef MSP430_LCD_H_
#define MSP430_LCD_H_

/**
  * MSP430 LCD interface
  * PIN 14 on the LCD is the left-most pin
  *
  *		      |	    LCD	    |
  * LCD Pin #|  Function	|	 MSP Pin #
  *------------------------------------------
  *	PIN 14	|	   DB7		 |	  P1.3
  *	PIN 13	|	   DB6		 |	  P1.2
  *	PIN 12	|	   DB5		 |	  P1.1
  *	PIN 11	|	   DB4		 |	  P1.0
  *	PIN 10	|	   DB3		 |	  NC
  *	PIN 9		|	   DB2		 |	  NC
  *	PIN 8		|	   DB1		 |	  NC
  *	PIN 7		|	   DB0		 |	  NC
  *	PIN 6		|	   E		   |	  P2.2
  *	PIN 5		|	   R/W		 |	  P2.1
  *	PIN 4		|	   RS		   |	  P2.0
  *	PIN 3		|	   CONTRAST|	  GND (POT)
  *	PIN 2		|	   VCC		 |	  5V
  *	PIN 1		|	   VSS		 |	  GND
  **/

/**
  * Define direction and data buses
  **/
#define		LCD_DATA_DIR		  P1DIR
#define		LCD_CTRL_DIR		  P2DIR
#define		LCD_DATA_OUT		  P1OUT
#define   LCD_DATA_IN       P1IN
#define		LCD_CTRL_OUT		  P2OUT

/**
  * Define bit masks
  **/
#define		LCD_DATA_MASK		  0x0F
#define		LCD_CTRL_MASK		  0x07

/**
  * Define symbolic LCD - MCU pin mappings
  * We've set DATA PIN TO 0, 1, 2, 3, 4,5,6,7
  * for easy translation
  **/
#define		LCD_PIN_EN			  BIT2			// P2.0
#define		LCD_PIN_RW			  BIT1			// P2.1
#define		LCD_PIN_RS			  BIT0			// P2.2
#define		LCD_PIN_D7			  BIT3			// P1.3
#define		LCD_PIN_D6			  BIT2			// P1.2
#define		LCD_PIN_D5			  BIT1			// P1.1
#define		LCD_PIN_D4			  BIT0			// P1.0

/**
  * LCD Module constants
  **/
#define		SET_FUNCTION	     0x2C  // Nibble Mode 5x10 Font
#define		DISPLAY_SET		     0x0C
#define		CLEAR_DISPLAY       0x01
#define		MODE_RT_NOSHIFT     0x06
#define     DDRAM_ADDR_INIT     0x80
#define     HOME_ADDRESS        0x02
#define     TRUE                1
#define     DELAY               1
#define     FALSE               0
#define     NO_DELAY            0
#define     UP                  1
#define     DOWN                0
#define     CURSOR_RIGHT        0x14
#define     CURSOR_LEFT         0x10
#define     DISPLAY_RIGHT       0x1C
#define     DISPLAY_LEFT        0x18
#define     TOP_LINE            0x80
#define     BOTTOM_LINE         0xC0

/**
  * LCD Driver Function Declarations
  **/
void lcd_init(char entryMode);
void lcd_customCharacterInit(unsigned char *ptr);
void lcd_strobe(int direction);
void lcd_writeData(char data, int isData);
void lcd_clearDisplay();
void lcd_goHome();
void lcd_printString(char *string, int delay, int line);
void lcd_printChar(char *letter);
void lcd_shift(int shift);
unsigned char lcd_readCursorData();

#endif /* MSP430_LCD_H_ */
