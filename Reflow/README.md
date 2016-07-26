# Reflow

## Overview

Controls a 20A Solid State Relay (SSR) and a buzzer. 

Every two seconds a byte is taken from EEPROM to control the PWM rate of the SSR for the next two seconds. The thermocouple is not used for direct feedback control since I would not feel safe walking away from such a setup. I use the thermocouple to get the profile correct, but once set the profile is hard wired into the control loop. If I made an error in the software it will be repeated each time, it will not be hidden by a chance mistake in PID logic. For this to work the starting condition needs to be the same (near room temperature) for each run, I find it takes about ten minutes.

Reads a Fluke 80TK Thermocouple Module (1mV/deg F) on channel zero. 

![Profile](https://raw.githubusercontent.com/epccs/RPUno/master/Reflow/profile/walmartBD,160622.png "Profile for Black & Decker Model NO. TO1303SB")

Do not modify the oven, if you do and it burns down your house the insurance may be able to deny payment. Turn the knobs so it is always on (as shown in the image) and set the temperature high enough that it will not turn off the oven. Now the Solid State Relay (SSR) can modulate power to the heating elements, the modulation does not need to be fast since they have a five-second thermal response, in fact, a two second PWM is fine. The SSR needs to be placed in a certified electrical enclosure to make sure insurance will pay (I'm not an expert, and there are always more rules). 

![Setup](https://raw.githubusercontent.com/epccs/RPUno/master/Reflow/profile/WalmartBD,TO1303SB.jpg "Setup of Black & Decker Model NO. TO1303SB")

A 255 value in EEPROM will turn on the buzzer on pin 6 for two seconds, while two consecutive values will terminate the profile. The program will otherwise run to the end of EEPROM memory.

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the Atmega328 board run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@straightneck:~/Samba/RPUno/Reflow$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o reflow.o reflow.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o ../Eeprom/ee.o ../Eeprom/ee.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o ../Uart/id.o ../Uart/id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o ../lib/adc.o ../lib/adc.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=9600UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Reflow.map  -Wl,--gc-sections  -Wl,-u,vfprintf -lprintf_flt -lm -mmcu=atmega328p main.o reflow.o ../Eeprom/ee.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/parse.o -o Reflow.elf
avr-size -C --mcu=atmega328p Reflow.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:    9676 bytes (29.5% Full)
(.text + .data + .bootloader)

Data:        189 bytes (9.2% Full)
(.data + .bss + .noinit)


rm -f Reflow.o main.o reflow.o ../Eeprom/ee.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex Reflow.elf Reflow.hex
rm -f Reflow.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:Reflow.hex

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
avrdude: reading input file "Reflow.hex"
avrdude: input file Reflow.hex auto detected as Intel Hex
avrdude: writing flash (9676 bytes):

Writing | ################################################## | 100% 1.44s

avrdude: 9676 bytes of flash written
avrdude: verifying flash memory against Reflow.hex:
avrdude: load data flash data from input file Reflow.hex:
avrdude: input file Reflow.hex auto detected as Intel Hex
avrdude: input file Reflow.hex contains 9676 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 1.14s

avrdude: verifying ...
avrdude: 9676 bytes of flash verified

avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 9600 /dev/ttyUSB0
``` 

log a terminal session to check the profile

``` 
script -f -c "picocom -b 9600 /dev/ttyUSB0" stuff.log
``` 


# Commands

Commands are interactive over the serial interface at 9600 baud rate. The echo will start after second charactor of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Reflow","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

## /0/reflow?

Start the reflow profile and read the Fluke 80TK Thermocouple Module set to Fahrenheit scale on analog channel zero.

``` 
/0/reflow?
{"millis":"10","pwm":"255","deg_c":"26.11"}
{"millis":"2010","pwm":"50","deg_c":"26.11"}
```

##  /0/ee? 0..1023

Return the EEPROM value at address. This checks if eeprom_is_ready() befor trying to read the EEPROM, if it is not ready the program loops back through the round robin where a received charactor may terminate the command. 

``` 
/0/ee? 0
{"EE[0]":"255"}
```

##  /0/ee 0..1023,0..255

Write the value given as argument one to the address given as Argument zero. This checks if eeprom_is_ready() befor trying to write to the EEPROM, if it is not ready the program loops back through the round robin where a received charactor may terminate the command. If the command is terminated the write may not occure. The JSON response is a read of the EEPROM. 

Warning writing EEPROM can lead to device failure, it is only rated for 100k write cycles.

``` 
/0/ee 0,255
{"EE[0]":"255"}
```


# Profile

The profile sub folder has Python3 program(s) that load the profile using ee and ee? commands. 


# Notes

I Used a Fluke 80TK Thermocouple Module set to Fahrenheit scale so the output voltage runs up to almost .5V, and set the ADC to use the internal 1.1V bandgap referance and that means nearly half that ADC scale is used. 

