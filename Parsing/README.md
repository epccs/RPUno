# Serial parsing for textual commands and arguments

## Overview

Demonstration of command parsing with the redirected stdio functions (e.g. printf() and simular)  from avr-libc. 

Toolchain setup http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html

Makefile based. Without the extras to increase transparency. The tools work in the way they were designed. 

# Command Line

/addr/command [arg[,arg[,arg[,arg[,arg]]]]]

The command line is structured like MQTT topics with some optional arguments 


## addr

The address is a single character. That is to say, "/0/id?" is at address '0', where "/id?" is a command. Echo of the command occurs after the address is received so the serial data can be checked as well as being somewhat keyboard friendly device.

## command

The command is made of one or more characters that are: alpha (see isalpha() function), '/', or '?'. 

## arg

The argument(s) are made of one or more characters that are: alphanumeric (see isalnum() function). 
