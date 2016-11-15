# Hardware

## Overview

Eagle Files, BOM, Status, and how to Test.

![Schematic](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/14140,Schematic.png "RPUno Schematic")

# Notes

```
PV Power Point Voltage: 18V7@0�C,16V8@25�C,15V7@40�C,14V5@70�C
PV Watage: 3 thrugh 20W
Max Power Point tracks 36 cell silicon PV with 100k B=4250 Thermistor
Charge Controler type: 12V SLA also tracks with 100k B=4250 Thermistor
Charge Voltage: 13.278V@40�C,13.63V@25�C,14.068V@0�C
Charge rate: about .055A per PV watt at 25�C
MCU type: ATMega328p
MCU clock: 16MHz
MCU Voltage: 5V (e.g. IOREF is 5V)
PULSE CURR SOURCE: 17mA current source for  MT, LT type sensors or to bias hall or VR sensor.
PULSE ALT CURR SOURCE: 10mA source used to feed open collector on hall or VR sensors that will shunt it.
PULSE CURR LOOP TERMINATION: 100 Ohm. Used to bias a NPN transistor that pulls down ICP1.
DIGITAL: five levle protected to IOREF and diode clamped to VIN which may be disconnectd from battery
ANALOG: two inputs with 22 mA current sources from VIN for loop power.
```