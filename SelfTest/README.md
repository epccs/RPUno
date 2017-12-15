# Self-Test

## Overview

Check RPUno Board Functions, runs once after a reset and then loops in a blink led section.

Current sources feed a 50 Ohm resistor and that is measured with ADC0. One of the 22mA digital current source connect to DIO13 and DIO12 befor going through a yellow LED and then feeding into the 50 Ohm resistor. The other 22mA digital current source connect to DIO3 and DIO10 befor going through a yellow LED and then feeding into the 50 Ohm resistor. The 22mA ADC0 current source connects to DIO11 befor going through a red LED and feeding into the 50 Ohm resistor. 

ICP1 has a 100 Ohm resistor on board and a 10mA current source jumper that feeds it, the voltage is measured with ADC1. Additionaly a 17mA current source is connected to DIO4 and feeds the ICP1 resistor through a green LED. 

Voltage references are saved in EEPROM for use with Adc and other applications. Measure the +5V supply accurately and set the REF_EXTERN_AVCC value in the main.c file. The band-gap reference is calculated and also saved.

The red and green LED are used to indicate the test status.

## Wiring Needed for RPUno

![Wiring](./Setup/SelfTestWiring.png)


## Power Supply

Connect a power supply with CV and CC mode. Set CC at 300mA then increase CV to 12.8V.


## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
make bootload
...
avrdude done.  Thank you.
make clean
rm -f SelfTest.hex SelfTest.map
``` 

Now connect with picocom (exit is C-a, C-x). 

``` 
picocom -b 38400 /dev/ttyUSB0
picocom v1.7
...
Terminal ready
Self Test date: Dec 14 2017
I2C provided address 0x31 from serial bus manager
PWR_I with CS_EN==off: 0.016 A
PWR at: 12.834 V
ADC0 without curr in R1: 0.000 V
ADC1 without curr in PL for ICP1: 0.000 V
ICP1's PL input has 0mA input and reads: 1
22MA_A0 source on R1: 0.022 A
22MA_A1 source on R1: 0.022 A
ICP1's PL input with 10mA: 0.010 A
   ADC1 reading used to calculate ref_intern_1v1_uV: 940 A
   calculated ref_intern_1v1_uV: 1076316 uV
REF_EXTERN_AVCC old value was in eeprom: 5008600 uV
REF_INTERN_1V1 old value was in eeprom: 1070621 uV
REF_EXTERN_AVCC saved in eeprom: 5008600 uV
REF_INTERN_1V1 saved in eeprom: 1076316 uV
ICP1 /w 10mA on plug termination reads: 0
PWR_I with CS_EN==on: 0.078 A
22MA_DIO11 curr source on R1: 0.022 A
DIO12 shunting 22MA_DIO11: 0.007 A
DIO13 shunting 22MA_DIO11: 0.007 A
22MA_DIO3 curr source on R1: 0.022 A
DIO10 shunting 22MA_DIO3: 0.007 A
DIO3 shunting 22MA_DIO3: 0.007 A
ICP1 17mA curr source on ICP1's PL plug: 0.018 A
[PASS]
``` 
