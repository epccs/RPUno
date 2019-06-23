# Transceiver Test

## Overview

Check Transceivers on an RPUpi, this runs once after a reset and then loops in a pass/fail section.

Test commands are issued to the manager's I2C port to operate it in test_mode. The manager needs to be loaded with its firmware (e.g., RPUpi's [Remote] firmware) 

[Remote]: https://github.com/epccs/RPUpi/tree/master/Remote

The wiring (from SelfTest) has a red and green LED that blink to indicate test status.

## Wiring Needed for Transceiver Test

![Wiring](./Setup/XcvrTestWiring.png)


## Power Supply

Connect a power supply with CV and CC mode. Set CC at 200mA and CV at 12.8V.


## Firmware Upload

With a serial port connected (e.g., [ICSP] or [USBuart]) to the RPUno's bootload port and optiboot installed run 'make' to build and then 'make bootload' to flash the MCU. Bootloading over the RPUbus is also an option but it would probably be a good idea to see a green light first. 

[ICSP]: https://github.com/epccs/Driver/tree/master/ICSP
[USBuart]: https://github.com/epccs/Driver/tree/master/USBuart

``` 
git clone https://github.com/epccs/RPUno/
cd /RPUno/XcvrTest
make
make bootload
...
avrdude done.  Thank you.
make clean
``` 

Now connect with picocom (exit is C-a, C-x). 

``` 
picocom -b 38400 /dev/ttyUSB0
picocom v2.2
...
Terminal ready
RPUpi Transceiver Test date: June 23 2019
avr-gcc --version: 5.4.0
I2C provided address 0x31 from manager
[PASS]
```

The test is not done, I will start working on it soon.

It needs to place the manager in test mode and measure quit a few things and then exit test mode and write the results to serial. 

I was a test engineer for many years, I use to think about testing from the outside into the inside, but this is from the inside to the outside. If the test fails the bootload port will be needed, I fear that during the analysis the host UART connected to the bootload port will be trashed with garbage, but I think clean data can be sent after the trash.

