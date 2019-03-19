# AVR  Interrupt-Driven SPI slave

## Overview

SPI slave is an interactive command line program. CS0 is used to operate a status LED. nSS must be connected to Raspberry PI nCE00 pin (fixed on RPUpi^5).


## Pi SPI

After loading this RPUno firmware and enabling the AVR's SPI we can test the interface with the Raspberry Pi [SPI] hardware. Raspian needs its [SPI] master driver enabled (e.g. [raspi-config]).

[SPI]: https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md
[raspi-config]: https://www.raspberrypi.org/documentation/configuration/raspi-config.md

Raspibin has an spi group setup in /etc/udev/rules.d/99-com.rules. I just need to add my user name to the group for the system to allow me to use the device.

``` 
sudo usermod -a -G spi rsutherland
# logout for the change to take
``` 

Compile spidev_test.c on the Pi with:

``` 
wget https://raw.githubusercontent.com/raspberrypi/linux/rpi-3.10.y/Documentation/spi/spidev_test.c
gcc -o spidev_test spidev_test.c
# run with
./spidev_test -s 100000 -D /dev/spidev0.0
./spidev_test -s 500000 -D /dev/spidev0.0
./spidev_test -s 1000000 -D /dev/spidev0.0
./spidev_test -s 2000000 -D /dev/spidev0.0
./spidev_test -s 500000 -D /dev/spidev0.0
./spidev_test -s 500000 -D /dev/spidev0.0

spi mode: 0
bits per word: 8
max speed: 500000 Hz (500 KHz)

0D FF FF FF FF FF
FF 40 00 00 00 00
95 FF FF FF FF FF
FF FF FF FF FF FF
FF FF FF FF FF FF
FF DE AD BE EF BA
AD F0
``` 

Note: The output is offset a byte since it was sent back from the AVR. 


## Wiring Setup

![Wiring](./Setup/SpiSlvWiring.png)


## Firmware Upload

If the host computer is a Raspberry Pi the rpubus manager needs set so that it can bootload. Connect with picocom (or ilk).

``` 
picocom -b 38400 /dev/ttyAMA0
/1/iaddr 41
{"address":"0x29"}
#set a rpubus bootload address
/1/ibuff 3,49
{"txBuffer[2]":[{"data":"0x3"},{"data":"0x31"}]}
/1/iread? 2
{"rxBuffer":[{"data":"0x3"},{"data":"0x31"}]}
#clear the bus manager status bits so the host can bootload
/1/ibuff 7,0
{"txBuffer[2]":[{"data":"0x7"},{"data":"0x0"}]}
/1/iread? 2
#exit is C-a, C-x
``` 

With a serial port connection and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
sudo apt-get install make git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUno/
cd /RPUno/SpiSlv
make bootload
...
avrdude done.  Thank you.
``` 

# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../lib/rpu_mgr.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated) as an adddress. The ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.


## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/1/id?
{"id":{"name":"SpiSlv","desc":"RPUno (14140^7) Board /w atmega328p","avr-gcc":"4.8"}}
``` 


## /0/spi UP|DOWN

Turn on SPI. This enables SPI slave mode which exchanges data in shift registers. An UP will pull down DIO3 and enable the SPI hardware, DIO3 can be connected to nSS (a.k.a. DIO10) so that it activates the SPI slave hardware (nSS can also be connected to 0V). 

``` 
/1/spi UP
{"SPI":"UP"}
``` 
