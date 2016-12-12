# Day-Night State Machine

## Overview

Use RPUno's photovoltaic voltage (PV_V on ADC channel 6) to approximately tell when it is day or night. 


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
{"id":{"name":"DayNight","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```


##  /0/day?

Report status of the state machine.

``` 
/1/day?
{"day_state":"DAY"}
... 11 hr
{"day_state":"DAY"}
{"day_state":"EVENING"}
... 15 min
{"day_state":"EVENING"}
{"day_state":"NIGHT"}
... 12 hr
{"day_state":"NIGHT"}
{"day_state":"MORNING"}
... 15 min
{"day_state":"MORNING"}
WaterTheGarden
{"day_state":"DAY"}
... 20 hr (e.g. using a power supply)
{"day_state":"DAY"}
{"day_state":"FAIL"}



```



