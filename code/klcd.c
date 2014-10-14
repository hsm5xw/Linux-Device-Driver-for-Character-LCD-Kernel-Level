/*  This work is licensed under a Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 *
 *  Copyright(c) 2014 Hong Moon 	All Rights Reserved
 */

/* description: a kernel level Linux device driver to control a 16x2 character LCD (with HD44780 LCD controller) with 4 bit mode.
  		The LCD is interfaced with a micro-controller using GPIO pins.

		(Tested on Linux 3.8.13)

   name:	Hong Moon (hsm5xw@gmail.com)
   date:	2014-Sept
   platform:	Beaglebone Black
*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>  // linux gpio interface

#include <linux/delay.h> // delay

#include "klcd.h"

#define DRIVER_AUTHOR	"Hong Moon <hsm5xw.gmail.com>"
#define DRIVER_DESC	"a 16x2 character LCD (HD44780 LCD controller) driver with 4 bit mode"	


// ************ Core Functions ************************************

/*
 * description:		 Set up a GPIO pin for LCD.
 * 
 * @param pin_number	 pin number to be set up. 	
*/
static int lcd_pin_setup(unsigned int pin_number)
{
	int ret;
	PIN_DIRECTION gpio_direction = OUTPUT_PIN;

	// request GPIO allocation
	ret = gpio_request( pin_number, "GPIO pin request");
	if( ret != 0 )	{
		printk( KERN_DEBUG "ERR: Failed to request GPIO pin %d \n", pin_number );
		return ret;
	}	
	
	// set GPIO pin export (also disallow user space to change GPIO direction)
	ret = gpio_export( pin_number, 0);
	if( ret != 0 )	{
		printk( KERN_DEBUG "ERR: Failed to export GPIO pin %d \n", pin_number );
		return ret;
	}

	// set GPIO pin direction
	ret = gpio_direction_output( pin_number, gpio_direction);
	if( ret != 0 )	{
		printk( KERN_DEBUG "ERR: Failed to set GPIO pin direction %d \n", pin_number );	
		return ret;
	}

	// set GPIO pin default value
	gpio_set_value( pin_number, 0);

	// return value when there is no error
	return 0; 
}

/*
 * description: Set up all GPIO pins needed for LCD
 * 
*/
static void lcd_pin_setup_All()
{
	lcd_pin_setup(LCD_RS_PIN_NUMBER);
	lcd_pin_setup(LCD_E_PIN_NUMBER);

	lcd_pin_setup(LCD_DB4_PIN_NUMBER);
	lcd_pin_setup(LCD_DB5_PIN_NUMBER);
	lcd_pin_setup(LCD_DB6_PIN_NUMBER);
	lcd_pin_setup(LCD_DB7_PIN_NUMBER);
}

/*
 * description:		 Release a GPIO pin for LCD.
 * 
 * @param pin_number	 pin number to be set up. 	
*/
static void lcd_pin_release(unsigned int pin_number)
{
	gpio_unexport( pin_number);	// return GPIO pin
	gpio_free( pin_number);		// unexport GPIO pin
}

/*
 * description: Release all GPIO pins needed for LCD
 * 
*/
static void lcd_pin_release_All()
{
	lcd_pin_release(LCD_RS_PIN_NUMBER);
	lcd_pin_release(LCD_E_PIN_NUMBER);

	lcd_pin_release(LCD_DB4_PIN_NUMBER);
	lcd_pin_release(LCD_DB5_PIN_NUMBER);
	lcd_pin_release(LCD_DB6_PIN_NUMBER);
	lcd_pin_release(LCD_DB7_PIN_NUMBER);
}

/*
 * description:		 send a 4-bit command to the HD44780 LCD controller.
 *
 * @param command	 command to be sent to the LCD controller. Only the upper 4 bits of this command is used.
*/
static void lcd_instruction(char command)
{
	int db7_data = 0;
	int db6_data = 0;
	int db5_data = 0;
	int db4_data = 0;

	usleep_range(2000, 3000);	// added delay instead of busy checking

	// Upper 4 bit data (DB7 to DB4)
	db7_data = ( (command)&(0x1 << 7) ) >> (7) ;
	db6_data = ( (command)&(0x1 << 6) ) >> (6) ;
	db5_data = ( (command)&(0x1 << 5) ) >> (5) ;
	db4_data = ( (command)&(0x1 << 4) ) >> (4) ;

	gpio_set_value(LCD_DB7_PIN_NUMBER, db7_data);
	gpio_set_value(LCD_DB6_PIN_NUMBER, db6_data);
	gpio_set_value(LCD_DB5_PIN_NUMBER, db5_data);
	gpio_set_value(LCD_DB4_PIN_NUMBER, db4_data);

	// Set to command mode
	gpio_set_value(LCD_RS_PIN_NUMBER, RS_COMMAND_MODE);
	usleep_range(5, 10);

	// Simulate falling edge triggered clock
	gpio_set_value(LCD_E_PIN_NUMBER, 1);
	usleep_range(5, 10);
	gpio_set_value(LCD_E_PIN_NUMBER, 0);
}


