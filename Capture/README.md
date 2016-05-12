# Capture 

## Overview

Timer 1 Input Capture (ICP1) is on the Arduino Uno pin 8. 

Referance ATmega328 datasheet 16.6 Input Capture Unit (page 118). A capture copies the value in timer TCNT1 into ICR1, which is a timestamp of the event.

Note the ISR needs about 300 machine cycles to finish. If events happen faster than 300 machine cycles, they may be skipped.

Makefile based. Without the extras to increase transparency. The tools work in the way they were designed. 

# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after second charactor of a new line line. 

## /0/id? [name|desc|avr-gcc]

identify 

## /0/count? [icp1]

Count of ICP1 (ATmega328 pin PB0. Uno pin 8) event captures. 

JSON {"icp1":{"count":"4294967295"}}

## /0/capture? [icp1,1..15] 

return ICP1 timer count delta(s) as a pair of low and high counts from the buffered capture events. These can be used to find the duty or period. The index value is the count of event captures.

JSON {"icp1":{"10000":{"low":"3001","high":"2999"}[,"9999":{"low":"3002","high":"2998"}[,..."9986":{"low":"30016","high":"2984"}]]}}
    
The JSON can be turned into a CSV with the web tool at https://json-csv.com/

## pwm oc2a|oc2b,0..255

Pulse width modulation using OC2A (ATmega328 pin PB3. Uno pin 11) or OC2B (ATmega328 pin PD3. Uno pin 3) can be used to feed the ICP1 input. Note that timer2 is used with OC2[A|B], while timer1 is needed for ICP1.