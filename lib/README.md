# Common Files

## Overview

These files are used by Applications that have a Makefile in their folder, this folder does not have a Makefile. These files make up the library the can be used to control the microcontroller peripherals. It is at best incomplete. 

While I was trying to gather the bits and pieces I did not waste much time trying to track who did what, I just made sure it was an open license. Much of what I found was then overhauled and fashioned to suit my taste, so it is not clear how much credit should have to be held over anyway. 

I do think credit needs to be given for [Wiring]<https://arduinohistory.github.io/> which is where the ideas seem to originate for commands like digitalRead(), digitalWrite(). 

# Notes

It seems that we are moving into a time where the toolchain for a physical computing device needs to live on a single board computer (SBC's) running Linux that is attached or can be attached to one or more physical computing devices. This guarantees that the physical computing firmware can be compiled rather or not a network connection can be established. It does not diminish the need to have a network connection to update the toolchain and other services. The Linux SBC is not a replacement for a physical computing device, partly because a bare metal MCU can boot and be in control of equipment almost instantly, and writing device drivers for things like input capture hardware is not a good fit for the Linux kernel.