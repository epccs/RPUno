# Makefile based non-blocking Blink

## Overview

Demonstration of General I/O, e.g. Blink an LED. 

Referance ATmega328 datasheet 14.0 I/O-Ports

Elliot Williams of <http://hackaday.com/2016/03/15/embed-with-elliot-microcontroller-makefiles> got me looking at Makefiles now I can compile over an SSH connection to my Linux machines, (I use both Raspibn and Ubuntu). 

DigitalPin.h was hacked from William Greiman so it is plain C, but still looks a little like Wiring. I do not like how C++ uses the heap memory on bare metal, it is fine when an OS can catch an out of memory condition but on bare metal, it is just better if the compiler uses static memory allocation and the stack alone, I also don't use malloc to ensure the heap remains unused. I understand that few will understand what I'm saying, but consider it a test question, or even a job interview question. 

