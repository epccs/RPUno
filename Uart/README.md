# AVR  Interrupt-Driven UART with stdio redirect

## Overview

Uart is an interactive command line program that demonstrates stdio redirect of a minimalized interrupt-driven UART core. 

Referance ATmega328 datasheet 20.0 USART0 Universal Synchronous and Asynchronous serial Receiver and Transmitter.

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Uart$ make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Uart","desc":"RPUno Board /w ATmega328p and LT3652","avr-gcc":"4.9"}}
``` 
