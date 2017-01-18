# Hardware

## Overview

This board is solar powered and has an ATmega328p. The Capture (ICP1) hardware is connected to an inverting open collector transistor that pulls down the ICP1 pin when current (e.g. 10mA) is flowing through a 100 Ohm sense resistor. The captured value is accurate to within one crystal count of the event (e.g. pulse edge caused by the transition of 3mA to 10mA from a current loop or a sensor output that goes open after it was a shunt for the 10mA source). This captured value is an excellent (e.g. crystal is 30ppm + drift) method for data acquisition from a flow meter or for other pulse interpolation task. The board also has six digital interfaces with voltage level conversion up to the board's internal supply voltage (VIN on schematic), and two analog inputs with current sources for two current loops. The ATmega328p can be programmed with the AVR toolchain on Debian which ends up in Ubuntu (upload from a host on [RPUftdi] shield), Raspbian (upload from a Pi Zero on a [RPUpi] shield), and others.

Bootloader options include [optiboot] and [xboot]. Uploading through a bootloader eliminates fuse setting errors and there are few register settings that can block an upload accidentally (some other bootloaders don't clear the watchdog and can get stuck in a loop). This has given the feel of robustness during my software development experience.

[optiboot]: https://github.com/Optiboot/optiboot
[xboot]: https://github.com/alexforencich/xboot
[RPUpi]: https://github.com/epccs/RPUpi/
[RPUftdi]: https://github.com/epccs/RPUftdi/

## Inputs/Outputs/Functions

```
        ATmega328p is a minimalistic easy to use microcontroller
        12V SLA with an LT3652 solar charge controller 
        High side battery current sensing ADC2 (Charging) and ADC3 (Discharging).
        Vin power will automatically disconnect when the battery is low.
        Six pluggable digital input/outputs (DIO 3,4,10,11,12,13) with level conversion clamped to Vin.
        Digital interface has a 22 mA current source from Vin (do not use the battery directly)
        Pulse input for capacitive sensors HT, LT, MT, PT, VR, or OneShot.
        Optinal 10mA source for use with flow meters that have an open collector output.
        Two Analog Loops each with current sources from Vin.
        Power off control to FT/Pulse (IO9) and Vin to Shield (IO2).
        MCU power (+5V) is converted with an SMPS from the VIN output of the solar charge controller.
```

## Uses

```
        Flow Meter Data Acquisition using Capture Hardware (ICP1).
        20W PV is able to keep a Pi Zero with a WiFi dongle (see RPUpi shield) running through the night.
        VIN to the shield can be powered down while the RPUno continues to run (also see RPUpi).
```

## Notice

```
        ADC4 and ADC5 are used for I2C exclusively and not connected to the analog header.
        AREF from ATmega328p is not connected to the header.
        3V3 is not present on the board, the header pin is not connected.
```


# Table Of Contents

1. [Status](#status)
2. [Design](#design)
3. [Bill of Materials](#bill-of-materials)
4. [How To Use](#how-to-use)


# Status

![Status](./status_icon.png "RPUno Status")

```
        ^6  Done: Design, Layout, BOM,
            WIP: Review*,
            Todo: Order Boards, Assembly, Testing, Evaluation.
            *during review the Design may change without changing the revision.
            use ADC6 to measure the raw PV on Anode of dark blocking diode.  

        ^5  location: 2016-12-18 Test Bench /w an RPUpi^1, start power management testing
                      2017-1-1 This^5 had ADC7 parts (and BOM) changed to measre battery.
                      2017-1-5 This^5 had ADC6 hacked to measure raw PV.

        ^4  location: 2016-12-1 SWall Encl /w K3^0, RPUadpt^4, SLP003-12U, 12V battery.
                      2017-1-1 This^4 had ADC7 parts changed to measre battery.
                      2017-1-1 This^4 had ADC6 hacked to measure raw PV.
                      2017-1-5 RPUadpt^4 had ICP1 hacked open.
                      
        ^2  location: 2016-8-1 SEPortch Encl /w CCtest^0, RPUadpt^2, SLP003-12U, 6V SLA.
                      2017-1-17 running but not doing anything useful
```

Debugging and fixing problems i.e. [Schooling](./Schooling/)

Setup and methods used for [Evaluation](./Evaluation/)


# Design

The board is 0.063 thick, FR4, two layer, 1 oz copper with ENIG (gold) finish.

![Top](./Documents/14140,Top.png "RPUno Top")
![TAssy](./Documents/14140,TAssy.jpg "RPUno Top Assy")
![Bottom](./Documents/14140,Bottom.png "RPUno Bottom")
![BAssy](./Documents/14140,BAssy.jpg "RPUno Bottom Assy")

## Electrical Parameters (Typical)

```
PV Power Point Voltage: 18V7@0ºC,16V8@25ºC,15V7@40ºC,14V5@70ºC
PV Watage: 3 thrugh 20W
Max Power Point tracks 36 cell silicon PV with 100k B=4250 Thermistor
Charge Controler type: 12V SLA also tracks with 100k B=4250 Thermistor
Charge Voltage: 13.278V@40ºC,13.63V@25ºC,14.068V@0ºC
Charge rate: about .055A per PV watt at 25ºC
MCU type: ATMega328p
MCU clock: 16MHz
MCU Voltage: 5V (e.g. IOREF is 5V)
PULSE CURR SOURCE: 17mA current source for  MT, LT type sensors or to bias hall or VR sensor.
PULSE ALT CURR SOURCE: 10mA source used to feed open collector on hall or VR sensors that will shunt it.
PULSE CURR LOOP TERMINATION: 100 Ohm. Used to bias a NPN transistor that pulls down ICP1.
DIGITAL: five levle protected to IOREF and diode clamped to VIN which may be disconnectd from battery
ANALOG: two inputs with 20 mA current sources (from VIN) for loop sensor power.
```

## Operating Temperature

```
        Charge control will shut down when outside 0 to 40 ºC
        This is OSH so refer to the parts used for storage and operation limits.
```

## Mounting

```
        DIN rail
```

## Electrical Schematic

![Schematic](./Documents/14140,Schematic.png "RPUno Schematic")

## Testing

Check correct assembly and function with [Testing] [./Testing/]


# Bill of Materials

Import the [BOM](./Design/14140,BOM.csv) into LibreOffice Calc (or Excel) with semicolon separated values, or use a text editor.


# How To Use

Fully charge the SLA battery that will be used, this step will help prevent frustration caused by waiting for the RPUno to charge it.

Connect the application electronics (e.g. flow meter, Digital, and Analog) and check the connections. Then plug in the battery (which will remain disconnected). Next plug in the solar (PV) power, once the PV voltage is enough to enable the charger it will connect to the battery, and start charging (though nothing is visable, and this has caused some frustration). When the battery voltage is over 13.1V it will connect to the on board VIN and power-up. Buffered power ensures hiccup free operation. 

In some ways, this board is like an Arduino Uno, but many functions are dedicated on the board.  Three digital lines (IO5, IO6, IO7) are connected to the solar charge controller. Two more digital lines (IO2, IO9) are used to control power to the SHLD_VIN and flow sensor current sources. Two analog lines (ADC4, ADC5) are dedicated to I2C (and not wired to the analog header). While four more analog lines (ADC7, ADC6, ADC3, ADC2) are used to measure the battery PWR voltage, PV_IN voltage, CHRG, and DISCHRG.

Without connecting anything more than a battery and a solar panel there is a lot of firmware options to consider. How well suited this board is for a task is not easy to answer. 

The [Solenoid] firmware is looking fairly interesting, it is a solenoid control state machine with some of the states using a timer with a programmed value. [Solenoid] also reads the flow sensor at specific states in order to accumulate the flow count (pulse count from flow meter) through the valve. It operates the valves several times with a delay between each operation. This should allow the drip irrigation to be done several times (e.g. 10 times with 5-minute watering and 30-minute delays between watering) during the day, rather than in one big pool (for 50 minutes). The idea is to give the vegetables a chance to use the water before it sinks bellow where their roots can access it. In my porous soil, the water sinks in fairly quick, I am looking forward to giving this idea a try. 

[Solenoid]: ../Solenoid