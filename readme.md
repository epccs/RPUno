# RPUno 

From <https://github.com/epccs/RPUno/>

## Overview

This no frills programmable board has an Arduino-style header and an easy to use ATmega328p microcontroller. The embedded LT3652 solar charge controller manages power storage for a remote bare metal control and data acquisition application.

[Forum](http://rpubus.org/bb/viewforum.php?f=6)

[HackaDay](https://hackaday.io/project/12784-rpuno)

[OSHpark](https://oshpark.com/shared_projects/84emcdT8)

[I2Cdebug]: ./i2c-debug
[RPUftdi]: https://github.com/epccs/RPUftdi
[RPUadpt]: https://github.com/epccs/RPUadpt

## Status

![Status](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/status_icon.png "Status")

## [Hardware](./Hardware)

Hardware files are in Eagle, there is also some testing, evaluation, and schooling notes for referance.

## Example with RPU BUS (RS-422)

A serial bus that allows multiple microcontroller boards to be connected to a host serial port. An [RPUftdi] shield with an on board USB device (a UART bridge) is placed near the host computer (USB cables can only reach about 2 meters). The remote MCU board(s) use an [RPUadpt] shield and are connected as a daisy-chain up to perhaps 1000 meters with CAT5 cable. 

![MultiDrop](https://raw.githubusercontent.com/epccs/RPUno/master/Hardware/Documents/MultiDrop.png "RPUno MultiDrop")

Why not use plain RS485? I want the host computer to use common serial programs (e.g. avrdude, PySerial, picocom...), and the microcontroller to work with common UART core(s) (e.g. Arduino and ilk). Modbus could be used on both the host and the bare metal side but that does not do firmware updates. RS-422 (with the transceivers automatically activated) work with the common UART core(s) and host programs for RS-232 so bootloaders can work as expected and so can the other serial programs (e.g. picocom and PySerial). 

I prefer a Command Line Interface (CLI), so that is what the examples use. The CLI is programmed to respond to commands terminated with a newline, so remember to press enter (which sends a newline) before starting a command. The command includes an address with a leading and trailing forward slash "/". The command echo starts after the address (second byte) is sent. The first byte will cause any transmitting device to stop and dump its outgoing buffer which should prevent collisions since the echo is delayed until after the second byte. 

As a short example, I'll connect with SSH (e.g. from a Pi) to the machine (an old x86 with Ubuntu) that has a USB connection to the [RPUftdi] board. These machines have matching usernames and keys placed so I don't need to use passwords. Then I will use picocom to interact with two different RPUno boards. They are on the serial bus at addresses '1' and '0' (note that ASCII '1' is 0x31, and ASCII '0' is 0x30, so they have an address that looks good on picocom but is probably not what was expected).  

```
rsutherland@raspberrypi:~ $ ssh conversion.local
Welcome to Ubuntu 16.04.1 LTS (GNU/Linux 4.4.0-53-generic i686)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/advantage

0 packages can be updated.
0 updates are security updates.

Last login: Fri Dec 16 12:59:33 2016 from 192.168.1.172
rsutherland@conversion:~$ picocom -b 115200 /dev/ttyUSB0
picocom v1.7

port is        : /dev/ttyUSB0
flowcontrol    : none
baudrate is    : 115200
parity is      : none
databits are   : 8
escape is      : C-a
local echo is  : no
noinit is      : no
noreset is     : no
nolock is      : no
send_cmd is    : sz -vv
receive_cmd is : rz -vv
imap is        :
omap is        :
emap is        : crcrlf,delbs,

Terminal ready
/1/id?
{"id":{"name":"Solenoid","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
/0/id?
{"id":{"name":"I2Cdebug","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
Ctrl-a,Ctrl-x 
Thanks for using picocom
```

At present, I'm using [I2Cdebug] to set the bus manager on the [RPUftdi] shield, it needs to know which address to reset so that it can lockout the others during bootload. Solenoid is the star of the show (so far), it is an attempt to control latching irrigation valves with cycles (inspired by Vinduino) and start the cycle at a daylight based offset, flow sensing for each zone is also working.

## AVR toolchain

The core files for this board are in the /lib folder. Each example has its files and a Makefile in its own folder. The toolchain packages that I use are available on Ubuntu and Raspbian. 

* sudo apt-get install [gcc-avr]
* sudo apt-get install [binutils-avr]
* sudo apt-get install [gdb-avr]
* sudo apt-get install [avr-libc]
* sudo apt-get install [avrdude]
    
[gcc-avr]: http://packages.ubuntu.com/search?keywords=gcc-avr
[binutils-avr]: http://packages.ubuntu.com/search?keywords=binutils-avr
[gdb-avr]: http://packages.ubuntu.com/search?keywords=gdb-avr
[avr-libc]: http://packages.ubuntu.com/search?keywords=avr-libc
[avrdude]: http://packages.ubuntu.com/search?keywords=avrdude

I am not a software developer (more of a hardware type), I started with the Arduino IDE and an Uno board and man was it easy to do cool stuff, unfortunately, when I tried to do my own board it was more pain than gain. I noticed Elliot Williams articles on "Makefile Madness" on Hack-a-day, and that was sort of a turning point. 

[Makefile Madness](http://hackaday.com/2016/03/11/embed-with-elliot-march-makefile-madness/)


