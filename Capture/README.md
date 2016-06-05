# Capture 

## Overview

Timer 1 Input Capture (ICP1) is on the Arduino Uno pin 8. 

Referance ATmega328 datasheet 16.6 Input Capture Unit (page 118). A capture copies the value in timer TCNT1 into ICR1, which is a timestamp of the event.

Note the ISR needs about 300 machine cycles to finish. If events happen faster than 300 machine cycles, they may be skipped.

For how I setup my Makefile toolchain <http://epccs.org/indexes/Document/DvlpNotes/LinuxBoxCrossCompiler.html>.

With optiboot installed run 'make bootload' and it will compile and then flash the MCU the same way Arduino does, but without any Arduino stuff.

``` 
rsutherland@straightneck:~/Samba/RPUno/Capture$ make bootload
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o main.o main.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o process.o process.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o pwm.o pwm.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o count.o count.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o capture.o capture.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../Uart/id.o ../Uart/id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/adc.o ../lib/adc.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/icp1.o ../lib/icp1.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Capture.map  -Wl,--gc-sections  -mmcu=atmega328p main.o process.o pwm.o count.o capture.o ../Uart/id.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/icp1.o ../lib/parse.o -o Capture.elf
avr-size -C --mcu=atmega328p Capture.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:    8162 bytes (24.9% Full)
(.text + .data + .bootloader)

Data:        406 bytes (19.8% Full)
(.data + .bss + .noinit)

...
avrdude: verifying ...
avrdude: 8162 bytes of flash verified

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
script -f -c "picocom -b 115200 /dev/ttyUSB0" stuff,capture.log
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


## /0/pwm oc2a|oc2b,0..255

Pulse width modulation using OC2A (ATmega328 pin PB3. Uno pin 11) or OC2B (ATmega328 pin PD3. Uno pin 3) can be used to feed the ICP1 input. Note that timer2 is used with OC2[A|B], while timer1 is needed for ICP1.

``` 
/0/pwm oc2a,127
{"pwm":{"OCR2A":"127"}}
``` 

# Notes

## Pulse Skipping

With an HT as the temperature goes up the high count goes down for example near Room temp I see 

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

Unfortunately, it is the other ISR's like Timer 0 (zero) used for millis() timing that is causing the skips. I need those interrupts to run so this is going to be something I will live with. The minimum pulse width that will ensure a capture is about 300 counts or 18.75 uSec. It will still gather data down to about 150 counts but some skipping will show up so the data will need outliers tossed. 

Some sensors like flow meters do not need both edges tracked, so the capture can be initialized to track one edge, which may allow pulse events to be gathered at up to about 30kHz. The "capture?" command will not work with only one edge.
