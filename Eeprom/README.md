# EEPROM

## Overview

Eeprom is an interactive command line program that demonstrates the control of EEPROM on an ATmega328p.

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Eeprom$ make bootload
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
{"EE[0]":{"dword":"4294967295","r":"4294967295"}}
```
Note: 4294967295 is 0xFFFFFFFF, it is the default for a blank chip.