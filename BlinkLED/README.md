# Makefile based non-blocking Blink

## Overview

Demonstration of General I/O, e.g. Blink an LED. 

Referance ATmega328 datasheet I/O-Ports

Also shows the UART core and how to redirect it to stdin and stdout, as well as some Python that sends an 'a' character to stop the LED from blinking. 

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUno/
cd /RPUno/BlinkLED
make bootload
...
avrdude done.  Thank you.
``` 

# Notes

pin_num.h was hacked from William Greiman work (https://github.com/greiman) into plain C, but it still looks a little like Wiring.

