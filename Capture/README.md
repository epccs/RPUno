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

Program:    9650 bytes (29.4% Full)
(.text + .data + .bootloader)

Data:        607 bytes (29.6% Full)
(.data + .bss + .noinit)

...
avrdude: verifying ...
avrdude: 9650 bytes of flash verified

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

# Commands

Commands are interactive over the serial interface at 115200 baud rate. The echo will start after second charactor of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/0/id?
{"id":{"name":"Capture"}}
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
{"icp1":{"count":"571415","low":"1643","high":"435","status":"0"}}
{"icp1":{"count":"571413","low":"1646","high":"435","status":"0"}}
{"icp1":{"count":"571411","low":"1646","high":"435","status":"0"}}
{"icp1":{"count":"586795","low":"1646","high":"435","status":"0"}}
{"icp1":{"count":"586793","low":"1647","high":"437","status":"0"}}
{"icp1":{"count":"586791","low":"1646","high":"437","status":"0"}}
{"icp1":{"count":"602193","low":"1645","high":"435","status":"64"}}
{"icp1":{"count":"602191","low":"1645","high":"437","status":"4"}}
{"icp1":{"count":"602189","low":"1645","high":"436","status":"0"}}

```

The status of 64 means the last event of the capture report had an ICF1 (capture) flag set while running the overflow ISR. The status of 4 means the first event of the capture report had its ICF1 flag set while running the overflow ISR. It takes three events to aggregate the data for a capture report, so the flag status can show in two reports (as the first and last event). The flag will only show once if it is in a middle event. 

## /0/event? [icp1,1..31] 

return ICP1 event timer values as a 32 bit unsign integer, which continuously rolls over. The status bit 0 shows rising (0 is falling) edge, bit 1 is for TOV1 (overflow flag) set while in capture ISR, bit 2 is for ICF1 (capture) set while in overflow ISR, bit 3 is set when a memory check byte fails (e.g. capture data was changed by a user program or hardware error).

``` 
/0/event? icp1,3
{"icp1":{"count":"12300654","event":"4236314232","status":"1"}}
{"icp1":{"count":"12300653","event":"4236312588","status":"4"}}
{"icp1":{"count":"12300652","event":"4236312143","status":"1"}}
```

Perhaps that is confusing. The event with a count of 12300652 had a rising edge and happened before the event with count 12300653 which had a falling edge. The difference of the event times is the number of clocks spent high (4236312588 - 4236312143 = 445). And the difference between the middle and most recent events is the number of clocks spent low (4236314232 - 4236312588 = 1644). The status 4 means an ICF1 (capture) flag was set while runing the overflow ISR.


## /0/pwm oc2a|oc2b,0..255

Pulse width modulation using OC2A (ATmega328 pin PB3. Uno pin 11) or OC2B (ATmega328 pin PD3. Uno pin 3) can be used to feed the ICP1 input. Note that timer2 is used with OC2[A|B], while timer1 is needed for ICP1.

``` 
/0/pwm oc2a,127
{"pwm":{"OCR2A":"127"}}
``` 

# Notes

## Corrupted Capture

From time to time the captured data is corrupted. I have added byte checks at the time of capture and the time of copying the captures to a buffer for reporting. This was done to check if the memory gets corrupted in place but that does not seem to be the case. 

The corruption seems to be prssent at the time the ISR records the data.

I have now added status that shows if the overflow flag is set while running the capture ISR and if the capture flag is set while running the overflow ISR. It is not yet clear if this can be used to fix a broken event but I think it points out the cause of the problem.


