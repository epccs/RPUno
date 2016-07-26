# I2C-Debug 

## Overview

2-wire Serial Interface (TWI, a.k.a. I2C) uses pins with SDA and SCL functions (ATmega328p have it on C4 and C5). 

Referance ATmega328 datasheet 22. 2-wire Serial Interface (page 209). 

Arduino has twi.c and twi.h which are done in C, I did some modification and updates for avr-libc and avr-gcc (4.9). It uses an ASYNC ISR but has options to block (busy wait) while reading or writing. I am using the blocking options, but need to research non-blocking.

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

With optiboot installed run 'make bootload' and it will compile and then flash the MCU the same way Arduino does, but without any Arduino stuff.

``` 
rsutherland@straightneck:~/Samba/RPUno/i2c-debug$ make bootload

``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 115200 /dev/ttyUSB0" stuff,i2c-debug.log
``` 


# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after second charactor of a new line. 

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

The first two bytes are the temperature data. The last two bits of LSB are status (ignore or mask them off). Some Python gives the result in
°C.

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

The first two bytes are the temperature data. The last two bits of LSB are status (ignore or mask them off). Some Python gives the result in
°C.

``` 
Stmp = 0x6596 & 0xFFFC
RH = -6 + 125 * Stmp / (2**16)
RH
43.6
``` 