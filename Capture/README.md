# Capture 

## Overview

Timer 1 Input Capture (ICP1) is on RPUno Digital I/O 8. 

Referance datasheet Input Capture Unit. A capture copies the value in timer TCNT1 into ICR1, which is a timestamp of the event.

Note the ISR needs about 300 machine cycles to finish. If events happen faster than 300 machine cycles, they may be skipped.

## Firmware Upload

With a serial port connection (see BOOTLOAD_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUno/
cd /RPUno/Capture
make bootload
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
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff,capture.log
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../lib/rpu_mgr.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated) as an adddress. The ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.

## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/1/id?
{"id":{"name":"Capture","desc":"RPUno (14140^9) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

## /0/initICP icp1,mode,prescale

Initialize Input Capture ICP1. Set the Input Capture Edge Select mode: rise, fall, both. Set the prescale: 0 = no clock, 1 = MCU clock, 2 = MCU clock/8, 3 = MCU clock/64, 4 = MCU clock/256 and 5 = MCU clock/1024.

``` 
/1/initICP icp1,both,1
{"icp1":{"edgSel":"both", "preScalr":"1"}}
```

## /0/toggleicp icp1,0..255

Toggle the CS_ICP1_EN a number of times to test the ICP1 input. The capture time stamps are the software loop times.

``` 
/1/toggleicp icp1,31
{"icp1":{"toggle_CS_ICP1_EN":"31"}}
``` 

Note: CS_ICP1_EN starts with a LOW and ends with a LOW, so the count may be off by one.


## /0/count? \[icp1\]

Return count of ICP1 (ATmega328p pin PB0. Digital 8) event captures. A zero means icp1 captures are not happening.

``` 
/1/count? icp1
{"icp1":{"count":"31"}}
```

## /0/capture? \[icp1,1..15\] 

return ICP1 timer delta(s) as a pair of low and high timing values from the buffered capture events. These can be used to find the duty or period. The count is the number of event captures. Note that a group of consecutive captures is, in fact, continous, there are no timing gaps. The resolution of continuous timing events can approach that of the stability of the timing sources (both timing sources have to be stable to gain maximum resolution). Pulse interpolation is a way of using a fast time source to measure a slow time source, but the fast time source occurs in buckets (e.g. quantum) and to measure something with those buckets to 100ppm requires 10k of them. 

[HT]: https://github.com/epccs/LoopSensor/tree/master/HT

``` 
/1/capture? icp1,10
{"icp1":{"count":"31","low":"658","high":"585","status":"1"}}
{"icp1":{"count":"29","low":"674","high":"652","status":"1"}}
{"icp1":{"count":"27","low":"773","high":"585","status":"1"}}
{"icp1":{"count":"25","low":"674","high":"585","status":"1"}}
{"icp1":{"count":"23","low":"674","high":"585","status":"1"}}
{"icp1":{"count":"21","low":"674","high":"585","status":"1"}}
{"icp1":{"count":"19","low":"674","high":"585","status":"1"}}
{"icp1":{"count":"17","low":"674","high":"585","status":"1"}}
{"icp1":{"count":"15","low":"674","high":"585","status":"1"}}
{"icp1":{"count":"13","low":"674","high":"585","status":"1"}}
```

It takes three events to aggregate the data for a capture report. The status reports only the most recent event status since the other two events can be inferred. 

## /0/event? \[icp1,1..31\] 

return ICP1 event timer values as a 16 bit unsigned integer, which continuously rolls over. The status bit 0 shows rising (0 is falling) edge.

``` 
/1/event? icp1,21
{"icp1":{"count":"31","event":"2610","status":"1"}}
{"icp1":{"count":"30","event":"1952","status":"0"}}
{"icp1":{"count":"29","event":"1367","status":"1"}}
{"icp1":{"count":"28","event":"693","status":"0"}}
{"icp1":{"count":"27","event":"41","status":"1"}}
{"icp1":{"count":"26","event":"64804","status":"0"}}
{"icp1":{"count":"25","event":"64219","status":"1"}}
{"icp1":{"count":"24","event":"63545","status":"0"}}
{"icp1":{"count":"23","event":"62960","status":"1"}}
{"icp1":{"count":"22","event":"62286","status":"0"}}
{"icp1":{"count":"21","event":"61701","status":"1"}}
{"icp1":{"count":"20","event":"61027","status":"0"}}
{"icp1":{"count":"19","event":"60442","status":"1"}}
{"icp1":{"count":"18","event":"59768","status":"0"}}
{"icp1":{"count":"17","event":"59183","status":"1"}}
{"icp1":{"count":"16","event":"58509","status":"0"}}
{"icp1":{"count":"15","event":"57924","status":"1"}}
{"icp1":{"count":"14","event":"57250","status":"0"}}
{"icp1":{"count":"13","event":"56665","status":"1"}}
{"icp1":{"count":"12","event":"55991","status":"0"}}
{"icp1":{"count":"11","event":"55406","status":"1"}}
```


# Notes

## Pulse Skipping

With an HT as the temperature goes up the high count goes down for example near Room temp I see 

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
