Linux-Device-Driver-for-Character-LCD-Kernel-Level
==================================================

a kernel level Linux Device Driver for a 16x2 character LCD (with HD44780 LCD controller) with 4 bit mode

	A demo video is available here: https://www.youtube.com/watch?v=icP9ckrTLKc

This is a kernel level Linux Device driver to control a 16x2 character LCD (with HD44780 LCD controller) with 4 bit mode.
The LCD is interfaced with a micro-controller using GPIO pins. 
The program has been tested on Linux 3.8.13 debian, and the target board is Beaglebone Black ARM micro-controller.

This project is a continuation of my user level program, which was not originally developed with Linux in mind.
The project has been developed in a form of loadable kernel module, cross-compiled in a Desktop PC.

- The datasheet was found from here: 	 https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

- And the LCD was purchased from here:	 http://www.amazon.com/gp/product/B004MGXOUQ/ref=oh_aui_detailpage_o07_s00?ie=UTF8&psc=1	


						Reference
	[1] HD44780U (LCD-II), Hitachi Ltd., Tokyo, Japan, 1998.