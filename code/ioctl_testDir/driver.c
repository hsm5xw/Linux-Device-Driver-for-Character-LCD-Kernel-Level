/*  This work is licensed under a Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 *
 *  Copyright(c) 2014 Hong Moon 	All Rights Reserved
 */

/* description: an ioctl test program.

		a user level test program to check the ioctl functionality ofthe implemented Linux device driver, which controls a 16x2  			character LCD (with HD44780 LCD controller) with 4 bit mode.
  		
		The LCD is interfaced with a micro-controller using GPIO pins.

		(Tested on Linux 3.8.13)

   name:	Hong Moon (hsm5xw@gmail.com)
   date:	2014-Sept
   platform:	Beaglebone Black
*/


#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
 
#include "driver.h"

int main ( int argc, char *argv[] )
{
	struct ioctl_mesg msg;
	const char *ioctl_command;
	char command;
	int fd;

	char * pEnd1;
	char * pEnd2;

    	if ( argc != 5 ) {
        	printf( "Usage: %s ([1]command) ([2]string to be printed) ([3]line number) ([4]nth Character offset)\n\n", argv[0] );
		return -1;
    	}

	//  ************ argument setting ******************************************************
	ioctl_command 	= argv[1];
	printf("ioctl command debug: %c \n", *ioctl_command);

	memset(  msg.kbuf, '\0', sizeof(char) * MAX_BUF_LENGTH );
	strncpy( msg.kbuf, argv[2],  MAX_BUF_LENGTH);
	msg.kbuf[MAX_BUF_LENGTH-1] = '\0';  // add null terminator to prevent buffer overflow

	msg.lineNumber   = (unsigned int) strtoul(argv[3],&pEnd1,10);
	msg.nthCharacter = (unsigned int) strtoul(argv[4],&pEnd2,10);

	//****************************************************************************************   

	fd = open("/dev/klcd", O_WRONLY | O_NDELAY);
	if(fd < 0){
		printf("[User level Debug] ERR: Unable to open klcd \n");
		return -1;
	}

	command = *(ioctl_command);
	
	switch( command ){
		// clear the LCD display
		case (IOCTL_CLEAR_DISPLAY ):
			printf("KLCD IOCTL Option: Clear Display \n");	

			if( ioctl( fd, (unsigned int) IOCTL_CLEAR_DISPLAY, &msg) < 0)
				perror("[ERROR] IOCTL_CLEAR_DISPLAY \n");
			break;

		// print on the beginning of the first line of the LCD. Other arguments ignored
		case (IOCTL_PRINT_ON_FIRSTLINE ):
			printf("KLCD IOCTL Option: Print on First Line \n");	

			if( ioctl( fd, (unsigned int) IOCTL_PRINT_ON_FIRSTLINE, &msg) < 0)
				perror("[ERROR] IOCTL_PRINT_ON_FIRSTLINE \n");			
			break;

		// print on the beginning of the second line of the LCD. Other arguments ignored
		case (IOCTL_PRINT_ON_SECONDLINE ):
			printf("KLCD IOCTL Option: Print on Second Line \n");	

			if( ioctl( fd, (unsigned int) IOCTL_PRINT_ON_SECONDLINE, &msg) < 0)
				perror("[ERROR] IOCTL_PRINT_ON_SECONDLINE \n");			
			break;		

		// print on the specified position (line number, nth Character) of the LCD. 
		case (IOCTL_PRINT_WITH_POSITION ):
			printf("KLCD IOCTL Option: Print With Specified Position \n");	

			if( ioctl( fd, (unsigned int) IOCTL_PRINT_WITH_POSITION, &msg) < 0)
				perror("[ERROR] IOCTL_PRINT_WITH_POSITION \n");				
			break;

		// enable a blinking cursor on the LCD
		case (IOCTL_CURSOR_ON ):
			printf("KLCD IOCTL Option: Cursor on \n");

			if( ioctl( fd, (unsigned int) IOCTL_CURSOR_ON, &msg) < 0)
				perror("[ERROR] IOCTL_CURSOR_ON \n");
			break; 
		
		// disable a blinkin cursor on the LCD
		case (IOCTL_CURSOR_OFF ):
			printf("KLCD IOCTL Option: Cursor off \n");

			if( ioctl( fd, (unsigned int) IOCTL_CURSOR_OFF, &msg) < 0)
				perror("[ERROR] IOCTL_CURSOR_OFF \n");
			break;

		// Write call Tests
		/* #### Test cases used for write mode robustness checking. Passed Test cases */
		/*
		case (WRITE_TEST_MODE1 ):
			write(fd, msg.kbuf, sizeof(char) * MAX_BUF_LENGTH );
			break;

		case (WRITE_TEST_MODE2 ):
			write(fd, NULL, sizeof(char) * MAX_BUF_LENGTH );
			break;

		case (WRITE_TEST_MODE3 ):
			write(fd, "ABCDEFG", 0 );
			break;
		*/		

		default:
			printf("[User level Debug] klcd Driver (ioctl): No such command \n");
			break;
	}

	close(fd);	
	printf("KLCD User level Test Program \n");
}
