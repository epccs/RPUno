# Digital Input/Output

## Overview

Digital is an interactive command line program that demonstrates control of RPUno's Digital input/output from its ATmega328p pins PD3 (IO3), PD4 (IO4), and PB2 through PB4 (nSS, MOSI, and MISO). LED_BUILTIN is maped to PB5 (SCK) and blinks on for a second and off for a second. The RPUno has these I/O's wired to a pluggable onboard connector. They are level converted to 5V and clamped to the on board VIN level (which is the battery voltage when it is connected by way of the TPS3700 used for battery management). Do not apply a voltage from a source that exceeds the on board VIN. A 20mA current source is provided from VIN, its voltage can get nearly to VIN in saturation, but will only allow about 22mA when shorted. The current source is completely safe to use with the digital and allows driving MOSFET gates or a LED string from the battery voltage.

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Digital$ make bootload
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


##  /0/digitalWrite 3|4|10|11|12,HIGH|LOW    

Set the Port Data Register (PORTx) bit that drives the pin or if mode (e.g. Port Input Register bit) is set as an INPUT enables a pullup. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/digitalWrite 3,LOW
{"PD3":"LOW"}
/1/digitalWrite 10,HIGH
{"PB2":"HIGH"}
```


##  /0/digitalToggle 3|4|10|11|12  

Toggle the Port Data Register (PORTx) bit if the Data Direction Register (DDRx) bit is set as an OUTPUT. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/digitalToggle 3
{"PD3":"HIGH"}
/1/digitalToggle 3
{"PD3":"LOW"}
/1/digitalToggle 3
{"PD3":"HIGH"}

```


##  /0/digitalRead? 3|4|10|11|12 

Read the Port Input Register (PINx) bit that was latched during last low edge of the system clock.

``` 
/1/digitalRead? 3
{"PD3":"HIGH"}
/1/digitalRead? 10
{"PB2":"LOW"}

```
I have a 1k Ohm resistor on the nSS pin (digital 10) to ground that causes it to read low, the ATmega328p pull-up is about 60k Ohm.
