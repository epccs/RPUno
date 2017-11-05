# RPUno 

From <https://github.com/epccs/RPUno/>

## Overview

This no-frills Controller board has an ATmega328p that is programmable with the open source GCC toolchain for AVR. The headers are for the shields [RPUftdi], [RPUadpt], and [RPUpi] that connect the controller UART to an RS422 bus.

[RPUftdi]: https://github.com/epccs/RPUftdi
[RPUadpt]: https://github.com/epccs/RPUadpt
[RPUpi]: https://github.com/epccs/RPUpi

[Forum](http://rpubus.org/bb/viewforum.php?f=6)

[HackaDay](https://hackaday.io/project/12784-rpuno)

## Status

Available through [Tindie](https://www.tindie.com/products/ron-sutherland/rpuno-a-solar-powered-atmega328p-board/)

![Status](./Hardware/status_icon.png "Status")

## [Hardware](./Hardware)

Hardware files and notes for referance.

## Example with RPU BUS (RS-422)

This example shows an RS-422 serial bus that allows multiple microcontroller boards to be connected to a single computer serial port. It has an [RPUftdi] shield with a USB device that acts as a UART bridge that is placed near the computer (USB cables should be less than 2 meters). I normally use an Uno Clone under the RPUftdi. The remote MCU boards are RPUno (or [Irrigate7]) and have an [RPUadpt] shield that daisy-chains the RS-422 over CAT5 cable with RJ45 connectors. 

[Irrigate7]: https://github.com/epccs/Irrigate7

![MultiDrop](./Hardware/Documents/MultiDrop.png "RPUno MultiDrop")

The transceivers are automatically activated, so common serial programs (e.g. avrdude, PySerial, picocom...) can be used. Also, the microcontroller's common UART libraries (e.g. like in the Arduino Uno core) work, but care must be taken to ensure only one microcontroller answeres.

I prefer a Command Line Interface (CLI), so that is what the examples use. The CLI example programs respond to commands terminated with a newline, so remember to press enter (which sends a newline) before starting a command. The command includes an address with a leading and trailing forward slash "/". The command echo starts after the address (second byte) is sent. The first byte ('/') will cause any transmitting device to stop and dump its outgoing buffer which should prevent collisions since the echo is delayed until after the second byte. 

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
rsutherland@conversion:~$ picocom -b 38400 /dev/ttyUSB0
picocom v1.7

port is        : /dev/ttyUSB0
flowcontrol    : none
baudrate is    : 38400
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

At present, I'm using [I2Cdebug] to set the bus manager on the [RPUftdi] shield, it needs to know which address to reset so that it can lockout the others during bootload. Solenoid is the star of the show (so far), it is an attempt to control latching irrigation valves with cycles that start after a daylight based offset, flow sensing for each zone is also working.

[I2Cdebug]: ./i2c-debug

## AVR toolchain

The core files for this board are in the /lib folder. Each example has its files and a Makefile in its own folder. The toolchain packages that I use are available on Ubuntu and Raspbian. 

```
sudo apt-get install git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno
cd RPUno/Adc
make
# serial bootload a firmware onto an ATmega328p with optiboot 
# detected UART: FTDI (/dev/ttyUSB0), Pi Zero (/dev/ttyAMA0)
# not detected UART: Uno (/dev/ttyACM0 it is my ICSP tool for now).
make bootload
```

* [gcc-avr](http://packages.ubuntu.com/search?keywords=gcc-avr)
* [binutils-avr](http://packages.ubuntu.com/search?keywords=binutils-avr)
* [gdb-avr](http://packages.ubuntu.com/search?keywords=gdb-avr)
* [avr-libc](http://packages.ubuntu.com/search?keywords=avr-libc)
* [avrdude](http://packages.ubuntu.com/search?keywords=avrdude)

I am not a software developer (more of a hardware type), I started with the Arduino IDE and an Uno board and found some things I like and some I did not. When I did my own boards I found the Arduino IDE to have more pain than gain. I also was having problems with C++ that was difficult to nail down. When I noticed an article from Elliot Williams on "Makefile Madness" at Hack-a-day. 

[Makefile Madness](http://hackaday.com/2016/03/11/embed-with-elliot-march-makefile-madness/)

That was sort of my turning point, I started to use Makefiles and plane C. C++ is C plus stuff to implement object-oriented programing. Unfortunately, some of the OOP stuff uses the heap memory system, and that seems to have been my problem. To keep the heap clean I ended up porting the C++ things I wanted to C. I think OOP is fine on a Raspberry Pi, but without an operating system that at the very least handles an out of memory fault when the heap memory system and stack memory system collide I don't want my programing language to use the heap. C does not use the heap unless explicitly told to, while C++ uses (and fragments) it in ways I am not interested in understanding.
