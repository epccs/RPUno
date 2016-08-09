# AVR  Interrupt-Driven UART with stdio redirect

## Overview

Uart is an interactive command line program that demonstrates stdio redirect of an interrupt-driven UART. 

Referance ATmega328 datasheet 20.0 USART0 Universal Synchronous and Asynchronous serial Receiver and Transmitter.

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

Minimalized Interrupt-driven UART code based in part on <https://github.com/hwstar/avr-uart>, with added streams based in part on <https://github.com/andygock/avr-uart>

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Uart$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit is C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after the second character of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Uart","desc":"RPUno Board /w ATmega328p and LT3652","avr-gcc":"4.9"}}
``` 
