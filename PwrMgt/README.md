# Power Management

## Overview

PwrMgt is an interactive command line program that demonstrates control of RPUno's ability to turn off the power to the SHLD_VIN pin (i.e. to the Shield), and turn off the current sources used for ICP1 and analog loops. Commands from [i2c-debug], [Adc], [Digital], [DayNight] and [AmpHr] are also available.

[i2c-debug]: ../i2c-debug
[Adc]: ../Adc
[Digital]: ../Digital
[DayNight]: ../DayNight
[AmpHr]: ../AmpHr

These commands are available on the RPUno UART. Some of the commands can be used to control the bus manager (over I2C) in order to shutdown a Pi Zero on an [RPUpi] shield. The bus manager is connected to a switch that when monitored by the Pi [Shutdown] script will halt the Pi. The bus manager uses a weak pull-up that the Pi can read to keep running. When a person presses the shutdown button or the manager pulls it down the Pi will [Shutdown]. This program demonstrates a number of states that may help prevent SD card file corruption (but keep in mind this is just my guess at how to do it, i.e. trust at your own risk).

[RPUpi]: https://github.com/epccs/RPUpi/
[Shutdown]: https://github.com/epccs/RPUpi/tree/master/Shutdown

After the bus manager is told to shutdown the Pi the battery current should drop and eventually stabilize (i.e. the Pi uses about 11mA from the battery when halted). One nasty thing happens at this time, the SD card will continue to do wear leveling, and this shows as slightly uneven current usage by the Pi (sd card) so I need to wait for a time (debounce) with the current usage very low and stable.  After the shutdown debounce is done the RPUno can power off its VIN pin which is controled with IO2 (reg-bit PD2).

Current sources can have power turned off (^5 only does ICP1 pulse sensor) using IO9 (reg-bit PB1).

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/PwrMgt$ make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. 

``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /[rpu_address]/[command [arg]]

rpu_address is taken from the I2C address 0x29 (e.g. ../Uart/id.h get_Rpu_address() ). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

The LED_BUILTIN is bliked fast (twice per second) if the I2C address is not found, also the rpu_address defaults to '0'. 

Commands and their arguments follow.


## /0/id? [name|desc|avr-gcc]

Identify is from ../Uart/id.h Id().

``` 
/1/id?
{"id":{"name":"PwrMgt","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

##  /0/vin DOWN|UP

Set power to the RPUno VIN shield pin. Each state should echo once.

``` 
/1/vin DOWN
{"VIN":"CCSHUTDOWN"}
{"VIN":"I2CHAULT"}
{"VIN":"ATHAULTCURR"}
{"VIN":"DELAY"}
{"VIN":"WEARLEVELINGCLEAR"}
{"VIN":"DOWN"}
```

Note: If a byte is sent on the serial it will stop the command at whatever point it reached and restart the charge controller. Keep in mind that once the charge controller stops it will not restart until the battery voltage is 1.25% bellow the float voltage setpoint.

CCSHUTDOWN shows that the solar charge control has to be turned off so that only the discharge current from battery needs looked at.

I2CHAULT shows that an I2C command was sent to the [RPUpi] shield bus manager to pull down the shutdown pin. The Pi needs to have a Python [Shutdown] script installed and running to halt. 

ATHAULTCURR shows that the ADC reading of discharge current on analog channel 3 is less than the expected value that indicates the Pi Zero has halted.

DELAY shows that HAULT_DELAY_MILSEC has passed.

WEARLEVELINGCLEAR shows that the ADC reading of discharge current on analog channel 3 has been stable for a period of time that indicates wear leveling may have cleared (grain of salt). Unfortunately, wear leveling is not in the SD spec so there is not a proper way to ensure it has really finished. 

DOWN shows that RPUno has pulled IO2 low and thus turned off the VIN pin power to the shield, and then restart the charge controller.

``` 
/1/vin UP
{"VIN":"UP"}
```

UP shows that RPUno has pulled IO2 high and thus turned on the VIN pin power to the shield.


##  /0/pulseloop DOWN|UP

Set power to the RPUno FT/PULSE plugable connectors pin. Each state should echo once.

``` 
/1/pulseloop DOWN
{"FT":"DOWN"}
/1/pulseloop UP
{"FT":"UP"}
```

DOWN shows that RPUno has pulled IO9 low and thus turned off the FT/PULSE loop current sources.

UP shows that RPUno has pulled IO9 high and thus turned on the FT/PULSE loop current sources.


##  /0/vin?

Return the shutdown_detected value from the bus manager over I2C, this will indicate if the shutdown button has been manualy pressed, and clear the flag. 

``` 
/1/vin?
{"SHUTDOWN":"DETECTED"}
/1/vin?
{"SHUTDOWN":"CLEAR"}
```


## [/0/analog? 0..7[,0..7[,0..7[,0..7[,0..7]]]]](../Adc#0analog-0707070707)


## [/0/iscan?](../i2c-debug#0iscan)


## [/0/iaddr 0..127](../i2c-debug#0iaddr-0127)


## [/0/ibuff 0..255[,0..255[,0..255[,0..255[,0..255]]]]](../i2c-debug#0ibuff-02550255025502550255)


## [/0/ibuff?](../i2c-debug#0ibuff)


## [/0/iwrite](../i2c-debug#0iwrite)


## [/0/iread? [1..32]](../i2c-debug#0iread-132)


## [/0/day?](../DayNight#0day)


## [/0/charge?](../AmpHr#0charge)