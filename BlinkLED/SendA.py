#!/usr/bin/env python3
# The above line tells the command shell to run env which will then run Python3.
# Unfortunately when I edit this file on Windows it saves with a CR-LF line-ending
# which will confuse the command processor. Line ending is the gift from hell.
# Anyway make sure that top line ends with a LF only. Python will work with a CR-LF or LF line ending 
# but that first line may confuse the OS runing on Linux and fail. 
# This python file also needs to be set as an executable with chmod +x <this_file>

# Pyserial has got worked on since last I used it, and I want to try the new stuff
# $ sudo apt install python3-pip
# $ pip3 install pyserial
#    Collecting pyserial
#    Downloading pyserial-3.3-py2.py3-none-any.whl (189kB)

import serial, struct, time
ser = serial.Serial('/dev/ttyUSB0',38400, timeout=3)
time.sleep(3) # wait for the bootloader to run
if ser.in_waiting:
    junk = ser.readline().strip() # clean any junk from the buffer
    print("junk: " + junk[-10:].decode("utf-8")) 
a = ord('a') # send the code for 'a'
# Python 3.6 strings are utf-8, they seem to need encode to ascii for Pyserial
ser.write((chr(a)).encode('ascii'))
echo = ser.readline().strip()
print("debug: " + echo[-10:].decode("utf-8")) 
