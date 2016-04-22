# Makefile 
# Use this to flash a serial bootloader
# hexfile is from https://github.com/Optiboot/optiboot
TARGET = Bootloader
LIBDIR = ../lib
OBJECTS = 

## programing ports. the FTDI on my board shows as ttyUSBx, while an Uno shows as ttyACMx
## /dev/ttyUSB0
##/dev/ttyACM0

all:

$(TARGET):

## http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p&LOW=FF&HIGH=DE&EXTENDED=05&LOCKBIT=2F
## stk500v1 is for Arduino as ISP (e.g. it is an example in the Arduino.cc IDE 1.6.7+)
## lock is from optiboot's Makefile.isp, If I understand it then overwriting an existing bootloader requires two steps
fuse: 
	avrdude -v -p atmega328p -c stk500v1 -P /dev/ttyACM0 -b 19200 -U lock:w:0x3f:m -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0xFD:m

## 328p can lock the bootloader section, which is a good idea even if it is not used
isp:
	avrdude -v -p atmega328p -c stk500v1 -P /dev/ttyACM0 -b 19200 -e -U flash:w:optiboot_atmega328.hex -U lock:w:0x2f:m

 
