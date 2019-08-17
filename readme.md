# RPUno 

From <https://github.com/epccs/RPUno/>

## Overview

A general purpose ATmega328p controller board with level shift IO and current sources that operate over the wide input voltage range.

The bare metal microcontroller on this board is the same as an Arduino Uno, I use it to evaluate the examples. Normally C is my preference so converting Arduino's C++ into C adds some overhead, the toolchain supports both but find fewer surprises with C. The toolchain is available on most Linux distributions, a Raspberry Pi makes a nice remote computer and can handle networking services robustly. I use SSH to log in to the remote Raspberry Pi and then I can pull updates from GitHub and run the "Makefile" rules that compile and upload the firmware. The mezzanine board [RPUadpt] or [RPUpi] can be used to add a multidrop serial (rpubus). This controller can turn off its shield VIN pin to power down the Raspberry Pi on RPUpi.

[RPUadpt]: https://github.com/epccs/RPUadpt
[RPUpi]: https://github.com/epccs/RPUpi

[Forum](https://rpubus.org/bb/viewforum.php?f=6)

[HackaDay](https://hackaday.io/project/12784-rpuno)

## Status

Note: bootloader speed has changed from 115.2k to 38.4k bps due to upload errors with the new transceiver on RPUpi^4. Command line serial speed was changed to 38.4k bps some time ago. I have no clue how the bootload upload was working.

[![Build Status](https://travis-ci.org/epccs/RPUno.svg?branch=master)](https://travis-ci.org/epccs/RPUno)

![Status](./Hardware/status_icon.png "Status")

[Options](./Hardware#bill-of-materials)

## [Hardware](./Hardware)

Hardware files and notes for referance.

## Example with RPU BUS

This example shows a multidrop serial bus that has several microcontroller boards connected to a single Raspberry Pi computer. Linux on the single board computer controls a hardware UART (/dev/ttyAMA0) that has serial lines connected to a transceiver and its differential pairs. The remote boards have a [RPUadpt] mezzanine board and CAT5 cable daisy-chain between them. 

[Irrigate7]: https://github.com/epccs/Irrigate7

![MultiDrop](./Hardware/Documents/MultiDrop.png "RPUno MultiDrop")

The transceivers are automatically activated, so common serial programs (e.g. avrdude, PySerial, picocom, and UART libraries) work without modification, but care must be taken to ensure only one controller answeres a host command. Each mezzanine board has a bus manager that is used to disconnect all the serial devices when the host connects, only the bootload target controller is connected. The host tells the manager what target to bootload over I2C (either using the local target controller or directly with 328pb second I2C interface). When the target runs its application and reads the bus address over I2C from the manager the manager broadcast a normal mode state that causes all the managers with valid controllers to connect to the serial lines.

I rely on a Command Line Interface (CLI) to the controllers. The CLI responds to commands terminated with a newline (inspired by console), press enter (which sends a newline) to start a command. The command includes an address with the first two bytes, but echo starts after the second byte is sent. The first byte will cause any transmitting device to stop and dump its outgoing buffer which prevents collisions in the data from the controllers to the host. The command length is also limited to allow the use of optimized buffer size.

As a short example, I'll connect with SSH (e.g. from a Windows 1809 build) to a Raspberry Pi Zero board. These machines have matching usernames, with configured SSH keys and known host file from a previous session. Once on the armv61 machine, I use picocom to interact with two different control boards. They are on the same serial bus at addresses '/1' and '/0' (note that ASCII '1' is 0x31, and ASCII '0' is 0x30, so they have an address that looks good on picocom but is probably not what was expected).

```
C:\Users\rsutherland>ssh pi1.local
Linux pi1 4.14.79+ #1159 Sun Nov 4 17:28:08 GMT 2018 armv6l

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Sat Feb 16 12:24:59 2019 from 192.168.4.6
rsutherland@pi1:~ $ picocom -b 38400 /dev/ttyAMA0
picocom v1.7

port is        : /dev/ttyAMA0
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
/0/id?
{"id":{"name":"NightLight","desc":"RPUno (14140^9) Board /w atmega328p","avr-gcc":"5.4.0"}}
/1/id?
{"id":{"name":"AmpHr","desc":"RPUlux (17323^2) Board /w atmega328p","avr-gcc":"5.4.0"}}
Ctrl-a,Ctrl-x 
Thanks for using picocom
rsutherland@pi1:~ $ exit
logout
Connection to pi1.local closed.

C:\Users\rsutherland>
```

At present, I'm using [I2Cdebug] to set the manager bootload address. 

[I2Cdebug]: ./i2c-debug

## AVR toolchain

The core files for this board are in the /lib folder. Each example has its files and a Makefile in its own folder. The toolchain packages that I use are available on Ubuntu and Raspbian. 

```
sudo apt-get install make git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno
```

packages on ubuntu are from Debain

* [gcc-avr](https://packages.ubuntu.com/search?keywords=gcc-avr)
* [binutils-avr](https://packages.ubuntu.com/search?keywords=binutils-avr)
* [gdb-avr](https://packages.ubuntu.com/search?keywords=gdb-avr)
* [avr-libc](https://packages.ubuntu.com/search?keywords=avr-libc)
* [avrdude](https://packages.ubuntu.com/search?keywords=avrdude)

The repository of binutils-avr for Debian is at

https://salsa.debian.org/debian/binutils-avr

The upstream where it was started from is 

https://www.gnu.org/software/binutils/

I don't see any AVR stuff in the upstream. Atmel must have forked it, and Debian is just patching critical material on the fork. Sadly I get the idea that these build tools are the last thing Microchip wants to mess with, I do not blame them it is a cluster, but it could be improved if they would start a public Github or Gitlab repository for these tools so I could have a place to contribute. Ideally, these updates would be added to the upstream, but I suspect that is not practical. Arduino uses these tools also, but they mainly want responsibility for there IDE. 

I place a [Bootloader] on the bare metal microcontroller with a fuse step and a step that uploads using an ISP tool. 

[Bootloader]: https://github.com/epccs/RPUno/tree/master/Bootloader

```
cd RPUno/Bootloader
# note /dev/ttyACM0 is my ICSP tool.
make fuse
make isp
```

The other applications are loaded through the bootloader using the host serial port. Note that the fuse cannot be changed with the bootloader thus reducing user issues with an application upload.

```
cd ~/RPUno/Adc
# note /dev/ttyUSB0 is my FTDI USBuart, and /dev/ttyAMA0 is my Raspberry Pi
make bootload
```

The software is a guide, it is in C because that is my preference for microcontrollers. If you want additional software please add a Github issue to this repository where we can discuss it. 


## Continuous Integration

Continuous Integration (CI) is the practice of automatically compiling and testing each time the mainline source is updated (e.g. git push). Travis CI is using a version of Ubuntu as there host environment for doing the test build. The build machine allows pulling in any packages I want including the AVR cross compiler. I don't do anything fancy, just run make. A rule like "make test" could be used if the test build machine had hardware connected (e.g. "make bootload" and then "make test") to the machine, but that is not practical in the foreseeable future. This was fairly simple to set up for Travis because the ATmega328p was in production at the time the Ubuntu toolchain was done.

[https://travis-ci.org/epccs/RPUno](https://travis-ci.org/epccs/RPUno)

Update: Travis has Ubuntu [Xenial] 16.04.

[Xenial]: https://docs.travis-ci.com/user/reference/xenial/


## Arduino IDE with Arduino 328p Core

The Arduino [IDE] can use the [Uno's AVR core] files that are included (my [core] files are C rather than C++), just remember to look at the schematic to see how the "Uno" is connected.  I do not use the Arduino IDE or C++ (I am a hardware designer,.and have limited intrest in software).

[IDE]: https://www.arduino.cc/
[Uno's AVR core]: https://github.com/arduino/ArduinoCore-avr
[core]: https://github.com/epccs/RPUno/tree/master/lib


## Visual Studio Code

VSC is one of the editors I use, and it is happy with Makefiles. The feature I like most is [IntelliSense], which is configured with JSON files in [.vscode]. 

[IntelliSense]: https://code.visualstudio.com/docs/editor/intellisense
[.vscode]: https://github.com/epccs/RPUno/tree/master/.vscode

The best way to use VSC with GCC (avr-gcc) is in an environment that just works, for me that is Linux, so I can enable [WSL] and install Ubuntu and then use the [Remote WSL].

[WSL]: https://docs.microsoft.com/en-us/windows/wsl/install-win10
[Remote WSL]: https://code.visualstudio.com/docs/remote/wsl

or I can install Linux on a 64bit x86 machine and use [Remote SSH].

[Remote SSH]: https://code.visualstudio.com/docs/remote/ssh
