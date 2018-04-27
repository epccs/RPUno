# Amp-Hour Counter

## Overview

This is not working on RPUno^7 since it does not have solar power for the day-nigh state machine.

On RPUno ADC2 is connected to a high side current sense for charging and ADC3 is connected to one for discharging. The [Adc] firmware is used to take the direct readings. This interactive command line program demonstrates how the ATmega328p can be used to estimate the solar power that has been stored, or discharged. 

[Adc]: ../Adc


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


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../Uart/id.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.


## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/0/id?
{"id":{"name":"Adc","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

##  /0/charge?

Report the charge gain and loss since the day-night state machine switched from morning debounce to the day state, where charge and discharge accumulators were zeroed. Also, report the remaining or difference between the charge and discharge accumulators that was present at the time the values were last zeroed. 

``` 
/1/charge?
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"0.03","RMNG_mAHr":"0.00","ACCUM_Sec":"6.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"0.27","RMNG_mAHr":"0.00","ACCUM_Sec":"66.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"0.51","RMNG_mAHr":"0.00","ACCUM_Sec":"126.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"0.74","RMNG_mAHr":"0.00","ACCUM_Sec":"186.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"0.98","RMNG_mAHr":"0.00","ACCUM_Sec":"246.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"1.22","RMNG_mAHr":"0.00","ACCUM_Sec":"306.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"1.46","RMNG_mAHr":"0.00","ACCUM_Sec":"366.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"1.70","RMNG_mAHr":"0.00","ACCUM_Sec":"426.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"1.94","RMNG_mAHr":"0.00","ACCUM_Sec":"486.24"}
{"CHRG_mAHr":"0.00","DCHRG_mAHr":"2.18","RMNG_mAHr":"0.00","ACCUM_Sec":"546.24"}
/1/analog? 2,3,6,7
{"CHRG_A":"0.000","DISCHRG_A":"0.026","PV_V":"0.29","PWR_V":"13.17"}
{"CHRG_A":"0.000","DISCHRG_A":"0.024","PV_V":"0.29","PWR_V":"13.18"}
{"CHRG_A":"0.000","DISCHRG_A":"0.022","PV_V":"0.23","PWR_V":"13.14"}
```

Note that a LED was blinking when the ADC value was taken, also it is not a buffered value so the UART has just finished sending the last block of data (e.g. "\"DISCHRG_A\":") and the reading from ADC is a buffered value that likely includes the transceivers current usage. The accumulated charge seems to show that about 15mA is the average used over the first 66 Seconds. 15mA is not what the MCU is using it is what the SMPS is converting from 13V into 5V for the MCU (e.g. about 30mA).


## [/0/day?](../DayNight#0day)


## [/0/analog? 0..7\[,0..7\[,0..7\[,0..7\[,0..7\]\]\]\]](../Adc#0analog-0707070707)
