# Reflow

## Overview

Controls a 20A Solid State Relay (SSR) and a buzzer. 

Every two seconds a byte is taken from EEPROM to control the PWM rate of the SSR for the next two seconds. The thermocouple is not used for direct feedback control since I would not feel safe walking away from such a setup. I use the thermocouple to get the profile correct, but once set the profile is hard wired into the control loop. If I made an error in the software it will be repeated each time, it will not be hidden by a chance mistake in PID logic. For this to work the starting condition needs to be the same (near room temperature) for each run, I find it takes about ten minutes.

Reads a Fluke 80TK Thermocouple Module (1mV/deg F) on channel zero. 

![Profile](https://raw.githubusercontent.com/epccs/RPUno/master/Reflow/profile/walmartBD,160622.png "Profile for Black & Decker Model NO. TO1303SB")

Do not modify the oven, if you do and it burns down your house the insurance may be able to deny payment. Turn the knobs so it is always on (as shown in the image) and set the temperature high enough that it will not turn off the oven. Now the Solid State Relay (SSR) can modulate power to the heating elements, the modulation does not need to be fast since they have a five-second thermal response, in fact, a two second PWM is fine. The SSR needs to be placed in a certified electrical enclosure to make sure insurance will pay (I'm not an expert, and there are always more rules). 

![Setup](https://raw.githubusercontent.com/epccs/RPUno/master/Reflow/profile/WalmartBD,TO1303SB.jpg "Setup of Black & Decker Model NO. TO1303SB")

A 255 value in EEPROM will turn on the buzzer on pin 6 for two seconds, while two consecutive values will terminate the profile. The program will otherwise run to the end of EEPROM memory.

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the Atmega328 board run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@straightneck:~/Samba/RPUno/Reflow$ make bootload

``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 9600 /dev/ttyUSB0
``` 

log a terminal session to check the profile

``` 
script -f -c "picocom -b 9600 /dev/ttyUSB0" stuff.log
``` 


# Commands

Commands are interactive over the serial interface at 9600 baud rate. The echo will start after second charactor of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Reflow"}}
```

## /0/reflow?

Start the reflow profile and read the Fluke 80TK Thermocouple Module set to Fahrenheit scale on analog channel zero.

``` 
/0/reflow?
{"millis":"10","pwm":"255","deg_c":"26.11"}
{"millis":"2010","pwm":"50","deg_c":"26.11"}
```

##  /0/ee? 0..1023

Return the EEPROM value at address. This checks if eeprom_is_ready() befor trying to read the EEPROM, if it is not ready the program loops back through the round robin where a received charactor may terminate the command. 

``` 
/0/ee? 0
{"EE[0]":"255"}
```

##  /0/ee 0..1023,0..255

Write the value given as argument one to the address given as Argument zero. This checks if eeprom_is_ready() befor trying to write to the EEPROM, if it is not ready the program loops back through the round robin where a received charactor may terminate the command. If the command is terminated the write may not occure. The JSON response is a read of the EEPROM. 

Warning writing EEPROM can lead to device failure, it is only rated for 100k write cycles.

``` 
/0/ee 0,255
{"EE[0]":"255"}
```


# Profile

The profile sub folder has Python3 program(s) that load the profile using ee and ee? commands. 


# Notes

I Used a Fluke 80TK Thermocouple Module set to Fahrenheit scale so the output voltage runs up to almost .5V, and set the ADC to use the internal 1.1V bandgap referance and that means nearly half that ADC scale is used. 

