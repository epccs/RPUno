# Analog-to-Digital Converter

## Overview

Adc is an interactive command line program that demonstrates control of an Analog-to-Digital Converter from pins PC0 through PC7 on an ATmega328p. 

Arduino Uno is marked as A0 though A5, which is sometimes confused as PA0. The ADMUX register is set to 0 when selecting ADC channel 0.  The analog channel has nothing to do with the digital pin number used by the [Wiring]. The pin used by ADC channel 0 has been assigned the number 14 for the digital Wiring functions (used with pinMod, digitalRead...), however for analogRead the ADC channel number is used.

[Wiring]: https://arduinohistory.github.io/


# EEPROM Memory map 

A map of calibration valuses in EEPROM. 

```
function            type        ee_addr:
id                  UINT16      30
ref_extern_avcc     UINT32      32
ref_intern_1v1      UINT32      36
```

The AVCC pin is used to power the analog to digital converter and is also selected as a reference. On RPUno the AVCC pin is powered by a switchmode supply that can be measured to use as a calibrated reference.

The internal 1V1 bandgap is not trimmed by the manufacturer, so it needs to be measured. However, once it is known it is a better reference than AVCC.

[SelfTest] loads calibration values for references in EEPROM.

[SelfTest]: https://github.com/epccs/RPUno/tree/master/SelfTest


# Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUno/
cd /RPUux/Adc
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.


``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff.log
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../lib/rpu_mgr.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated) as an adddress. The ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.


## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/1/id?
{"id":{"name":"Adc","desc":"RPUno (14140^9) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

##  /0/analog? 0..7\[,0..7\[,0..7\[,0..7\[,0..7\]\]\]\]    

Analog-to-Digital Converter reading from up to 5 ADMUX channels. The reading repeats every 60 Seconds until the Rx buffer gets a character. On RPUno channel 7 is the input voltage (PWR_V), channel 6 is the input current (PWR_I), channel 3 is a voltage divider on the channel 2 input, channel 2,  1, and channel 0 inputs can read up to 3.5V (higher voltages are blocked by a level shift).  Note channel 4 and 5 are used for I2C on RPUno.

``` 
/1/analog? 6,7
{"PWR_I":"0.000","PWR_V":"12.51"}
/1/analog? 0,1,2,3
{"ADC0":"1.38","ADC1":"2.83","ADC2":"0.72","ADC3":"0.61"}
/1/analog? 4,5
{"ADC4":"SDA","ADC5":"SCL"}
```

Values are from floating ADC channels. PWR_I is AVCC referance (SelfTest uses the 1V1 band gap).

The value reported is based on the avcc referance value which is saved in EEPROM, see bellow.


##  /0/avcc 4500000..5500000

Calibrate the AVCC reference in microvolts.

``` 
/1/avcc 5006500
# do this with the SelfTest
``` 


##  /0/onevone 900000..1300000

Set the 1V1 internal bandgap reference in microvolts.

```
/1/onevone 1100000
# do this with the SelfTest
``` 

The SelfTest will calculate this value when it is ran based on the AVCC value compiled into source.


##  /0/reftoee

Save the reference in static memory to EEPROM.

```
/1/reftoee
# do this with the SelfTest
```


##  /0/reffrmee

Load the reference from EEPROM into static memory.

```
/1/reffrmee
{"REF":{"extern_avcc":"5.0065","intern_1v1":"1.0733",}}
```

