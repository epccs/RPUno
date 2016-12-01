# RPUno has an ATmega328p with on-board solar charge controller

![Status](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/status_icon.png "Status")

From <https://github.com/epccs/RPUno/>

## Overview

The Board has an easy to use MCU and LT3652 solar charge controller, it does not have USB or an LED.

![Schematic](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/Documents/14140,Schematic.png "RPUno Schematic")

![Bottom](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/Documents/14140,Bottom.png "RPUno Board Bottom")

![Top](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/Documents/14140,Top.png "RPUno Board Top")

[Forum](http://rpubus.org/bb/viewforum.php?f=6)

[HackaDay](https://hackaday.io/project/12784-rpuno)

## Example with RPU BUS (RS-422)

A wired serial bus that allows multiple microcontroller boards to be connected to a host port. An RPUftdi shield is on one of the MCU boards near the host computer (USB can be run for 2 or 3 meters). The other boards use an RPUadpt shield and are connected as a daisy-chain up to perhaps 1000 meters. 

![MultiDrop](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/Documents/MultiDrop.png "RPUno MultiDrop")

Wired connections are inherently secure. However, if the host provides a raw connection (e.g. telnet, or a raw console server) any chance of security will be compromised. I use SSH to the host for remote access and run picocom within the SSH session. SSL options could also work but they will fail when the upstream network is down. The problem with OpenWrt devices (e.g. Yun) is they end up having to expose a raw serial terminal to the AVR since they are too dumb to run the AVR toolchain and must offload the task of building and uploading firmware. A Pi Zero can run the toolchain within its environment and when connected instead of an OpenWrt device the raw serial terminal need not be exposed on the network as the Pi can build and upload firmware directly (just like a host computer on RPUftdi). 

## AVR toolchain

The core files are in the /lib folder while each example has its own Makefile.

    * sudo apt-get install [gcc-avr](http://packages.ubuntu.com/search?keywords=gcc-avr)
    * sudo apt-get install [binutils-avr](http://packages.ubuntu.com/search?keywords=binutils-avr)
    * sudo apt-get install [gdb-avr](http://packages.ubuntu.com/search?keywords=gdb-avr)
    * sudo apt-get install [avr-libc](http://packages.ubuntu.com/search?keywords=avr-libc)
    * sudo apt-get install [avrdude](http://packages.ubuntu.com/search?keywords=avrdude)
    