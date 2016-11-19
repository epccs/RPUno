# Serial Bootloader

## Overview

AVR microcontrollers are usually programmed through an In-circuit Serial Programmer (ICSP/ISP), however with this firmware loaded in your microcontroller programing can be done over a serial interface (TX/RX) without the ICSP programmer. This is called serial bootloading. 

Unfortunalty to install a serial bootloader requires an ICSP tool. Arduino has an ArduinoISP.ino <../ArduinoISP> file that I load into a seperate ATmega328 board to use as an ICSP tool.

It is a two step process. First run "make fuse" which will program LOCKBIT=3F to open the bootloader area and also LOW=FF, HIGH=DE, EXTENDED=05. The fuse setting will switch operation form the internal oscilator to an exteranl 16MHz crystal, if it is missing the MCU will be locked up. The second step is "make isp" which will load the bootloader and program LOCKBIT=2F, which should prohibit the Store Program Memory (SPM) instruction from running in the boot flash section. 

    WARNING: programing these fuse valuse will lock up the MCU when the crystal is missing or damaged.

