# Digital Input/Output

## Overview

Digital is an interactive command line program that demonstrates control of RPUno's Digital input/output from its ATmega328p pins PD3 (IO3), PD4 (IO4), and PB2 through PB4 (nSS, MOSI, and MISO). LED_BUILTIN is maped to PB5 (SCK) and blinks on for a second and off for a second. The RPUno has these I/O's wired to a pluggable onboard connector. They are level converted to 5V and clamped to the on board VIN level (which is the battery voltage when it is connected by way of the TPS3700 used for battery management). Do not apply a voltage from a source that exceeds the on board VIN. A 20mA current source is provided from VIN, its voltage can get nearly to VIN in saturation, but will only allow about 22mA when shorted. The current source is completely safe to use with the digital and allows driving MOSFET gates or a LED string from the battery voltage.

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Digital$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o digital.o digital.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Uart/id.o ../Uart/id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/twi.o ../lib/twi.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Digital.map  -Wl,--gc-sections  -Wl,-u,vfprintf -lprintf_flt -lm -mmcu=atmega328p main.o digital.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/twi.o ../lib/parse.o -o Digital.elf
avr-size -C --mcu=atmega328p Digital.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:   10928 bytes (33.3% Full)
(.text + .data + .bootloader)

Data:        419 bytes (20.5% Full)
(.data + .bss + .noinit)


rm -f Digital.o main.o digital.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/twi.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex Digital.elf Digital.hex
rm -f Digital.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:Digital.hex

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
avrdude: reading input file "Digital.hex"
avrdude: input file Digital.hex auto detected as Intel Hex
avrdude: writing flash (10928 bytes):

Writing | ################################################## | 100% 1.55s

avrdude: 10928 bytes of flash written
avrdude: verifying flash memory against Digital.hex:
avrdude: load data flash data from input file Digital.hex:
avrdude: input file Digital.hex auto detected as Intel Hex
avrdude: input file Digital.hex contains 10928 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 1.12s

avrdude: verifying ...
avrdude: 10928 bytes of flash verified

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

or log the terminal session

``` 
script -f -c "picocom -b 115200 /dev/ttyUSB0" stuff.log
``` 


# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after the second character of a new line. 


## /[rpu_address]/[command [arg]]

rpu_address is taken from the I2C address 0x29 (e.g. ../Uart/id.h get_Rpu_address() ). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

The LED_BUILTIN is bliked fast (twice per second) if the I2C address is not found, also the rpu_address defaults to '0'. 

Commands and their arguments follow.


## /0/id? [name|desc|avr-gcc]

Identify is from ../Uart/id.h Id().

``` 
/1/id?
{"id":{"name":"Digital","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

##  /0/pinMode 3|4|10|11|12,INPUT|OUTPUT    

Set the Data Direction Register (DDRx) bit that sets a pin as INPUT or OUTPUT.

``` 
/1/pinMode 3,OUTPUT
{"PD3":"OUTPUT"}
/1/pinMode 10,INPUT
{"PB2":"INPUT"}
```


##  /0/digitalWrite 3|4|10|11|12|13,HIGH|LOW    

Set the Port Data Register (PORTx) bit that drives the pin or if mode (e.g. Port Input Register bit) is set as an INPUT enables a pullup. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/digitalWrite 3,LOW
{"PD3":"LOW"}
/1/digitalWrite 10,HIGH
{"PB2":"HIGH"}
```


##  /0/digitalToggle 3|4|10|11|12|13  

Toggle the Port Data Register (PORTx) bit if the Data Direction Register (DDRx) bit is set as an OUTPUT. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/digitalToggle 3
{"PD3":"HIGH"}
/1/digitalToggle 3
{"PD3":"LOW"}
/1/digitalToggle 3
{"PD3":"HIGH"}

```


##  /0/digitalRead? 3|4|10|11|12|13  

Read the Port Input Register (PINx) bit that was latched during last low edge of the system clock.

``` 
/1/digitalRead? 3
{"PD3":"HIGH"}
/1/digitalRead? 10
{"PB2":"HIGH"}

```
I have a 1k Ohm resistor on the nSS pin (digital 10) to ground that causes it to read low, the ATmega328p pull-up is about 60k Ohm.
