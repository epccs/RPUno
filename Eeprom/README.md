# AVR EEPROM

## Overview

Eeprom is an interactive command line program that demonstrates the control of an ATmega328p EEPROM.

Depends on avr/eeprom.h from avr-libc <http://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html>.

For how I setup my Makefile toolchain see <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

With optiboot installed run 'make bootload' and it will compile and then flash the MCU the same way Arduino does, but without any Arduino stuff.

``` 
rsutherland@straightneck:~/Samba/RPUno/Eeprom$ make bootload
TBD
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.


``` 
#exit is C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after second charactor of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Eeprom"}}
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