# Capture 

## Overview
Timer 1 Input Capture (ICP1) is on Arduino Uno pin 8. 
All files compile with Arduino IDE 
load this sketch (firmware) on a Uno 

Referance ATmega328 datasheet 16.6 Input Capture Unit (page 118 of 8271G–AVR–02/2013)
A capture copies the value in timer TCNT1 into ICR1, which is a timestamp of the event.

Limitations exist due to the ISR, which needs about 300 machine cycles to finish. 
If events happen faster than 300 machine cycles, they may be skipped.

Use serial monitor with line ending set to Newline, baud rate is 9600.

## Commands

###count?

pulse or cycle count on ICP1 (Uno pin 8). 

### duty? [l2h|h2l]

    active (or inactive) pulse duty measured in clock counts on ICP1 (Uno pin 8).
    "l2h" is inactive time, falling edge to rising edge.
    "h2l" is active time, rising edge to falling edge.
    The duty cycle is typically a percentage of the period signal is active 
    rather than clock counts.

### period? [rise|fall]

    pulse period measured in clock counts on ICP1 (Uno pin 8).
    rising (falling) edge to rising (falling) edge period
    period is the time it takes a signal to repeat.

### pwm [3 thru 252]

    pulse width modulation pin 11 with the duty provided in argument.
    Since timer1 is used for signal capture, timer3 is free to use
    for pwm on pin 11. The duty has a minimum active (inactive) time
    or the ISR may skip pulses (about 300 counts).