# Solenoid Control 

## Overview

Solenoid is an interactive command line program that demonstrates control of the K3 latching solenoid driver board using the I/O pins of an ATmega328p. 

## Wiring K3 to RPUno

``` 
RPUno   (digital)   K3 
------------------------
PD3     (IO3)       E3
0V       na         nE2
0V       na         nE1
PB2     (nSS/IO10)  A0
PB3     (MOSI/IO11) A1
PB4     (MISO/IO12) A2 
PB5     (SCK/IO13)  LED_BUILTIN
``` 

The RPUno has those I/O's wired to a pluggable onboard connector. They are level converted to 5V and will ouput 4V without a pullup (which is enough for a minimum high on 74HC logic). 

The IO13 pin is used as LED_BUILTIN and blinks on and off for a second when an rpu_address is read over I2C (if I2C failed it blinks four times as fast). 

[![RPUno^5 With K3^0](http://rpubus.org/bb/download/file.php?id=25)](http://rpubus.org/Video/14140%5E5WithK3%5E0.mp4 "RPUno^5 With K3^0")


# Memory map 

EEPROM has the values that are loaded after initialization (if the id value is correct). 

```
function            type       addr:K1  K2  K3
id                  UINT16          40  60  80
delay_start_in_sec  UINT32          42  62  82
runtiem_in_sec      UINT32          46  66  86
delay_in_sec        UINT32          50  70  90
flow_stop           UINT32          54  74  94
cycles              UINT8           58  78  98
```


# Programing

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@conversion:~/Samba/RPUno/Solenoid$ make bootload
...
avr-size -C --mcu=atmega328p Solenoid.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:   17114 bytes (52.2% Full)
(.text + .data + .bootloader)

Data:        530 bytes (25.9% Full)
(.data + .bss + .noinit)
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.


``` 
#exit is C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 115200 /dev/ttyUSB0" stuff.log
``` 


# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after the second character of a new line. 


## /[rpu_address]/[command [arg]]

rpu_address is taken from the I2C address 0x29 (e.g. ../Uart/id.h get_Rpu_address() ). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

The LED_BUILTIN is bliked fast (twice per second) if the I2C address is not found, also the rpu_address defaults to '0'. 

Commands and their arguments follow.


## /0/id? [name|desc|avr-gcc]

Identify is from ../Uart/id.h Id().

``` 
/1/id?
{"id":{"name":"Solenoid","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```


##  /0/run k[,cycles] 

Start the solenoid k (1|2|3) operation, with option to override cycles (1..255). 

If EEPROM does not have settings the solenoids retains initialized values (delay_start = 1..7, runtime= 1, delay = 3600, flow_stop = not used) that will cause each solenoid to operate for a second after a delay_start time that spaces there operation out by 3 seconds each (it insures all are in a known state).

After a solenoid has entered the delay state and let go of the flow meter resource another solenoid that is ready to use the flow meter will do so. Make sure to set the delay time long enough that all the other solenoids can use their runtime, or the flow meter becomes a resource constraint and some zones will get shorted. For example set all the delay times to 360 and make sure the combined runtimes do not add up to 360 (i.e. 100, 80, 120).

```
/1/run 1,1
{"K1":{"delay_start_sec":"1","runtime_Sec":"1","delay_Sec":"3600","cycles":"1"}}
/1/run 2,1
{"K2":{"delay_start_sec":"4","runtime_Sec":"1","delay_Sec":"3600","cycles":"1"}}
/1/run 3,1
{"K3":{"delay_start_sec":"7","runtime_Sec":"1","delay_Sec":"3600","cycles":"1"}}
```


##  /0/save k,cycles 

Save the solenoid k (1|2|3) with cycles (1..255) to EEPROM, it can then autostart.

Saved settings are loaded after the solenoids have initialized.

```
/1/save 1,10
{"K1":{"delay_start_sec":"10","runtime_Sec":"15","delay_Sec":"60","cycles":"10"}}
/1/save 2,10
{"K2":{"delay_start_sec":"30","runtime_Sec":"15","delay_Sec":"60","cycles":"10"}}
/1/save 3,10
{"K3":{"delay_start_sec":"50","runtime_Sec":"15","delay_Sec":"60","cycles":"10"}}
```

##  /0/load k

Load the solenoid k (1|2|3) from EEPROM. Use run to start it.

```
/1/load 1
{"K1":{"delay_start_sec":"10","runtime_Sec":"15","delay_Sec":"60","cycles":"10"}}
/1/load 2
{"K2":{"delay_start_sec":"30","runtime_Sec":"15","delay_Sec":"60","cycles":"10"}}
/1/load 3
{"K3":{"delay_start_sec":"50","runtime_Sec":"15","delay_Sec":"60","cycles":"10"}}


##  /0/stop k 

Reduce the delay_start, runtime, and delay to one second each to stop the solenoid k (1|2|3) operation.

To change the solenoids setting use /stop, then /load, and change the desired setting (e.g. /runtime) and finally save it and perhaps /run it.

```
/1/stop 1
{"K1":{"stop_time_sec":"3"}}
```


##  /0/delay k,delay_in_sec

Set the solenoid k (1|2|3) delay between runs (1..86400, e.g. 24 hr max). 

``` 
/1/delay 3,60
{"K3":{"delay_sec":"40"}}
/1/run 3,1
{"K3":{"delay_start_sec":"7","runtime_sec":"1","delay_sec":"60","cycles":"1"}}
```


##  /0/runtime k,runtime_in_sec

Set the solenoid k (1|2|3) run time (1..21600, e.g. 6hr max). 

``` 
/1/runtime 3,20
{"K3":{"runtime_sec":"20"}}
/1/run 3,1
{"K3":{"delay_start_sec":"7","runtime_sec":"20","delay_sec":"40","cycles":"1"}}
```


##  /0/pre k,delay_start_in_sec

Set the solenoid k (1|2|3) one time delay befor cycles run (1..21600, e.g. 6hr max). 

``` 
/1/pre 3,10
{"K3":{"delay_start_sec":"10"}}
/1/run 3,1
{"K3":{"delay_start_sec":"10","runtime_sec":"20","delay_sec":"40","cycles":"1"}}
``` 


##  /0/fstop k,flow_stop

Set the solenoid k (1|2|3) flow_stop (1..0xFFFFFFFF) that also stops the solenoid (e.g. when flow count is reached).

``` 
/1/fstop 3,500
{"K3":{"flow_stop":"500"}}
/1/run 3,1
{"K3":{"delay_start_sec":"10","runtime_sec":"20","delay_sec":"40","cycles":"1","flow_stop":"500"}}
``` 

##  /0/flow? k

Report the solenoid k (1|2|3) flow_cnt or pulses events on ICP1.

``` 
/1/flow? 3
{"K3":{"cycle_state":"11","cycles":"7","flow_cnt":"0"}}
``` 


##  /0/time? k

Report the solenoid k (1|2|3) runtime in millis.

``` 
/1/time? 3
{"K3":{"cycle_state":"11","cycles":"9","cycle_millis":"15000"}}
``` 