/*
 * description:		send a 1-byte ASCII character data to the HD44780 LCD controller.
 * @param data		a 1-byte data to be sent to the LCD controller. Both the upper 4 bits and the lower 4 bits are used.
*/
static void lcd_data(char data)
{
	int db7_data = 0;
	int db6_data = 0;
	int db5_data = 0;
	int db4_data = 0;

	// Part 1.  Upper 4 bit data (from bit 7 to bit 4)
	usleep_range(2000, 3000); 	// added delay instead of busy checking

	db7_data = ( (data)&(0x1 << 7) ) >> (7) ;
	db6_data = ( (data)&(0x1 << 6) ) >> (6) ;
	db5_data = ( (data)&(0x1 << 5) ) >> (5) ;
	db4_data = ( (data)&(0x1 << 4) ) >> (4) ;

	gpio_set_value(LCD_DB7_PIN_NUMBER, db7_data);
	gpio_set_value(LCD_DB6_PIN_NUMBER, db6_data);
	gpio_set_value(LCD_DB5_PIN_NUMBER, db5_data);
	gpio_set_value(LCD_DB4_PIN_NUMBER, db4_data);

	// Part 1. Set to data mode
	gpio_set_value(LCD_RS_PIN_NUMBER, RS_DATA_MODE);
	usleep_range(5, 10);

	// Part 1. Simulate falling edge triggered clock
	gpio_set_value(LCD_E_PIN_NUMBER, 1);
	usleep_range(5, 10);
	gpio_set_value(LCD_E_PIN_NUMBER, 0);	


	// Part 2. Lower 4 bit data (from bit 3 to bit 0)
	usleep_range(2000, 3000);	// added delay instead of busy checking

	db7_data = ( (data)&(0x1 << 3) ) >> (3) ;
	db6_data = ( (data)&(0x1 << 2) ) >> (2) ;
	db5_data = ( (data)&(0x1 << 1) ) >> (1) ;
	db4_data = ( (data)&(0x1)      )        ;

	gpio_set_value(LCD_DB7_PIN_NUMBER, db7_data);
	gpio_set_value(LCD_DB6_PIN_NUMBER, db6_data);
	gpio_set_value(LCD_DB5_PIN_NUMBER, db5_data);
	gpio_set_value(LCD_DB4_PIN_NUMBER, db4_data);

	// Part 2. Set to data mode
	gpio_set_value(LCD_RS_PIN_NUMBER, RS_DATA_MODE);
        usleep_range(5, 10);

	// Part 2. Simulate falling edge triggered clock
	gpio_set_value(LCD_E_PIN_NUMBER, 1);
	usleep_range(5, 10);
	gpio_set_value(LCD_E_PIN_NUMBER, 0);	
}

/*
 * description: 	initialize the LCD in 4 bit mode as described on the HD44780 LCD controller document.
*/
static void lcd_initialize()
{
	usleep_range(41*1000, 50*1000);	// wait for more than 40 ms once the power is on

	lcd_instruction(0x30);		// Instruction 0011b (Function set)
	usleep_range(5*1000, 6*1000);	// wait for more than 4.1 ms

	lcd_instruction(0x30);		// Instruction 0011b (Function set)
	usleep_range(100,200);		// wait for more than 100 us

	lcd_instruction(0x30);		// Instruction 0011b (Function set)
	usleep_range(100,200);		// wait for more than 100 us

	lcd_instruction(0x20);		/* Instruction 0010b (Function set)
					   Set interface to be 4 bits long
					*/
	usleep_range(100,200);		// wait for more than 100 us

	lcd_instruction(0x20);		// Instruction 0010b (Function set)
	lcd_instruction(0x80);		/* Instruction NF**b
					   Set N = 1, or 2-line display
					   Set F = 0, or 5x8 dot character font
					 */
	usleep_range(41*1000,50*1000);

					/* Display off */
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0x80);		// Instruction 1000b
	usleep_range(100,200);

					/* Display clear */
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0x10);		// Instruction 0001b
	usleep_range(100,200);

					/* Entry mode set */
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0x60);		/* Instruction 01(I/D)Sb -> 0110b
					   Set I/D = 1, or increment or decrement DDRAM address by 1
					   Set S = 0, or no display shift
					*/
	usleep_range(100,200);

	/* Initialization Completed, but set up default LCD setting here */

					/* Display On/off Control */
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0xF0);		/* Instruction 1DCBb  
					   Set D= 1, or Display on
					   Set C= 1, or Cursor on
					   Set B= 1, or Blinking on
					*/
	usleep_range(100,200);
}


