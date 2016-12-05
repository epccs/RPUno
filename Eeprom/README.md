# EEPROM

## Overview

Eeprom is an interactive command line program that demonstrates the control of EEPROM on an ATmega328p.

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Eeprom$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ee.o ee.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Uart/id.o ../Uart/id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/twi.o ../lib/twi.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/adc.o ../lib/adc.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Eeprom.map  -Wl,--gc-sections  -Wl,-u,vfprintf -lprintf_flt -lm -mmcu=atmega328p main.o ee.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/twi.o ../lib/adc.o ../lib/parse.o -o Eeprom.elf
avr-size -C --mcu=atmega328p Eeprom.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:    9048 bytes (27.6% Full)
(.text + .data + .bootloader)

Data:        303 bytes (14.8% Full)
(.data + .bss + .noinit)


rm -f Eeprom.o main.o ee.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/twi.o ../lib/adc.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex Eeprom.elf Eeprom.hex
rm -f Eeprom.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:Eeprom.hex

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
avrdude: reading input file "Eeprom.hex"
avrdude: input file Eeprom.hex auto detected as Intel Hex
avrdude: writing flash (9048 bytes):

Writing | ################################################## | 100% 1.28s

avrdude: 9048 bytes of flash written
avrdude: verifying flash memory against Eeprom.hex:
avrdude: load data flash data from input file Eeprom.hex:
avrdude: input file Eeprom.hex auto detected as Intel Hex
avrdude: input file Eeprom.hex contains 9048 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.92s

avrdude: verifying ...
avrdude: 9048 bytes of flash verified

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

# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after the second character of a new line. 


## /[rpu_address]/[command [arg]]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../Uart/id.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.


## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Eeprom","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

##  /0/ee? address[,type]

Return the EEPROM value at address [0..1023 on RPUno]. Type is UINT8, UINT16 or UINT32. Default type is UINT8. This checks if eeprom_is_ready() befor trying to read the EEPROM, if not ready it loops back through the program. 

``` 
/1/ee? 0
{"EE[0]":{"r":"255"}}
/1/ee? 1
{"EE[1]":{"r":"128"}}
/1/ee? 2
{"EE[2]":{"r":"255"}}
/1/ee? 3,UINT8
{"EE[3]":{"r":"32"}}
/1/ee? 2,UINT16
{"EE[2]":{"r":"65535"}}
/1/ee? 0,UINT32
{"EE[0]":{"r":"553615615"}}
```

Note: 553615615 is 0x20FF80FF, so the numbers are packed little endian by the gcc compiler (AVR itself has no endianness)


##  /0/ee address,value[,type]

Write the value to the address [0..1023 on RPUno] as type. Type is Type is UINT8, UINT16 or UINT32. Default type is UINT8. This checks if eeprom_is_ready() befor trying to read the EEPROM, if not ready it loops back through the program. The JSON response is a read of the EEPROM. 

__Warning__ writing EEPROM can lead to device failure, it is only rated for 100k write cycles.

``` 
/1/ee 0,255
{"EE[0]":{"byte":"255","r":"255"}} 
/1/ee 1,128,UINT8
{"EE[1]":{"byte":"128","r":"128"}}
/1/ee 2,65535,UINT16
{"EE[2]":{"word":"65535","r":"65535"}}
/1/ee 0,4294967295,UINT32
{"EE[0]":"4294967295"}
```
Note: 4294967295 is 0xFFFFFFFF, it is the default for a blank chip.