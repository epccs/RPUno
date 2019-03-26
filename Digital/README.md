# Digital Input/Output

## Overview

Digital is an interactive command line program that demonstrates control of the Digital input/output from ATmega328p pins PB2..PB5 (10..13) and PC0..PC3 (14..17).

## Firmware Upload

With a serial port connection (see BOOTLOAD_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
sudo apt-get install make git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno/
cd /RPUno/Digital
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk).

``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. ../Uart/id.h get_Rpu_address() ). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

The STATUS_LED is bliked fast (twice per second) if the I2C address is not found, also the rpu_address defaults to '0'. 

Commands and their arguments follow.


## /0/id? \[name|desc|avr-gcc\]

Identify is from ../Uart/id.h Id().

``` 
/1/id?
{"id":{"name":"Digital","desc":"RPUno (14140^9) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

##  /0/pMod 10..17,INPUT|OUTPUT    

Set the Data Direction Register (DDRx) bit that sets a pin as INPUT or OUTPUT.

``` 
/1/pMod 10,OUTPUT
{"PB2":"OUTPUT"}
/1/pMod 14,INPUT
{"PC0":"INPUT"}
```


##  /0/dWrt 10..17,HIGH|LOW    

Set the Port Data Register (PORTx) bit that drives the pin or if mode (e.g. Port Input Register bit) is set as an INPUT enables a pullup. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/dWrt 10,LOW
{"PB2":"LOW"}
/1/dWrt 14,HIGH
{"PC0":"LOW"}
```

Pin 14 is set as INPUT so it is not in the push-pull mode, the HIGH turns on a weak pullup and a LOW turns off the pullup. Since I have the SelfTest setup connected the ADC0 has 33.3 Ohm to ground and reads back a LOW.


##  /0/dTog 10..17

Toggle the Port Data Register (PORTx) bit if the Data Direction Register (DDRx) bit is set as an OUTPUT. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/dTog 10
{"PC0":"HIGH"}
/1/dTog 10
{"PC0":"LOW"}
```

The board has a 127 Ohm output resistor that goes to the SelfTest wiring thus about 23mA is in the yellow LED when digital pin 10 is HIGH. 


##  /0/dRe? 10..17

Read the Port Input Register (PINx) bit that was latched during last low edge of the system clock.

``` 
/1/dRe? 14
{"PC0":"LOW"}
/1/dRe? 10
{"PB2":"LOW"}
```
