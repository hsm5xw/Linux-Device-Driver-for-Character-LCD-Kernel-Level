#
# 	A Makefile to cross-compile a kernel module
#
#	(from a Ubuntu PC to a Beaglebone Black ARM target board)
#

obj-m	:= klcd.o
 
KDIR	:= /root/bb-kernel/KERNEL
PWD	:= $(shell pwd) 
 
default:
	$(MAKE) ARCH:=arm CROSS_COMPILE:=/root/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/bin/arm-linux-gnueabihf- \
	-C $(KDIR) M=$(PWD) modules 

clean:	
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf *.order
	rm -rf *.symvers
	rm -rf *.c~
	rm -rf *.h~
	
