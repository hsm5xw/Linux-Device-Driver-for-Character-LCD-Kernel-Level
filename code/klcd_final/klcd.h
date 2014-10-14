/*  This work is licensed under a Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 *
 *  Copyright(c) 2014 Hong Moon 	All Rights Reserved  
 */

/* description: a kernel level Linux device driver to control a 16x2 character LCD (with HD44780 LCD Controller) with 4 bit mode.
		The LCD is interfaced with a micro-controller using GPIO pins.
		See below "LCD Pin Configuration" to see GPIO pin connections

		(Tested on Linux 3.8.13)

   name:	Hong Moon (hsm5xw@gmail.com)
   date:	2014-Sept
   platform:	Beaglebone Black
*/

#ifndef KLCD_H_
#define KLCD_H_

// ******** LCD Pin Configuration *****************************************************************

#define LCD_RS_PIN_NUMBER	67  // LCD_RS: P8_8  (GPIO pin 67)
#define LCD_E_PIN_NUMBER	68  // LCD_E:  P8_10 (GPIO pin 68)

#define LCD_DB4_PIN_NUMBER	65  // LCD_DB4: P8_18 (GPIO pin 65)
#define LCD_DB5_PIN_NUMBER	46  // LCD_DB5: P8_16 (GPIO pin 46)
#define LCD_DB6_PIN_NUMBER	26  // LCD_DB6: P8_14 (GPIO pin 26)
#define LCD_DB7_PIN_NUMBER	44  // LCD_DB7: P8_12 (GPIO pin 44)


// ******** LCD Constants ************************************************************************

#define RS_COMMAND_MODE		0   // command mode to select Insruction register with RS signal
#define RS_DATA_MODE		1   // data mode to select Data register with RS signal

#define LCD_FIRST_LINE		1
#define LCD_SECOND_LINE		2

#define NUM_CHARS_PER_LINE      16  // the number of characters per line

// ********* Linux driver Constants ******************************************************************

#define MINOR_NUM_START		0   // minor number starts from 0
#define MINOR_NUM_COUNT		1   // the number of minor numbers required

#define MAX_BUF_LENGTH  	50  // maximum length of a buffer to copy from user space to kernel space

#define MIN(x, y) (((x) < (y)) ? (x) : (y))


// ********* IOCTL COMMAND ARGUMENTS ******************************************************************

#define IOCTL_CLEAR_DISPLAY 	  	'0'	// Identifiers for ioctl reqursts
#define IOCTL_PRINT_ON_FIRSTLINE  	'1'
#define IOCTL_PRINT_ON_SECONDLINE 	'2'	/* (Note) ioctl will not be called if this is unsigned int 2, which
						          is a reserved number. Thus it is fixed to '2'
						 */
#define IOCTL_PRINT_WITH_POSITION 	'3'
#define IOCTL_CURSOR_ON			'4'
#define IOCTL_CURSOR_OFF		'5'

struct ioctl_mesg{				// a structure to be passed to ioctl argument
	char kbuf[MAX_BUF_LENGTH];

	unsigned int lineNumber;
	unsigned int nthCharacter;
};


// ********* Device Structures *********************************************************************

#define CLASS_NAME  	"klcd"
#define DEVICE_NAME 	"klcd"

static dev_t 		dev_number;	// dynamically allocated device major number
struct cdev  		klcd_cdev;	// cdev structure
static struct class *  	klcd_class;	// class structure

// ********* GPIO Support *************************************************************************

typedef enum pin_dir
{
	INPUT_PIN  = 0,
	OUTPUT_PIN = 1
} PIN_DIRECTION;

// ********* Function Prototypes *******************************************************************

static int  lcd_pin_setup(unsigned int pin_number);
static void lcd_pin_setup_All( void );
static void lcd_pin_release(unsigned int pin_number);
static void lcd_pin_release_All( void );


static void lcd_instruction(char command);
static void lcd_data(char data);
static void lcd_initialize(void);
static void lcd_print(char * msg, unsigned int lineNumber);
static void lcd_print_WithPosition(char * msg, unsigned int lineNumber, unsigned int nthCharacter);

static void lcd_setLinePosition(unsigned int line);
static void lcd_setPosition(unsigned int line, unsigned int nthCharacter);
static void lcd_clearDisplay(void);

static void lcd_cursor_on(void);
static void lcd_cursor_off(void);

#endif