/*
 * description: 	print a string data on the LCD
 * 			(If the line number is 1 and the string is too long to be fit in the first line, the LCD
 * 			will continue to print the string on the second line)
 *
 * @param lineNumber	the line number of the LCD where the string is be printed. It should be either 1 or 2.
 * 			Otherwise, it is readjusted to 1.
 *
 * detail:		I implemented the code to only allow a certain number of characters to be written on the LCD.
 * 			As each character is written, the DDRAM address in the LCD controller is incrmented.
 * 			When the string is too long to be fit in the LCD, the DDRAM can be set back to 0 and the existing
 * 			data on the LCD are overwritten by the new data. This causes the LCD to be very unstable and also
 * 			lose data.		
*/
static void lcd_print(char * msg, unsigned int lineNumber)
{
	unsigned int counter = 0;
	unsigned int lineNum = lineNumber;

	if(msg == NULL){
		printk( KERN_DEBUG "ERR: Empty data for lcd_print \n");
		return;
	}

	if( (lineNum != 1) && (lineNum != 2) ) { 
		printk( KERN_DEBUG "ERR: Invalid line number readjusted to 1 \n");
		lineNum = 1;
	}

	if( lineNum == 1 )
	{
		lcd_setLinePosition( LCD_FIRST_LINE );

		while( *(msg) != '\0' )
		{
			if(counter >=  NUM_CHARS_PER_LINE )
			{
				lineNum = 2;	// continue writing on the next line if the string is too long
				counter = 0;
				break;		
			}

			lcd_data(*msg);
			msg++;
			counter++;
		}
	}

	if( lineNum == 2)
	{
		lcd_setLinePosition( LCD_SECOND_LINE);
		
		while( *(msg) != '\0' )
		{
			if(counter >=  NUM_CHARS_PER_LINE )
			{
				 break;
			}

			lcd_data(*msg);
			msg++;
			counter++;
		}
	}
}

/*
 * description: 	print a string data on the specified position of the LCD
 * 			(If the line number is 1 and the string is too long to be fit in the first line, the LCD
 * 			 will continue to print the string on the second line)
 *
 * @param lineNumber 	the line number of the LCD where the string is printed. It should be either 1 or 2.
 * 			Otherwise, it is readjusted to 1.
 *
 * @param nthCharacter  the nth character of the line where the string is printed.
 * 			It starts from 0, which indicates the beginning of the line specified.
*/

static void lcd_print_WithPosition(char * msg, unsigned int lineNumber, unsigned int nthCharacter)
{
	unsigned int counter = nthCharacter;
	unsigned int lineNum = lineNumber;
	unsigned int nthChar = nthCharacter;

	if( msg == NULL ){
		printk( KERN_DEBUG "ERR: Empty data for lcd_print_WithPosition \n");
		return;
	}

	if( (lineNum != 1) && (lineNum != 2)  ){
		printk( KERN_DEBUG "ERR: Invalid line number input readjusted to 1 \n");
		lineNum = 1;
	}

	if( lineNum == 1 )
	{
		lcd_setPosition( LCD_FIRST_LINE, nthChar );
		
		while( *(msg) != '\0' )
		{
			if( counter >= NUM_CHARS_PER_LINE )
			{
				lineNum = 2;  // continue writing on the next line if the string is too long
				counter = 0;
				nthChar = 0;
				break;
			}
			lcd_data(*msg);
			msg++;
			counter++;
		}
	}

	if( lineNum == 2)
	{	
		lcd_setPosition( LCD_SECOND_LINE, nthChar );

		while( *(msg) != '\0' )
		{
			if( counter >= NUM_CHARS_PER_LINE )
			{
				break;
			}	
			lcd_data(*msg);
			msg++;
			counter++;
		}
	}
}

/*
 * description:	set the cursor to the beginning of the line specified.
 * @param line  line number should be either 1 or 2.	
*/
void lcd_setLinePosition(unsigned int line)
{
	if(line == 1){
		lcd_instruction(0x80);	// set position to LCD line 1
		lcd_instruction(0x00);
	}
	else if(line == 2){
		lcd_instruction(0xC0);
		lcd_instruction(0x00);
	}
	else{
		printk("ERR: Invalid line number. Select either 1 or 2 \n");
	}
}

