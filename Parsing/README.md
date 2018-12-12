# Parsing serial for textual commands and arguments

## Overview

Demonstration of command parsing with the redirected stdio functions (e.g. printf() and simular)  from avr-libc. 

## Firmware Upload

With a serial port connection (set the BOOTLOAD_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

```
sudo apt-get install git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno/
cd /RPUno/Parsing
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk).

``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Command Line

/addr/command [arg[,arg[,arg[,arg[,arg]]]]]

The command is structured like a topic after the address but has optional arguments. An echo starts after the address byte is seen on the serial interface at 38400 baud rate. The UART core has a 32-byte buffer, which is optimal for AVR. Commands and arguments are ignored if they overflow the buffer.


## addr

The address is a single character. That is to say, "/0/id?" is at address '0', where "/id?" is a command. Echo of the command occurs after the address is received so the serial data can be checked as well as being somewhat keyboard friendly device.

## command

The command is made of one or more characters that are: alpha (see isalpha() function), '/', or '?'. 

## arg

The argument(s) are made of one or more characters that are: alphanumeric (see isalnum() function). 
