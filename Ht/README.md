# HT 

## Overview

Combines Capture and Adc to track tempeture, humidity, and solar power. 

Referance [Capture](../Capture) and [Adc](../Adc).

The capture ISR was shown to skip events when less than 300 machine cycles are available between the events, it was shown in [Capture](../Capture). This adds an Adc ISR which may cause skipping at even  higher machine cycle counts. 

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

With optiboot installed run 'make bootload' and it will compile and then flash the MCU the same way Arduino does.

``` 
rsutherland@straightneck:~/Samba/RPUno/Ht$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ht.o ht.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Capture/pwm.o ../Capture/pwm.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Capture/capture.o ../Capture/capture.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Uart/id.o ../Uart/id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/adc.o ../lib/adc.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/icp_buf.o ../lib/icp_buf.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/icp1.o ../lib/icp1.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Ht.map  -Wl,--gc-sections  -Wl,-u,vfprintf -lprintf_flt -lm -mmcu=atmega328p main.o ht.o ../Capture/pwm.o ../Capture/capture.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/icp_buf.o ../lib/icp1.o ../lib/parse.o -o Ht.elf
avr-size -C --mcu=atmega328p Ht.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:   13208 bytes (40.3% Full)
(.text + .data + .bootloader)

Data:        428 bytes (20.9% Full)
(.data + .bss + .noinit)


rm -f Ht.o main.o ht.o ../Capture/pwm.o ../Capture/capture.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/icp_buf.o ../lib/icp1.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex Ht.elf Ht.hex
rm -f Ht.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:Ht.hex

avrdude: Version 6.2
         Copyright (c) 2000-2005 Brian Dean, http://www.bdmicro.com/
         Copyright (c) 2007-2014 Joerg Wunsch

         System wide configuration file is "/etc/avrdude.conf"
         User configuration file is "/home/rsutherland/.avrduderc"
         User configuration file does not exist or is not a regular file, skipping

         Using Port                    : /dev/ttyUSB0
         Using Programmer              : arduino
         Overriding Baud Rate          : 115200
         AVR Part                      : ATmega328P
         Chip Erase delay              : 9000 us
         PAGEL                         : PD7
         BS2                           : PC2
         RESET disposition             : dedicated
         RETRY pulse                   : SCK
         serial program mode           : yes
         parallel program mode         : yes
         Timeout                       : 200
         StabDelay                     : 100
         CmdexeDelay                   : 25
         SyncLoops                     : 32
         ByteDelay                     : 0
         PollIndex                     : 3
         PollValue                     : 0x53
         Memory Detail                 :

                                  Block Poll               Page                       Polled
           Memory Type Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack
           ----------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------
           eeprom        65    20     4    0 no       1024    4      0  3600  3600 0xff 0xff
           flash         65     6   128    0 yes     32768  128    256  4500  4500 0xff 0xff
           lfuse          0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           hfuse          0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           efuse          0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           lock           0     0     0    0 no          1    0      0  4500  4500 0x00 0x00
           calibration    0     0     0    0 no          1    0      0     0     0 0x00 0x00
           signature      0     0     0    0 no          3    0      0     0     0 0x00 0x00

         Programmer Type : Arduino
         Description     : Arduino
         Hardware Version: 3
         Firmware Version: 4.4
         Vtarget         : 0.3 V
         Varef           : 0.3 V
         Oscillator      : 28.800 kHz
         SCK period      : 3.3 us

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e950f (probably m328p)
avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: NOTE: "flash" memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: reading input file "Ht.hex"
avrdude: input file Ht.hex auto detected as Intel Hex
avrdude: writing flash (13208 bytes):

Writing | ################################################## | 100% 2.03s

avrdude: 13208 bytes of flash written
avrdude: verifying flash memory against Ht.hex:
avrdude: load data flash data from input file Ht.hex:
avrdude: input file Ht.hex auto detected as Intel Hex
avrdude: input file Ht.hex contains 13208 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 1.44s

avrdude: verifying ...
avrdude: 13208 bytes of flash verified

avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 115200 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 115200 /dev/ttyUSB0" stuff,ht.log
``` 


# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after second charactor of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Capture"}}
```

## /0/initICP icp1,(rise|fall|both),(0..7)

Initialize Input Capture ICP1. Set the Input Capture Edge Select mode. Set the prescale (0 = no clock, 1 = MCU clock, 2 = MCU clock/8... see datasheet for others. Note some are not based on the MCU clock)

``` 
/0/initICP icp1,both,1
{"icp1":{"edgSel":"both", "preScalr":"1"}}
```

## /0/count? [icp1]

Return count of ICP1 (ATmega328 pin PB0. Uno pin 8) event captures. A zero means icp1 captures are not happening (because I forgot to connect a sensor).

``` 
/0/count? icp1
{"icp1":{"count":"0"}}
{"icp1":{"count":"0"}}
{"icp1":{"count":"0"}}
```

## /0/capture? [icp1,1..15] 

return ICP1 timer delta(s) as a pair of low and high timing values from the buffered capture events. These can be used to find the duty or period. The count is the number of event captures (now I've connected a [HT](http://epccs.org/indexes/Board/HT/) sensor). Note that a group of consecutive captures is, in fact, continous, there are no timing gaps. The resolution of continuous timing events can approach that of the stability of the timing sources (both timing sources have to be stable to gain maximum resolution). Pulse interpolation is a way of using a fast time source to measure a slow time source, but the fast time source occurs in buckets (e.g. quantum) and to measure something with those buckets to 100ppm requires 10k of them. 

``` 
/0/capture? icp1,3
{"icp1":{"count":"11642055","low":"1606","high":"349","status":"0"}}
{"icp1":{"count":"11642053","low":"1609","high":"349","status":"0"}}
{"icp1":{"count":"11642051","low":"1603","high":"349","status":"0"}}
{"icp1":{"count":"11805739","low":"1606","high":"349","status":"0"}}
{"icp1":{"count":"11805737","low":"1603","high":"348","status":"0"}}
{"icp1":{"count":"11805735","low":"1607","high":"349","status":"0"}}
{"icp1":{"count":"11969471","low":"1607","high":"348","status":"1"}}
{"icp1":{"count":"11969469","low":"1603","high":"349","status":"1"}}
{"icp1":{"count":"11969467","low":"1607","high":"349","status":"1"}}
```

It takes three events to aggregate the data for a capture report. The status reports only the most recent event status since the other two events can be inferred. 

## /0/event? [icp1,1..31] 

return ICP1 event timer values as a 16 bit unsign integer, which continuously rolls over. The status bit 0 shows rising (0 is falling) edge.

``` 
/0/event? icp1,3
{"icp1":{"count":"2880842","event":"850","status":"1"}}
{"icp1":{"count":"2880841","event":"64779","status":"0"}}
{"icp1":{"count":"2880840","event":"64426","status":"1"}}
```

Perhaps that is confusing. The event with a count of 2880840 had a rising edge and happened before the event with count 2880841 which had a falling edge. The difference of the event times is the number of clocks spent high (64779 - 64426 = 353). And the difference between the middle and most recent events is the number of clocks spent low ((850+2^16) - 64779 = 1607).

## /0/ht? 

Return capture count, capture sample size, Temperature and RH using ICP1. Additionaly solar input voltage and battery voltage from ADC.

``` 
/0/ht?
{"count":"2642422","smpl_sz":"15","deg_f":"83.44","%RH":"10.98","PV_IN":"28.00""PWR":" 6.75"}
{"count":"2658000","smpl_sz":"15","deg_f":"83.45","%RH":"10.89","PV_IN":"28.00""PWR":" 6.75"}
{"count":"2673576","smpl_sz":"15","deg_f":"83.44","%RH":"11.13","PV_IN":"28.00""PWR":" 6.75"}

``` 


# Notes

## Pulse Skipping

With an HT sensor, as the temperature goes up the high count goes down for example near Room temp I see 

``` 
{"icp1":{"count":"519107161","low":"1669","high":"550","status":"0"}}
``` 

As the heat is turned up the capture looks like 

``` 
{"icp1":{"count":"514717561","low":"1646","high":"199","status":"0"}}
``` 

and some start to show skipping

``` 
{"icp1":{"count":"514717563","low":"1645","high":"2045","status":"0"}}
``` 

The short pulse is the one that skips because the ISR is not able to change edge detection in time to see the falling edge of the short high pulse so it has to wait for the next falling edge. It takes about 50 machine cycles to enter an interrupt (since all registers used have to be preserved) and then a similar amount of time to restore the interrupted process to its original state. 

Unfortunately, it is the other ISR's like Timer 0 (zero) used for millis() timing that is causing the skips. I need those interrupts to run so this is going to be something I will live with. The minimum pulse width that will ensure a capture is about 50 counts more than the longest interrupt, I am not seeing skips on pulses above 300 counts (18.75 uSec). The program will still gather data down to about 130 counts but some skipping will show up need tossed (they are sampling errors, e.g. outlier). 

Some sensors like flow meters do not need both edges tracked, so the capture can be initialized to track one edge, which may allow pulse events to be gathered at up to about 30kHz. The "capture?" command will not work with only one edge.
