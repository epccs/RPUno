# I2C-Debug 

## Overview

2-wire Serial Interface (TWI, a.k.a. I2C) uses pins with SDA and SCL functions. 

Referance datasheet 2-wire Serial Interface. 

Arduino has twi.c and twi.h which was done in C, I did some modification and updates for avr-libc and avr-gcc (4.9). It uses an ASYNC ISR but does block (busy wait) while reading or writing.

## Firmware Upload

With a serial port connection (set the BOOTLOAD_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
sudo apt-get install make git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno/
cd /RPUno/i2c-debug
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk).

``` 
#exit picocom with C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../lib/rpu_mgr.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated) as an adddress. The ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.

## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/0/id?
{"id":{"name":"I2Cdebug^1","desc":"RPUno (14140^9) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

## /0/iscan?

Scan of I2C bus shows all 7 bit devices found. I have a PCA9554 at 0x38 and an 24C02AN eeprom at 0x50.

``` 
/1/iscan?
{"scan":[{"addr":"0x29"},{"addr":"0x38"},{"addr":"0x50"}]}
```

Note: the address does not include the Read/Write bit. 


## /0/iaddr 0..127

Set the I2C address 

``` 
/1/iaddr 56
{"address":"0x38"}
```

Note: Set it with the decimel value, it will return the hex value. This value is used durring read and write, it will also reset the xtBuffer.


## /0/ibuff 0..255\[,0..255\[,0..255\[,0..255\[,0..255\]\]\]\]

Add up to five bytes to I2C transmit buffer. JSON reply is the full buffer. 

``` 
/1/ibuff 3,0
{"txBuffer":["data":"0x3","data":"0x0"]}
``` 


## /0/ibuff?

Show buffer data.

``` 
/1/ibuff?
{"txBuffer":["data":"0x3","data":"0x0"]}
``` 

## /0/iwrite

Attempt to become master and write the txBuffer bytes to I2C address (PCA9554). The txBuffer will clear if write was a success.

``` 
/1/iaddr 56
{"address":"0x38"}
/1/ibuff 3,0
{"txBuffer":["data":"0x3","data":"0x0"]}
/1/iwrite
{"returnCode":"success"}
``` 

## /0/iread? \[1..32\]

If txBuffer has values, attempt to become master and write the byte(s) in buffer (e.g. command byte) to I2C address (example is for a PCA9554) without a stop condition. The txBuffer will clear if write was a success. Then send a repeated Start condition, followed by address and obtain readings into rxBuffer.

``` 
/1/iaddr 56
{"address":"0x38"}
/1/ibuff 3
{"txBuffer":["data":"0x3"}
/1/iread? 1
{"rxBuffer":[{"data":"0xFF"}]}
``` 

Note the PCA9554 has been power cycled in this example, so the reading is the default from register 3.


# PCA9554 Example

Load the PCA9554 configuration register 3 (DDR) with zero to set the port as output. Then alternate register 1 (the output port) with 85 and 170 to toggle its output pins. 

``` 
/1/iaddr 56
{"address":"0x38"}
/1/ibuff 3,0
{"txBuffer":["data":"0x3","data":"0x0"]}
/1/iwrite
{"returnCode":"success"}
/1/ibuff 1,170
{"txBuffer":[{"data":"0x1"},{"data":"0xAA"}]}
/1/iwrite
{"returnCode":"success"}
/1/ibuff 1,85
{"txBuffer":[{"data":"0x1"},{"data":"0x55"}]}
/1/iwrite
{"returnCode":"success"}
``` 

# HTU21D Example 

``` 
/1/scan?
{"scan":[{"addr":"0x29"},{"addr":"0x40"}]}
``` 

Command 0xE3 measures temperature, the clock is streached until data is ready.

``` 
/1/iaddr 64
{"address":"0x40"}
/1/ibuff 227
{"txBuffer":["data":"0xE3"]}
/1/iread? 3
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
/1/iaddr 64
{"address":"0x40"}
/1/ibuff 229
{"txBuffer":["data":"0xE5"]}
/1/read? 3
{"rxBuffer":[{"data":"0x65"},{"data":"0x96"},{"data":"0xBC"}]}
``` 

The first two bytes are the temperature data. The last two bits of LSB are status (ignore or mask them off). Some Python gives the result in deg C.

``` 
Stmp = 0x6596 & 0xFFFC
RH = -6 + 125 * Stmp / (2**16)
RH
43.6
``` 
