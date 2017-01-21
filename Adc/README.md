# Analog-to-Digital Converter

## Overview

Adc is an interactive command line program that demonstrates control of an ATmega328p Analog-to-Digital Converter from pins PC0 through PC7. 

Note Arduino marked there Uno board as A0 though A5, which is somtimes confused as PA0, I think they wanted it to mean the ADMUX value. 

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Adc$ make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.


``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff.log
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
{"id":{"name":"Adc","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

##  /0/analog? 0..7[,0..7[,0..7[,0..7[,0..7]]]]    

Analog-to-Digital Converter reading from up to 5 ADMUX channels. The reading repeats every 60 Seconds until the Rx buffer gets a character. On RPUno channel 6 is the solar input voltage, channel 7 is the main power node (PWR) voltage, channel 3 is the battery discharge current, channel 2 is the battery charging current, channel 1 and channel 0 each have a current source available (e.g. a 100 Ohm sense resistor with the current source connected to it).  Note ADC4 and ADC5 are used for I2C on RPUno.

``` 
/1/analog? 2,3,6,7
{"CHRG_A":"0.076","DISCHRG_A":"0.000","PV_V":"19.04","PWR_V":"13.54"}
{"CHRG_A":"0.076","DISCHRG_A":"0.000","PV_V":"19.04","PWR_V":"13.54"}
/1/analog? 0,1
{"ADC0":"2.09","ADC1":"2.09"}
{"ADC0":"2.09","ADC1":"2.09"}
/1/analog? 4,5
{"ADC4":"SDA","ADC5":"SCL"}
```

The main power node is a little higher voltage than the battery when charging, and a little less when discharging.
