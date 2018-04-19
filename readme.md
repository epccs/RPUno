# RPUno 

From <https://github.com/epccs/RPUno/>

## Overview

This general purpose programmable controller board has a bare metal ATmega328p microcontroller which has several programing options.  The headers are for the shields [RPUadpt], and [RPUpi] that connect the controller UART to a multidrop serial bus.

[RPUadpt]: https://github.com/epccs/RPUadpt
[RPUpi]: https://github.com/epccs/RPUpi

[Forum](http://rpubus.org/bb/viewforum.php?f=6)

[HackaDay](https://hackaday.io/project/12784-rpuno)

## Status

Available through [Tindie](https://www.tindie.com/products/8862/)

![Status](./Hardware/status_icon.png "Status")

## [Hardware](./Hardware)

Hardware files and notes for referance.

## Example with RPU BUS

This example shows a multidrop serial bus that allows several microcontroller boards to be connected to a single host computer. It has an RPUftdi shield with a USB device that acts as a UART bridge that is placed near the computer (USB cables should be less than 2 meters). I normally use an Uno Clone under the RPUftdi. The remote MCU boards are RPUno (or [Irrigate7]) and have an [RPUadpt] shield that daisy-chains the serial over CAT5 cable with RJ45 connectors. 

[Irrigate7]: https://github.com/epccs/Irrigate7

![MultiDrop](./Hardware/Documents/MultiDrop.png "RPUno MultiDrop")

The transceivers are automatically activated, so common serial programs (e.g. avrdude, PySerial, picocom...) can be used. Also, the microcontroller's UART libraries (e.g. like in the Arduino Uno core) work, but care must be taken to ensure only one microcontroller answeres.

Most of the examples use a Command Line Interface (CLI) on the controler's serial port (UART). The CLI responds to commands terminated with a newline, so remember to press enter (which sends a newline) before starting a command. The command includes an address with a leading and trailing forward slash "/". The command echo starts after the address (second byte) is sent. The first byte ('/') will cause any transmitting device to stop and dump its outgoing buffer which should prevent collisions since the echo is delayed until after the second byte. 

As a short example, I'll connect with SSH (e.g. from a Raspberry Pi) to an old x86 machine (Ubuntu) that has a USB connection to the [RPUftdi] board. These machines have matching usernames and SSH keys placed (e.g. passwords are not used). Once on the x86 machine, I use picocom to interact with two different RPUno boards. They are on the same serial bus at addresses '1' and '0' (note that ASCII '1' is 0x31, and ASCII '0' is 0x30, so they have an address that looks good on picocom but is probably not what was expected).  

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
{"id":{"name":"Solenoid","desc":"RPUno Board /w atmega328p","avr-gcc":"4.9"}}
/0/id?
{"id":{"name":"I2Cdebug","desc":"RPUno Board /w atmega328p","avr-gcc":"4.9"}}
Ctrl-a,Ctrl-x 
Thanks for using picocom
```

At present, I'm using [I2Cdebug] to set the bus manager on the RPUftdi shield, it needs to know which address to reset so that it can lockout the others during bootload. Solenoid is the star of the show (so far), it is an attempt to control latching irrigation valves with cycles that start after a daylight based offset, flow sensing for each zone is also working.

[I2Cdebug]: ./i2c-debug

## AVR toolchain

The core files for this board are in the /lib folder. Each example has its files and a Makefile in its own folder. The toolchain packages that I use are available on Ubuntu and Raspbian. 

```
sudo apt-get install git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno
```

* [gcc-avr](http://packages.ubuntu.com/search?keywords=gcc-avr)
* [binutils-avr](http://packages.ubuntu.com/search?keywords=binutils-avr)
* [gdb-avr](http://packages.ubuntu.com/search?keywords=gdb-avr)
* [avr-libc](http://packages.ubuntu.com/search?keywords=avr-libc)
* [avrdude](http://packages.ubuntu.com/search?keywords=avrdude)

I like to place a [Bootloader] on the bare metal microcontroler with an ISP tool. 

[Bootloader]: https://github.com/epccs/RPUno/tree/master/Bootloader

```
cd RPUno/Bootloader
# note /dev/ttyACM0 it is my ICSP tool.
make fuse
make isp
```

The other applications are loaded through the bootloader using the host serial port. 

```
cd ~/RPUno/Adc
# note /dev/ttyUSB0 is my FTDI USBuart, and /dev/ttyAMA0 is my Raspberry Pi
make bootload
```

The software is a guide, it is in C because that is my preference when lacking an operating system.


## Arduino IDE

The Arduino IDE can use the [Uno's AVR core] files that are included (my [core] files are C rather than C++), just remember to look at the schematic to see how the "Uno" is connected. I do not use the Arduino core or C++.

[Uno's AVR core]: https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr/cores/arduino
[core]: https://github.com/epccs/RPUlux/tree/master/lib

