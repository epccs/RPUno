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
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o id.o id.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o pwm.o pwm.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o count.o count.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o capture.o capture.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/timers.o ../lib/timers.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/uart.o ../lib/uart.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/adc.o ../lib/adc.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/icp1.o ../lib/icp1.c
avr-gcc -Os -g -std=gnu99 -Wall -ffunction-sections -fdata-sections  -DF_CPU=16000000UL   -DBAUD=115200UL -I.  -mmcu=atmega328p -c -o ../lib/parse.o ../lib/parse.c
avr-gcc -Wl,-Map,Capture.map  -Wl,--gc-sections  -mmcu=atmega328p main.o process.o id.o pwm.o count.o capture.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/icp1.o ../lib/parse.o -o Capture.elf
avr-size Capture.elf
   text    data     bss     dec     hex filename
   7600      16     512    8128    1fc0 Capture.elf
rm -f Capture.o main.o process.o id.o pwm.o count.o capture.o ../lib/timers.o ../lib/uart.o ../lib/adc.o ../lib/icp1.o ../lib/parse.o
avr-objcopy -j .text -j .data -O ihex Capture.elf Capture.hex
rm -f Capture.elf
avrdude -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:Capture.hex

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
avrdude: reading input file "Capture.hex"
avrdude: input file Capture.hex auto detected as Intel Hex
avrdude: writing flash (7616 bytes):

Writing | ################################################## | 100% 1.08s

avrdude: 7616 bytes of flash written
avrdude: verifying flash memory against Capture.hex:
avrdude: load data flash data from input file Capture.hex:
avrdude: input file Capture.hex auto detected as Intel Hex
avrdude: input file Capture.hex contains 7616 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.78s

avrdude: verifying ...
avrdude: 7616 bytes of flash verified

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
{"icp1":{"count":"598068","low":"1731","high":"530"}}
{"icp1":{"count":"598066","low":"1729","high":"530"}}
{"icp1":{"count":"598064","low":"1732","high":"530"}}
{"icp1":{"count":"739543","low":"1734","high":"531"}}
{"icp1":{"count":"739541","low":"1729","high":"531"}}
{"icp1":{"count":"739539","low":"1734","high":"530"}}
{"icp1":{"count":"881007","low":"1732","high":"530"}}
{"icp1":{"count":"881005","low":"1736","high":"531"}}
{"icp1":{"count":"881003","low":"1735","high":"531"}}
```

## /0/pwm oc2a|oc2b,0..255

Pulse width modulation using OC2A (ATmega328 pin PB3. Uno pin 11) or OC2B (ATmega328 pin PD3. Uno pin 3) can be used to feed the ICP1 input. Note that timer2 is used with OC2[A|B], while timer1 is needed for ICP1.

``` 
/0/pwm oc2a,127
{"pwm":{"OCR2A":"127"}}
``` 