/*
 * description:  	 set the cursor to the nth character of the line specified.
 * @param line 		 the line number should be either 1 or 2. 
 * @param nthCharacter	 n'th character where the cursor should start on the line specified.
 * 			 It starts from 0, which indicates the beginning of the line.
*/
void lcd_setPosition(unsigned int line, unsigned int nthCharacter)
{
	char command;

	if(line == 1){
		command = 0x80 + (char) nthCharacter;
		
		lcd_instruction(  command & 0xF0 ); 	  // upper 4 bits of command
		lcd_instruction( (command & 0x0F) << 4 ); // lower 4 bits of command 
	}
	else if(line == 2){
		command = 0xC0 + (char) nthCharacter;

		lcd_instruction(  command & 0xF0 ); 	  // upper 4 bits of command
		lcd_instruction( (command & 0x0F) << 4 ); // lower 4 bits of command
	}
	else{
		printk("ERR: Invalid line number. Select either 1 or 2 \n");
	}	
}

/*
 * description:	clear the display on the LCD	
*/
static void lcd_clearDisplay()
{
	lcd_instruction( 0x00 ); // upper 4 bits of command
	lcd_instruction( 0x10 ); // lower 4 bits of command

	printk(KERN_INFO "klcd Driver: display clear\n");
}

/*
 * description:	show a blinking cursor on the LCD screen	
*/
static void lcd_cursor_on()
{
					/* Display On/off Control */
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0xF0);		/* Instruction 1DCBb  
					   Set D= 1, or Display on

					   Set C= 1, or Cursor on
					   Set B= 1, or Blinking on
					*/
	printk(KERN_INFO "klcd Driver: lcd_cursor_on\n");
}

/*
 * description:	hide a blinking cursor from the LCD screen	
*/
static void lcd_cursor_off()
{
					/* Display On/off Control */
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0xC0);		/* Instruction 1DCBb  
					   Set D= 1, or Display on

					   Set C= 0, or Cursor off
					   Set B= 0, or Blinking off
					*/
	printk(KERN_INFO "klcd Driver: lcd_cursor_off\n");
}



/*
 * description:	turn off the LCD display. It is called upon module exit	
*/
static void lcd_display_off(void)
{
	lcd_instruction(0x00);		// Instruction 0000b
	lcd_instruction(0x80);		/* Instruction 1DCBb  
					   Set D= 0, or Display off

					   Set C= 0, or Cursor off
					   Set B= 0, or Blinking off
					*/
	printk(KERN_INFO "klcd Driver: lcd_display_off\n");
}


// ************* File Operations *****************************************************************

static int klcd_open(struct inode *p_inode, struct file *p_file )
{
	printk(KERN_INFO "klcd Driver: open()\n");
	return 0;
}
static int klcd_close(struct inode *p_inode, struct file *p_file )
{
	printk(KERN_INFO "klcd Driver: close()\n\n");
	return 0;
}
static ssize_t klcd_read(struct file *p_file, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "klcd Driver: read()\n");
	return 0;
}
static ssize_t klcd_write(struct file *p_file, const char __user *buf, size_t len, loff_t *off)
{
	char kbuf[MAX_BUF_LENGTH];
	unsigned long copyLength;

	if( buf == NULL){
		printk( KERN_DEBUG "ERR: Empty user space buffer \n" );
		return -ENOMEM;
	}

	memset( kbuf, '\0', sizeof(char) * MAX_BUF_LENGTH );
	copyLength = MIN( (MAX_BUF_LENGTH-1), (unsigned long) (len-1) );

	if(copyLength < 0)
		copyLength = 0;

	// Copy user space buffer to kernel space buffer
	if( copy_from_user(&kbuf, buf, copyLength ) ){
		printk( KERN_DEBUG "ERR: Failed to copy from user space buffer \n" );
		return -EFAULT;
	}

	//printk( KERN_INFO "***** value copied from user space:  %s *****\n", kbuf );
	//printk( KERN_INFO "***** copyLength:  %lu *****\n", copyLength );
	
	// clear display
	lcd_clearDisplay();

	// print on the first line by default
	lcd_print( kbuf, LCD_FIRST_LINE);

	printk(KERN_INFO "klcd Driver: write()\n");

	return len;
}

