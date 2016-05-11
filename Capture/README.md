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

## /0/count?

event count on ICP1 (Uno pin 8). 

JSON {"icp1":{"count":"4294967295"}}

## /0/duty? [1 thru 15] 

return ICP1 timer count delta(s) as a pair of low and high counts from the buffered capture events. These can be used to find the duty or period. The index value is the event count.

JSON {"icp1":{"10000":{"low":"3001","high":"2999"}[,"9999":{"low":"3002","high":"2998"}[,..."9986":{"low":"30016","high":"2984"}]]}}
    
The JSON can be turned into a CSV with the web tool at https://json-csv.com/

## pwm [3 thru 252]

pulse width modulation using OC2A (B3, or pin 11) can be used to feed the ICP1 input. Note that  timer2 is use with OC2A since it is not needed for ICP1, timer1 is needed for use with ICP1.