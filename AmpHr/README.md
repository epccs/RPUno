# Amp-Hour Counter

## Overview

On RPUno ADC2 is connected to a high side current sense for charging and ADC3 is connected to one for discharging. The [Adc] firmware is used to take the direct readings. This interactive command line program demonstrates how the ATmega328p can be used to estimate the solar power that has been stored, or discharged. 

[Adc]: ../Adc

Since the mcu is good with integers (not floats) the units will be mA*mSec as integers. 

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/AmpHr$ make bootload
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

##  /0/charge?

Report the charge gain since the day-night state machine switched from morning debounce to the day state. The charge and discharge values are zeroed at the start of the day as soon as charging is detected. 

``` 
/1/charge?
{"CHRG_mAmSec":"123450","DAY_mSec":"12345"}
```

##  /0/discharge?

Report the charge loss since the day-night state machine switched from morning to day and began charging. 

``` 
/1/discharge?
{"DCHRG_mAmSec":"123450","DAY_mSec":"12345"}
```

##  /0/remaining?

Report the difference between charge and discharge that was present at the time the values were last zeroed. This can be a negative value, but that means a power deficet for the day (where is that credit card... choped up!, opps).

``` 
/1/remaining?
{"RMNG_mAmSec":"123450"}
```
