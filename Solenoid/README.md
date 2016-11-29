# Solenoid Control 

## Overview

Solenoid is an interactive command line program that demonstrates control of the K3 latching solenoid driver board using an ATmega328p pins. 

``` 
RPUno   (digital)   K3 
------------------------
PD3     (IO3)       E3, 
0V       na         nE2
0V       na         nE1
PB2     (nSS/IO10)  A0
PB3     (MOSI/IO11) A1
PB4     (MISO/IO12) A2 
PB5     (SCK/IO13)  LED_BUILTIN
``` 

The RPUno has those I/O's wired to a pluggable onboard connector. They are level converted to 5V so will ouput 4V without a pullup (which is just enough for a minimum high with 74HC logic). The LED_BUILTIN pin blinks on for a second and off for a second when the rpu_address is read over I2C (else it blinks four times as fast). 

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Solenoid$ make bootload
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


##  /0/runtime 1|2|3,runtime_in_sec

Set the solenoid run time (6hr max). 

defaults: delay = 3600 Sec, cycles = 1, flow_stop = not used, delay_start = 0.

``` 
/1/runtime 3,20
{"K3":{"runtime_sec":"20"}}
```


##  /0/delay 1|2|3,delay_in_sec

Set the solenoid delay between runs (24 hr max). 

``` 
/1/delay 3,40
{"K3":{"delay_sec":"40"}}
```


##  /0/run 1|2|3

Start the solenoid operation. 

Note the default value for delay_start is zero, and for cycles it is one.

After a solenoid is finished the lowest value solenoid that is ready to run will do so. 

``` 
/1/run 3
{"K3":{"delay_start_sec":"0","runtime_sec":"20", "delay_sec":"40","cycles":"1"}}
```
