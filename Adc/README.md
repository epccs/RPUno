# Capture 

## Overview

Adc is an interactive command line program that demonstrates control of an ATmega328p (e.g. Arduino Uno) Analog-to-Digital Converter from pins PC0 through PC7. Warning Arduino marked there board as A0 though A5, which is somtimes confused as PA0, I think they wanted it to mean the ADMUX value. 

Makefile based. Without the extras to increase transparency. The tools work in the way they were designed. 

# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after second charactor of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Adc"}}
```

##  /0/analog? 0..7[,0..7[,0..7[,0..7[,0..7]]]]    

Analog-to-Digital Converter reading from up to 5 ADMUX channels. The reading repeats until the Rx buffer gets a character. On RPUno channel 6 is the solar input voltage, and channel 7 is the battery voltage.

``` 
/0/analog? 5,6,7
{"ADC5":"0.00","PV_IN":"23.95","PWR":"6.86"}
{"ADC5":"0.00","PV_IN":"23.95","PWR":"6.86"}
{"ADC5":"0.00","PV_IN":"23.95","PWR":"6.86"}
{"ADC5":"0.00","PV_IN":"23.95","PWR":"6.86"}
```