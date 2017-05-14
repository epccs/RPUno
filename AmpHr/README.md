# Amp-Hour Counter

## Overview

Note: values are reported in Amp-Seconds. To convert the reported value to the typical notion divide by 3600. 

On RPUno ADC2 is connected to a high side current sense for charging and ADC3 is connected to one for discharging. The [Adc] firmware is used to take the direct readings. This interactive command line program demonstrates how the ATmega328p can be used to estimate the solar power that has been stored, or discharged. 

[Adc]: ../Adc

Since the mcu is good with integers (not floats) the units will be mA*mSec as integers. 


# Start of Day 

The [day-night][../DayNight] state machine is used to load and run EEPROM values after the morning debounce. This means the valves will start to cycle after the delay_start time has elapsed each morning.


# Firmware Upload

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
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff.json
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

Report the charge gain and loss since the day-night state machine switched from morning debounce to the day state, where charge and discharge accumulators were zeroed. Also, report the remaining or difference between the charge and discharge accumulators that was present at the time the values were last zeroed. 

``` 
/1/charge?
{"CHRG_ASec":"0.67","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"9.12"}
{"CHRG_ASec":"5.11","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"69.12"}
{"CHRG_ASec":"9.56","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"129.12"}
{"CHRG_ASec":"14.00","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"189.12"}
{"CHRG_ASec":"18.44","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"249.12"}
{"CHRG_ASec":"22.88","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"309.12"}
{"CHRG_ASec":"27.32","DCHRG_ASec":"0.00","RMNG_ASec":"0.00","ACCUM_Sec":"369.12"}
/1/analog? 2,3,6,7
{"CHRG_A":"0.076","DISCHRG_A":"0.000","PV_V":"16.86","PWR_V":"13.54"}
# note an LED is blinking
```

## [/0/day?](../DayNight#0day)


## [/0/analog? 0..7[,0..7[,0..7[,0..7[,0..7]]]]](../Adc#0analog-0707070707)