static long klcd_ioctl( struct file *p_file, unsigned int ioctl_command, unsigned long arg)
{
	struct ioctl_mesg ioctl_arguments;

	printk(KERN_INFO "klcd Driver: ioctl\n");
      	
	if( ((const void *)arg) == NULL){
		printk( KERN_DEBUG "ERR: Invalid argument for klcd IOCTL \n");
		return -EINVAL;
	}

	memset( ioctl_arguments.kbuf, '\0', sizeof(char) * MAX_BUF_LENGTH );

	// copy ioctl command argument from user space
	if( copy_from_user( &ioctl_arguments, (const void *)arg, sizeof(ioctl_arguments) ) ){
		printk( KERN_DEBUG "ERR: Failed to copy from user space buffer \n" );
		return -EFAULT;
	}

	switch( (char) ioctl_command ){
		case IOCTL_CLEAR_DISPLAY:
			lcd_clearDisplay();
			break;

		case IOCTL_PRINT_ON_FIRSTLINE:
			lcd_print( ioctl_arguments.kbuf, LCD_FIRST_LINE);
			break;

		case IOCTL_PRINT_ON_SECONDLINE:
			lcd_print( ioctl_arguments.kbuf, LCD_SECOND_LINE);
			break;

		case IOCTL_PRINT_WITH_POSITION:
			lcd_print_WithPosition( ioctl_arguments.kbuf, ioctl_arguments.lineNumber, ioctl_arguments.nthCharacter);
			break;

		case IOCTL_CURSOR_ON:
			lcd_cursor_on();
			break;

		case IOCTL_CURSOR_OFF:
			lcd_cursor_off();
			break;

		default:
			printk(KERN_DEBUG "klcd Driver (ioctl): No such command \n");
			return -ENOTTY;
	}
	
	return 0;
}


/* file operation structure */
static struct file_operations klcd_fops =
{
	.owner = THIS_MODULE,
	.open  =  klcd_open,
	.release = klcd_close,
	.read    = klcd_read,
	.write   = klcd_write,
	.unlocked_ioctl	= klcd_ioctl,
};

/*
 * description:	initialize the device module	
*/
static int __init klcd_init(void)
{
	struct device *dev_ret;

	// dynamically allocate device major number
	if( alloc_chrdev_region( &dev_number, MINOR_NUM_START , MINOR_NUM_COUNT , DEVICE_NAME ) < 0) 
	{
		printk( KERN_DEBUG "ERR: Failed to allocate major number \n" );
		return -1;
	}

	// create a class structure
	klcd_class = class_create( THIS_MODULE, CLASS_NAME );
	
	if( IS_ERR(klcd_class) )
	{		
		unregister_chrdev_region( dev_number, MINOR_NUM_COUNT );
		printk( KERN_DEBUG "ERR: Failed to create class structure \n" );
		
		return PTR_ERR( klcd_class ) ;
	}
			
	// create a device and registers it with sysfs
	dev_ret = device_create( klcd_class, NULL, dev_number, NULL, DEVICE_NAME );
	
	if( IS_ERR(dev_ret) )
	{
		class_destroy( klcd_class );
		unregister_chrdev_region( dev_number, MINOR_NUM_COUNT );
		printk( KERN_DEBUG "ERR: Failed to create device structure \n" );		

		return PTR_ERR( dev_ret );
	}

	// initialize a cdev structure
	cdev_init( &klcd_cdev, &klcd_fops);

	// add a character device to the system
	if( cdev_add( &klcd_cdev, dev_number, MINOR_NUM_COUNT) < 0 )
	{
		device_destroy( klcd_class, dev_number);
		class_destroy(  klcd_class );
		unregister_chrdev_region( dev_number, MINOR_NUM_COUNT );
		printk( KERN_DEBUG "ERR: Failed to add cdev \n" );		

		return -1;		
	}

	// setup GPIO pins
	lcd_pin_setup_All();

	// initialize LCD once
	lcd_initialize();

	printk(KERN_INFO "klcd Driver Initialized. \n");
	return 0;
}

/*
 * description:	release the device module	
*/
static void __exit klcd_exit(void)
{
	// turn off LCD display
	lcd_display_off();

	// remove a cdev from the system
	cdev_del( &klcd_cdev);

	// remove device
	device_destroy( klcd_class, dev_number );

	// destroy class
	class_destroy( klcd_class );	

	// deallocate device major number
	unregister_chrdev_region( MAJOR(dev_number), MINOR_NUM_COUNT );

	// releasse GPIO pins
	lcd_pin_release_All();

	printk(KERN_INFO "klcd Driver Exited. \n");
}

module_init( klcd_init );
module_exit( klcd_exit );


// ************* EXTRA INFO *********************************************

MODULE_LICENSE("GPL");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("test device");

