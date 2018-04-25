# Self-Test

## Overview

Check RPUno Board Functions, runs once after a reset and then loops in a pass/fail section.

ICP1 (PL) has a 100 Ohm resistor to 0V on board.

Voltage references are saved in EEPROM for use with Adc and other applications. Measure the +5V supply accurately and set the REF_EXTERN_AVCC value in the main.c file. The band-gap reference is calculated and also saved.

The wiring has red and green LED that blink to indicate test status.

## Wiring Needed for RPUno

![Wiring](./Setup/SelfTestWiring.png)

Note: blocking diode with LED is to prevent damage when connected wrong.


## Power Supply

Connect a power supply with CV and CC mode. Set CC at 150mA then increase CV to 12.8V.


## Firmware Upload

With a serial port setup for serial bootloading (see BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should build the code and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUno/
cd /RPUno/SelfTest
make bootload
...
avrdude done.  Thank you.
make clean
``` 

Now connect with picocom (exit is C-a, C-x). 

``` 
picocom -b 38400 /dev/ttyUSB0
picocom v2.2
...
Terminal ready
RPUno Self Test date: Apr 23 2018
avr-gcc --version: 5.4.0
I2C provided address 0x31 from RPUadpt serial bus manager
adc reading for PWR_V: 355
PWR at: 12.722 V
ADC0 R1 /W all CS off: 0.000 V
ADC1 at ICP1 TERM /w all CS off: 0.000 V
ADC2 at GN LED /w DIO13 sinking and all CS off: 0.000 V
ADC3 at YE LED /w DIO10 sinking and all CS off: 0.000 V
ICP1 input should be HIGH with 0mA loop current: 1 
CS0 on R1: 0.022 A
DIO11 shunting CS0: 0.014 A
CS1 source on R1: 0.022 A
   ADC0 reading used to calculate ref_intern_1v1_uV: 689 A
   calculated ref_intern_1v1_uV: 1082791 uV
REF_EXTERN_AVCC old value found in eeprom: 5007000 uV
REF_INTERN_1V1 old value found in eeprom: 1082204 uV
REF_EXTERN_AVCC from eeprom is same
PWR_I at no load use INTERNAL_1V1: 0.012 A
CS2 source on R1: 0.022 A
Yellow LED D4 fwd /w CS2 V: 2.112 V
DIO10 shunting CS2: 0.015 A
CS3 source on R1: 0.022 A
DIO12 shunting CS3: 0.015 A
CS_ICP1 in UUT PL input: 0.018 A
Green LED D1 fwd /w CS_ICP1 V: 2.147 V
ICP1 /w 17mA on termination reads: 0 
DIO13 shunting CS_ICP1: 0.015 A
[PASS]
```

