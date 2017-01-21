# Charge Control test

## Overview

__Do not use this program__ it was for RPUno^2 and has not been updated.

I made a board that has high side current sensing for the PV input and battery charging and discharging. The high side sensor only measures one direction so two are needed for the battery. The battery current sensing is now part of RPUno ^4. A load is also provided, it has four digital control lines that enable current sinks. 

At startup, the program waits until the PV_IN is over 18V which means the solar has charged the battery to its float level. It then delays for 3hr to allow an absorption cycle to complete. After which a delay occurs until PV_IN has dropped to less than 5V (e.g. night). Next, the load settings are stepped through for a record. Then it discharges and reports as the Battery voltage crosses 50mV thresholds until the discharge voltage is reached. If the serial receives a character it will interrupt the test and enable charging.

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@straightneck:~/Samba/RPUno/CCtest$ make bootload
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

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after the second character of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"CCtest","desc":"RPUno Board /w atmega328p and LT3652","avr-gcc":"4.9"}}
```

##  /0/cctest? [load_step]|[start_step,end_step,bat_mv]

For zero or three argument test, first the program waits until the PV_IN is over 19V which means solar has charged the battery to its float level. Next, the load settings are stepped through for a record. Then the Loop enters now to discharge and reports as the Battery voltage crosses 50mV thresholds until the discharge voltage is reached. Cycling through a verify MPPT mode that starts charging up to the float voltage. Following an absorption cycle to complete the charge. After which it waits until Night (PV_IN less than 5V). And then the loop cycles back into discharge. If the serial line receives a character it will interrupt the test and enable charging.

One argument test sets the load step value and runs with the charger off until receiving a character on the UART.

``` 
/0/cctest?
{"DIS_A":"0.006","PV_V":"19.17","PWR_V":"6.69","TIME":"5","LD":"0"}
{"DIS_A":"0.030","PV_V":"19.14","PWR_V":"6.68","TIME":"2007","LD":"1"}
{"DIS_A":"0.036","PV_V":"19.04","PWR_V":"6.68","TIME":"4007","LD":"2"}
{"DIS_A":"0.045","PV_V":"19.07","PWR_V":"6.67","TIME":"6008","LD":"3"}
{"DIS_A":"0.050","PV_V":"19.04","PWR_V":"6.67","TIME":"8009","LD":"4"}
{"DIS_A":"0.059","PV_V":"19.04","PWR_V":"6.67","TIME":"10010","LD":"5"}
{"DIS_A":"0.066","PV_V":"19.04","PWR_V":"6.66","TIME":"12011","LD":"6"}
{"DIS_A":"0.073","PV_V":"19.04","PWR_V":"6.65","TIME":"14012","LD":"7"}
{"DIS_A":"0.082","PV_V":"19.04","PWR_V":"6.65","TIME":"16013","LD":"8"}
{"DIS_A":"0.089","PV_V":"19.04","PWR_V":"6.64","TIME":"18014","LD":"9"}
{"DIS_A":"0.098","PV_V":"19.04","PWR_V":"6.64","TIME":"20015","LD":"10"}
{"DIS_A":"0.105","PV_V":"19.04","PWR_V":"6.63","TIME":"22017","LD":"11"}
{"DIS_A":"0.113","PV_V":"19.04","PWR_V":"6.63","TIME":"24017","LD":"12"}
{"DIS_A":"0.121","PV_V":"19.04","PWR_V":"6.62","TIME":"26018","LD":"13"}
{"DIS_A":"0.128","PV_V":"19.01","PWR_V":"6.61","TIME":"28019","LD":"14"}
{"DIS_A":"0.135","PV_V":"19.01","PWR_V":"6.61","TIME":"30020","LD":"15"}
{"DIS_A":"0.119","PV_V":"19.01","PWR_V":"6.55","TIME":"79441","LD":"15"}
{"DIS_A":"0.119","PV_V":"19.04","PWR_V":"6.50","TIME":"139063","LD":"15"}
{"DIS_A":"0.118","PV_V":"19.14","PWR_V":"6.45","TIME":"275579","LD":"15"}
/0/cctest?
{"CHRG_A":"0.284","PV_V":"15.20","PWR_V":"6.61","TIME":"494596","LD":"0"}
```

# Notes

More debuging to do.

When I stopped the test and then stat it at the comand line I find it charging at 284mA @ 6.61V (i.e. about 1.9W) with a SLP003-12U solar PV module.

The battery discharge current reads near 136mA when the serial driver is more active and 119mA when it is not active, this is because the RS485 drivers were not sinking current at the moment of the 119mA reading. 