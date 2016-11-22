# RPUno has an ATmega328p with on-board solar charge controller

![Status](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/status_icon.png "Status")

From <https://github.com/epccs/RPUno/>

## Overview

The Board has an easy to use MCU and LT3652 solar charge controller, it does not have USB or an LED.

![Schematic](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/14140,Schematic.png "RPUno Schematic")

![Bottom](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/14140,Bottom.png "RPUno Board Bottom")

![Top](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/14140,Top.png "RPUno Board Top")

[Forum](http://rpubus.org/bb/viewforum.php?f=6)

[HackaDay](https://hackaday.io/project/12784-rpuno)

## Example with RPU BUS (RS-422)

A wired serial bus that allows multiple microcontroller boards to be connected to a host port. An RPUftdi shield is on one of the MCU boards near the host computer (USB can be run for 2 or 3 meters). The other boards use an RPUadpt shield and are connected as a daisy-chain across the property. 

![MultiDrop](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/MultiDrop.png "RPUno MultiDrop")

Wired connections are inherently secure. Resist the temptation to have the host provide a raw connection (e.g. telnet, or a terminal server for example). Have the host use SSH for local remote access, SSL options could also work but they fail when the upstream network is down. 

## AVR toolchain

The core files are in the /lib folder while each example has its own Makefile.

    * sudo apt-get install [gcc-avr](http://packages.ubuntu.com/search?keywords=gcc-avr)
    * sudo apt-get install [binutils-avr](http://packages.ubuntu.com/search?keywords=binutils-avr)
    * sudo apt-get install [gdb-avr](http://packages.ubuntu.com/search?keywords=gdb-avr)
    * sudo apt-get install [avr-libc](http://packages.ubuntu.com/search?keywords=avr-libc)
    * sudo apt-get install [avrdude](http://packages.ubuntu.com/search?keywords=avrdude)
    