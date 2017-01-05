# I2C-Debug 

## Overview

2-wire Serial Interface (TWI, a.k.a. I2C) uses pins with SDA and SCL functions (ATmega328p have it on C4 and C5). 

Referance ATmega328 datasheet 22. 2-wire Serial Interface (page 209). 

Arduino has twi.c and twi.h which are done in C, I did some modification and updates for avr-libc and avr-gcc (4.9). It uses an ASYNC ISR but does block (busy wait) while reading or writing.

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/i2c-debug$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o i2c-scan.o i2c-scan.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o i2c-cmd.o i2c-cmd.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Uart/id.o ../Uart/id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/twi.o ../lib/twi.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/adc.o ../lib/adc.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,I2c-debug.map  -Wl,--gc-sections  -mmcu=atmega328p main.o i2c-scan.o i2c-cmd.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/twi.o ../lib/adc.o ../lib/parse.o -o I2c-debug.elf
avr-size -C --mcu=atmega328p I2c-debug.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:    8686 bytes (26.5% Full)
(.text + .data + .bootloader)

Data:        373 bytes (18.2% Full)
(.data + .bss + .noinit)


rm -f I2c-debug.o main.o i2c-scan.o i2c-cmd.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/twi.o ../lib/adc.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex I2c-debug.elf I2c-debug.hex
rm -f I2c-debug.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:I2c-debug.hex

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
avrdude: reading input file "I2c-debug.hex"
avrdude: input file I2c-debug.hex auto detected as Intel Hex
avrdude: writing flash (8686 bytes):

Writing | ################################################## | 100% 1.22s

avrdude: 8686 bytes of flash written
avrdude: verifying flash memory against I2c-debug.hex:
avrdude: load data flash data from input file I2c-debug.hex:
avrdude: input file I2c-debug.hex auto detected as Intel Hex
avrdude: input file I2c-debug.hex contains 8686 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.89s

avrdude: verifying ...
avrdude: 8686 bytes of flash verified

avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. Samba file share is used for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 115200 /dev/ttyUSB0" stuff,i2c-debug.log
``` 


# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after the second character of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"I2Cdebug","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

## /0/scan?

Scan of I2C bus shows all 7 bit devices found. I have a PCA9554 at 0x38 and an 24C02AN eeprom at 0x50.

``` 
/0/scan?
{"scan":[{"addr":"0x38"},{"addr":"0x50"}]}
```

Note: the address does not include the Read/Write bit. 


## /0/address (0..127)

Set the I2C address 

``` 
/0/address 56
{"address":"0x38"}
```

Note: Set it with the decimel value, it will return the hex value. This value is used durring read and write, it will also reset the xtBuffer.


## /0/buffer (0..255)[,(0..255)[,(0..255)[,(0..255)[,(0..255)]]]]

Add up to five bytes to I2C transmit buffer. JSON reply is the full buffer. 

``` 
/0/buffer 3,0
{"txBuffer":["data":"0x3","data":"0x0"]}
``` 


## /0/buffer?

Show buffer data.

``` 
/0/buffer?
{"txBuffer":["data":"0x3","data":"0x0"]}
``` 

## /0/write

Attempt to become master and write the txBuffer bytes to I2C address (PCA9554). The txBuffer will clear if write was a success.

``` 
/0/address 56
{"address":"0x38"}
/0/buffer 3,0
{"txBuffer":["data":"0x3","data":"0x0"]}
/0/write
{"returnCode":"success"}
``` 

## /0/read? [1..32]

If buffer has values, attempt to become master and write the byte(s) in buffer (e.g. command byte) to I2C address (exampel is for a PCA9554) without a stop condition. The txBuffer will clear if write was a success. Then send a repeated Start condition, followed by address and obtain readings.

``` 
/0/address 56
{"address":"0x38"}
/0/buffer 3
{"txBuffer":["data":"0x3"}
/0/read? 1
{"rxBuffer":[{"data":"0xFF"}]}
``` 

Note the PCA9554 has been power cycled in this example, so the reading is the default from register 3.


# PCA9554 Example

Load the PCA9554 configuration register 3 (DDR) with zero to set the port as output. Then alternate register 1 (the output port) with 85 and 170 to toggle its output pins. 

``` 
/0/address 56
{"address":"0x38"}
/0/buffer 3,0
{"buffer":["data":"0x3","data":"0x0"]}
/0/write
{"returnCode":"success"}
/0/buffer 1,170
{"buffer":[{"data":"0x1"},{"data":"0xAA"}]}
/0/write
{"returnCode":"success"}
/0/buffer 1,85
{"buffer":[{"data":"0x1"},{"data":"0x55"}]}
/0/write
{"returnCode":"success"}
``` 

# HTU21D Example 

``` 
/0/scan?
{"scan":[{"addr":"0x40"}]}
``` 

Command 0xE3 measures temperature, the clock is streached until data is ready.

``` 
/0/address 64
{"address":"0x40"}
/0/buffer 227
{"buffer":["data":"0xE3"]}
/0/read? 3
{"rxBuffer":[{"data":"0x6A"},{"data":"0xC"},{"data":"0xC6"}]}
``` 

The first two bytes are the temperature data. The last two bits of LSB are status (ignore or mask them off). Some Python gives the result in deg C.

``` 
Stmp = 0x6A0C
Temp = -46.85 + 175.72 * Stmp / (2**16)
Temp
25.9
``` 

Command 0xE5 measures humidity, again the clock is streached until data is ready.

``` 
/0/address 64
{"address":"0x40"}
/0/buffer 229
{"buffer":["data":"0xE5"]}
/0/read? 3
{"rxBuffer":[{"data":"0x65"},{"data":"0x96"},{"data":"0xBC"}]}
``` 

The first two bytes are the temperature data. The last two bits of LSB are status (ignore or mask them off). Some Python gives the result in deg C.

``` 
Stmp = 0x6596 & 0xFFFC
RH = -6 + 125 * Stmp / (2**16)
RH
43.6
``` 
