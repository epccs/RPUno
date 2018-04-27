# Makefile based non-blocking Blink

## Overview

Demonstration of General I/O, e.g. Blink an LED (LED_BUILTIN defined in pins_board.h). 

Referance ATmega328 datasheet 14.0 I/O-Ports

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

Elliot Williams of <http://hackaday.com/2016/03/15/embed-with-elliot-microcontroller-makefiles> got me looking at Makefiles now I can compile over an SSH connection to my Linux machines, (I use both Raspibn and Ubuntu). 

DigitalPin.h was hacked from William Greiman into plain C, but it still looks a little like Wiring.

