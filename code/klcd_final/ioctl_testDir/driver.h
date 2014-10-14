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

#ifndef DRIVER_H_
#define DRIVER_H_

#include <linux/ioctl.h>

#define MAX_BUF_LENGTH  	50  /* maximum length of a buffer to copy from user space to kernel space
				       (MUST NOT CHANGE THIS)
				    */
struct ioctl_mesg{
	char kbuf[MAX_BUF_LENGTH];	// a string to be printed on the LCD

	unsigned int lineNumber;	// line number (should be either 1 or 2)
	unsigned int nthCharacter;	// nth Character of a line (0 refers to the beginning of the line)
};

// ******************* IOCTL COMMAND ARGUMENTS ********************************************

#define KLCD_MAGIC_NUMBER   		0xBC  // a "magic" number to uniquely identify the device 

#define IOCTL_CLEAR_DISPLAY 	  	'0'   // Identifiers for ioctl reqursts
#define IOCTL_PRINT_ON_FIRSTLINE  	'1'
#define IOCTL_PRINT_ON_SECONDLINE 	'2'	/* (Note) ioctl will not be called if this is unsigned int 2, which
						          is a reserved number. Thus it is fixed to '2'
						 */
#define IOCTL_PRINT_WITH_POSITION 	'3'
#define IOCTL_CURSOR_ON			'4'
#define IOCTL_CURSOR_OFF		'5'

#define WRITE_TEST_MODE1		'W'    // check error handling
#define WRITE_TEST_MODE2		'X'
#define WRITE_TEST_MODE3		'Y'

// ******************** IOCTL MACROS ********************************************************

#define KLCD_IOCTL_CLEAR_DISPLAY    	_IOW( KLCD_MAGIC_NUMBER, IOCTL_CLEAR_DISPLAY , struct ioctl_mesg)
#define KLCD_IOCTL_PRINT_ON_FIRSTLINE   _IOW( KLCD_MAGIC_NUMBER, IOCTL_PRINT_ON_FIRSTLINE,  struct ioctl_mesg)
#define KLCD_IOCTL_PRINT_ON_SECONDLINE  _IOW( KLCD_MAGIC_NUMBER, IOCTL_PRINT_ON_SECONDLINE, struct ioctl_mesg)
#define KLCD_IOCTL_PRINT_WITH_POSITION  _IOW( KLCD_MAGIC_NUMBER, IOCTL_PRINT_WITH_POSITION, struct ioctl_mesg)

#define KLCD_IOCTL_CURSOR_ON  		_IOW( KLCD_MAGIC_NUMBER, IOCTL_CURSOR_ON, struct ioctl_mesg)
#define KLCD_IOCTL_CURSOR_OFF  		_IOW( KLCD_MAGIC_NUMBER, IOCTL_CURSOR_OFF, struct ioctl_mesg)

#endif
