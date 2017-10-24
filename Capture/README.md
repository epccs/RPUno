# Capture 

## Overview

Timer 1 Input Capture (ICP1) is on RPUno Digital (Wiring) pin 8. 

Referance ATmega328 datasheet 16.6 Input Capture Unit (page 118). A capture copies the value in timer TCNT1 into ICR1, which is a timestamp of the event.

Note the ISR needs about 300 machine cycles to finish. If events happen faster than 300 machine cycles, they may be skipped.

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Capture$ make bootload
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

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/1/id?
{"id":{"name":"Capture","desc":"RPUno (14140^7) Board /w atmega328p","avr-gcc":"4.9"}}
```

## /0/initICP icp1,mode,prescale

Initialize Input Capture ICP1. Set the Input Capture Edge Select mode: rise, fall, both. Set the prescale: 0 = no clock, 1 = MCU clock, 2 = MCU clock/8, 3 = MCU clock/64, 4 = MCU clock/256, 5 = MCU clock/1024, 6 and 7 are for an external (IO5) clock source (see the datasheet). Note that IO5 is used for SHUTDOWN of the solar charge controler.

``` 
/1/initICP icp1,both,1
{"icp1":{"edgSel":"both", "preScalr":"1"}}
```

## /0/count? [icp1]

Return count of ICP1 (ATmega328 pin PB0. Uno pin 8) event captures. A zero means icp1 captures are not happening (because I forgot to connect a sensor).

``` 
/1/count? icp1
{"icp1":{"count":"0"}}
{"icp1":{"count":"0"}}
{"icp1":{"count":"0"}}
```

## /0/capture? [icp1,1..15] 

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

## /0/event? [icp1,1..31] 

return ICP1 event timer values as a 16 bit unsigned integer, which continuously rolls over. The status bit 0 shows rising (0 is falling) edge.

``` 
/0/event? icp1,3
{"icp1":{"count":"2880842","event":"850","status":"1"}}
{"icp1":{"count":"2880841","event":"64779","status":"0"}}
{"icp1":{"count":"2880840","event":"64426","status":"1"}}
```

Perhaps that is confusing. The event with a count of 2880840 had a rising edge and happened before the event with count 2880841 which had a falling edge. The difference of the event times is the number of clocks spent high (64779 - 64426 = 353). And the difference between the middle and most recent events is the number of clocks spent low ((850+2^16) - 64779 = 1607).


## /0/pwm oc2a|oc2b,0..255

Pulse width modulation using OC2A (ATmega328 pin PB3. Uno pin 11) or OC2B (ATmega328 pin PD3. Uno pin 3) can be used to feed the ICP1 input, use a 10k Ohm resistor between them to be safe. Note that timer2 is used with OC2[A|B], while timer1 is needed for ICP1. 

``` 
/0/pwm oc2a,127
{"pwm":{"OCR2A":"127"}}
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
