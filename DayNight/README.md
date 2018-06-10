# Day-Night State Machine

## Overview

Use a red LED's photovoltaic voltage to approximately tell if it is day or night. 

Note: use the Day_AttachDayWork() and Night_AttachWork() functions to register a callback that will be run at the start of each day. This framework is how I debuged the day-night stat machine.


## Sensor

CREE C503B-RCN-CW0Z0AA1 with a 100k load resistor. The LED works somewhat like a solar cell but it develops up to about 1.6V with red light and shorter wave lenghts. 


## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
sudo apt-get install git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno/
cd /RPUno/DayNight
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

rpu_address is taken from the I2C address 0x29 (e.g. ../Uart/id.h get_Rpu_address() ). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice.

The STATUS_LED is bliked fast (twice per second) if the I2C address is not found, also the rpu_address defaults to '0'. 

Commands and their arguments follow.


## /0/id? \[name|desc|avr-gcc\]

Identify is from ../Uart/id.h Id().

``` 
/1/id?
{"id":{"name":"DayNight","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```


## /0/day?

Report status of the state machine.

``` 
/1/day?
{"day_state":"DAY"}
... 11 hr
{"day_state":"DAY"}
{"day_state":"EVENING"}
... 15 min
{"day_state":"EVENING"}
TurnOnLED's
{"day_state":"NIGHT"}
... 12 hr
{"day_state":"NIGHT"}
{"day_state":"MORNING"}
... 15 min
{"day_state":"MORNING"}
WaterTheGarden
{"day_state":"DAY"}
... 20 hr (e.g. using an indoor lamp)
{"day_state":"DAY"}
{"day_state":"FAIL"}
```


# Notes

Some readings on ADC2 with the red LED and a 100k Ohm burden

```
#Morning 8:30AM 6/10/18 battery was near full at start
/1/altcnt?
{"alt_count":"2"}
/1/analog? 6,7,2
{"PWR_I":"0.023","PWR_V":"13.55","ADC2":"0.21"}
#Morning 9:30AM 6/10/18
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"0.25"}
#Morning 10:40AM 6/10/18
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"0.51"}
#Peak was at about 11:50AM 6/10/18, sample every 20 sec
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"0.60"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"0.60"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"0.60"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"0.61"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"0.61"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"0.62"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"0.63"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"0.64"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"0.65"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.66"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.67"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.68"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.68"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"0.69"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"0.71"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.72"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.73"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.74"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.75"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.77"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.78"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.81"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.82"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.85"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.86"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.88"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.89"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.91"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.92"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.93"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"0.95"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.96"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.97"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"0.99"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.00"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.03"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.02"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.04"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.04"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.04"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.07"}
{"PWR_I":"0.033","PWR_V":"13.40","ADC2":"1.07"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.10"}
{"PWR_I":"0.033","PWR_V":"13.62","ADC2":"1.11"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.12"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.14"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.16"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.17"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.19"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.20"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.22"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.23"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.24"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.25"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.26"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.28"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.29"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.30"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.32"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.33"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.33"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.34"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.35"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.36"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.36"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.37"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.37"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.38"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.38"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.38"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.39"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.39"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.39"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.40"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.40"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.40"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.40"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.41"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.41"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.41"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.42"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.42"}
{"PWR_I":"0.023","PWR_V":"13.47","ADC2":"1.42"}
{"PWR_I":"0.022","PWR_V":"13.47","ADC2":"1.42"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.42"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.42"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.023","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.023","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.023","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.023","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.023","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.024","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.47","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.44"}
{"PWR_I":"0.022","PWR_V":"13.40","ADC2":"1.43"}
{"PWR_I":"0.022","PWR_V":"13.51","ADC2":"1.43"}
{"PWR_I":"0.022","PWR_V":"13.55","ADC2":"1.45"}
{"PWR_I":"0.022","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.022","PWR_V":"13.62","ADC2":"1.44"}
{"PWR_I":"0.023","PWR_V":"13.51","ADC2":"1.45"}
{"PWR_I":"0.023","PWR_V":"13.51","ADC2":"1.44"}
{"PWR_I":"0.023","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.023","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.023","PWR_V":"13.55","ADC2":"1.44"}
{"PWR_I":"0.023","PWR_V":"13.55","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.44"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.44"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.43"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.43"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.43"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.42"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.42"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.42"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.42"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.42"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.41"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.41"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.41"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.41"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.40"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"1.40"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.40"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.39"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.39"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.38"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.38"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.38"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.37"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.36"}
{"PWR_I":"0.033","PWR_V":"13.51","ADC2":"1.35"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.34"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.33"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.33"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.32"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"1.31"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.29"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.27"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.25"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"1.21"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"1.18"}
{"PWR_I":"0.033","PWR_V":"13.55","ADC2":"1.14"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.12"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.08"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.04"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"1.00"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.97"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.94"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.90"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.88"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.84"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.82"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.79"}
{"PWR_I":"0.032","PWR_V":"13.58","ADC2":"0.77"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.75"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.72"}
{"PWR_I":"0.033","PWR_V":"13.58","ADC2":"0.70"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.69"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.67"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.66"}
{"PWR_I":"0.032","PWR_V":"13.47","ADC2":"0.65"}
{"PWR_I":"0.032","PWR_V":"13.51","ADC2":"0.65"}
{"PWR_I":"0.032","PWR_V":"13.55","ADC2":"0.64"}
```

