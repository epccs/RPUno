# Serial parsing for textual commands and arguments

## Overview

Demonstration of command parsing with the redirected stdio functions (e.g. printf() and simular)  from avr-libc. 

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

``` 
rsutherland@straightneck:~/Samba/RPUno/Parsing$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Parsing.map  -Wl,--gc-sections  -mmcu=atmega328p main.o ../lib/uart.o ../lib/parse.o -o Parsing.elf
avr-size Parsing.elf
   text    data     bss     dec     hex filename
   4520      16     123    4659    1233 Parsing.elf
rm -f Parsing.o main.o ../lib/uart.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex Parsing.elf Parsing.hex
rm -f Parsing.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:Parsing.hex

avrdude: Version 6.2
         Copyright (c) 2000-2005 Brian Dean, http://www.bdmicro.com/
         Copyright (c) 2007-2014 Joerg Wunsch

         System wide configuration file is "/etc/avrdude.conf"
         User configuration file is "/home/rsutherland/.avrduderc"
         User configuration file does not exist or is not a regular file, skipping

         Using Port                    : /dev/ttyUSB0
         Using Programmer              : arduino
         Overriding Baud Rate          : 115200
         AVR Part                      : ATmega328P
         Chip Erase delay              : 9000 us
         PAGEL                         : PD7
         BS2                           : PC2
         RESET disposition             : dedicated
         RETRY pulse                   : SCK
         serial program mode           : yes
         parallel program mode         : yes
         Timeout                       : 200
         StabDelay                     : 100
         CmdexeDelay                   : 25
         SyncLoops                     : 32
         ByteDelay                     : 0
         PollIndex                     : 3
         PollValue                     : 0x53
         Memory Detail                 :

                                  Block Poll               Page                       Polled
           Memory Type Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack
           ----------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------
           eeprom        65    20     4    0 no       1024    4      0  3600  3600 0xff 0xff
           flash         65     6   128    0 yes     32768  128    256  4500  4500 0xff 0xff
           lfuse          0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           hfuse          0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           efuse          0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           lock           0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           calibration    0     0     0    0 no          1    0      0     0     0 0x00 0x00
           signature      0     0     0    0 no          3    0      0     0     0 0x00 0x00

         Programmer Type : Arduino
         Description     : Arduino
         Hardware Version: 3
         Firmware Version: 4.4
         Vtarget         : 0.3 V
         Varef           : 0.3 V
         Oscillator      : 28.800 kHz
         SCK period      : 3.3 us

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e950f (probably m328p)
avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: NOTE: "flash" memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: reading input file "Parsing.hex"
avrdude: input file Parsing.hex auto detected as Intel Hex
avrdude: writing flash (4536 bytes):

Writing | ################################################## | 100% 0.68s

avrdude: 4536 bytes of flash written
avrdude: verifying flash memory against Parsing.hex:
avrdude: load data flash data from input file Parsing.hex:
avrdude: input file Parsing.hex auto detected as Intel Hex
avrdude: input file Parsing.hex contains 4536 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.54s

avrdude: verifying ...
avrdude: 4536 bytes of flash verified

avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit is C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

# Command Line

/addr/command [arg[,arg[,arg[,arg[,arg]]]]]

The command line is structured like MQTT topics with some optional arguments 


## addr

The address is a single character. That is to say, "/0/id?" is at address '0', where "/id?" is a command. Echo of the command occurs after the address is received so the serial data can be checked as well as being somewhat keyboard friendly device.

## command

The command is made of one or more characters that are: alpha (see isalpha() function), '/', or '?'. 

## arg

The argument(s) are made of one or more characters that are: alphanumeric (see isalnum() function). 
