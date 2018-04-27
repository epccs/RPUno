# HT 

## Overview

Combines Capture and Adc to track tempeture, humidity, and solar power. 

Referance [Capture](../Capture) and [Adc](../Adc).

The capture ISR will skip events when less than 300 machine cycles are available between them, see [Capture](../Capture). This adds an Adc ISR which may cause skipping at higher machine cycle counts. 

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@straightneck:~/Samba/RPUno/Ht$ make bootload
...
avr-size -C --mcu=atmega328p Ht.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:   13208 bytes (40.3% Full)
(.text + .data + .bootloader)

Data:        428 bytes (20.9% Full)
(.data + .bss + .noinit)

...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff,ht.log
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../lib/rpu_mgr.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated) as an adddress. The ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.

## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/0/id?
{"id":{"name":"Capture"}}
```

## /0/initICP icp1,(rise|fall|both),(0..7)

Initialize Input Capture ICP1. Set the Input Capture Edge Select mode. Set the prescale (0 = no clock, 1 = MCU clock, 2 = MCU clock/8... see datasheet for others. Note some are not based on the MCU clock)

``` 
/0/initICP icp1,both,1
{"icp1":{"edgSel":"both", "preScalr":"1"}}
```

## /0/count? \[icp1\]

Return count of ICP1 (ATmega328 pin PB0. Uno pin 8) event captures. A zero means icp1 captures are not happening (because I forgot to connect a sensor).

``` 
/0/count? icp1
{"icp1":{"count":"0"}}
{"icp1":{"count":"0"}}
{"icp1":{"count":"0"}}
```

## /0/capture? \[icp1,1..15\] 

return ICP1 timer delta(s) as a pair of low and high timing values from the buffered capture events. These can be used to find the duty or period. The count is the number of event captures (now I've connected a [HT](http://epccs.org/indexes/Board/HT/) sensor). Note that a group of consecutive captures is, in fact, continous, there are no timing gaps. The resolution of continuous timing events can approach that of the stability of the timing sources (both timing sources have to be stable to gain maximum resolution). Pulse interpolation is a way of using a fast time source to measure a slow time source, but the fast time source occurs in buckets (e.g. quantum) and to measure something with those buckets to 100ppm requires 10k of them. 

``` 
/0/capture? icp1,3
{"icp1":{"count":"11642055","low":"1606","high":"349","status":"0"}}
{"icp1":{"count":"11642053","low":"1609","high":"349","status":"0"}}
{"icp1":{"count":"11642051","low":"1603","high":"349","status":"0"}}
{"icp1":{"count":"11805739","low":"1606","high":"349","status":"0"}}
{"icp1":{"count":"11805737","low":"1603","high":"348","status":"0"}}
{"icp1":{"count":"11805735","low":"1607","high":"349","status":"0"}}
{"icp1":{"count":"11969471","low":"1607","high":"348","status":"1"}}
{"icp1":{"count":"11969469","low":"1603","high":"349","status":"1"}}
{"icp1":{"count":"11969467","low":"1607","high":"349","status":"1"}}
```

It takes three events to aggregate the data for a capture report. The status reports only the most recent event status since the other two events can be inferred. 

## /0/event? \[icp1,1..31\] 

return ICP1 event timer values as a 16 bit unsign integer, which continuously rolls over. The status bit 0 shows rising (0 is falling) edge.

``` 
/0/event? icp1,3
{"icp1":{"count":"2880842","event":"850","status":"1"}}
{"icp1":{"count":"2880841","event":"64779","status":"0"}}
{"icp1":{"count":"2880840","event":"64426","status":"1"}}
```

Perhaps that is confusing. The event with a count of 2880840 had a rising edge and happened before the event with count 2880841 which had a falling edge. The difference of the event times is the number of clocks spent high (64779 - 64426 = 353). And the difference between the middle and most recent events is the number of clocks spent low ((850+2^16) - 64779 = 1607).

## /0/ht? 

Return capture count, capture sample size, Temperature and RH using ICP1. Additionaly solar input voltage and battery voltage from ADC.

``` 
/0/ht?
{"count":"2642422","smpl_sz":"15","deg_f":"83.44","%RH":"10.98","PV_IN":"28.00""PWR":" 6.75"}
{"count":"2658000","smpl_sz":"15","deg_f":"83.45","%RH":"10.89","PV_IN":"28.00""PWR":" 6.75"}
{"count":"2673576","smpl_sz":"15","deg_f":"83.44","%RH":"11.13","PV_IN":"28.00""PWR":" 6.75"}

``` 


# Notes

## Pulse Skipping

With an HT sensor, as the temperature goes up the high count goes down for example near Room temp I see 

``` 
{"icp1":{"count":"519107161","low":"1669","high":"550","status":"0"}}
``` 

As the heat is turned up the capture looks like 

``` 
{"icp1":{"count":"514717561","low":"1646","high":"199","status":"0"}}
``` 

and some start to show skipping

``` 
{"icp1":{"count":"514717563","low":"1645","high":"2045","status":"0"}}
``` 

The short pulse is the one that skips because the ISR is not able to change edge detection in time to see the falling edge of the short high pulse so it has to wait for the next falling edge. It takes about 50 machine cycles to enter an interrupt (since all registers used have to be preserved) and then a similar amount of time to restore the interrupted process to its original state. 

Unfortunately, it is the other ISR's like Timer 0 (zero) used for millis() timing that is causing the skips. I need those interrupts to run so this is going to be something I will live with. The minimum pulse width that will ensure a capture is about 50 counts more than the longest interrupt, I am not seeing skips on pulses above 300 counts (18.75 uSec). The program will still gather data down to about 130 counts but some skipping will show up need tossed (they are sampling errors, e.g. outlier). 

Some sensors like flow meters do not need both edges tracked, so the capture can be initialized to track one edge, which may allow pulse events to be gathered at up to about 30kHz. The "capture?" command will not work with only one edge.
