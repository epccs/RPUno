# AVR  Interrupt-Driven UART with stdio redirect

## Overview
Demonstration of stdio to interrupt-driven UART redirect. 

Referance ATmega328 datasheet 20.0 USART0 Universal Synchronous and Asynchronous serial Receiver and Transmitter.

Toolchain setup http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html

Makefile based, without the extras. Allowing the tools to work in a way that prevents the oddnesses caused by mashing "ino" files into a single cpp file (ino is a compaction of into... haha that is funny).

Minimalized Interrupt-driven UART code from <https://github.com/hwstar/avr-uart>, and added streams from <https://github.com/andygock/avr-uart>

## Commands

### id?

JSON who am I?, what am I?
