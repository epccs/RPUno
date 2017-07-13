# Makefile 
TARGET = NightLight
LIBDIR = ../lib
OBJECTS = main.o \
	nightlight.o \
	../Uart/id.o \
	../DayNight/day_night.o \
	../Adc/analog.o \
	../Adc/references.o \
	../Capture/capture.o \
	$(LIBDIR)/timers.o \
	$(LIBDIR)/uart.o \
	$(LIBDIR)/icp_buf.o \
	$(LIBDIR)/icp1.o \
	$(LIBDIR)/twi.o \
	$(LIBDIR)/rpu_mgr.o \
	$(LIBDIR)/adc.o \
	$(LIBDIR)/parse.o

## Chip and project-specific global definitions
MCU   =  atmega328p
F_CPU = 16000000UL  
BAUD  =  38400UL
CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I. 

## Cross-compilation
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

## programing ports. the FTDI on my board shows as ttyUSBx, while an Uno shows as ttyACMx
BOOT_PORT = /dev/ttyUSB0
ISP_PORT = /dev/ttyACM0

## Compiler/linker options
CFLAGS = -Os -g -std=gnu99 -Wall
# CFLAGS += -funsigned-char -funsigned-bitfields 
# CFLAGS += -fpack-struct -fshort-enums 
CFLAGS += -ffunction-sections -fdata-sections 

TARGET_ARCH = -mmcu=$(MCU)

LDFLAGS = -Wl,-Map,$(TARGET).map 
LDFLAGS += -Wl,--gc-sections 

## printf() is one of the vfprintf() family of functions which does not implement floating point conversion by default
## warning this takes about 2k of program space
LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm

all: $(TARGET).hex

$(TARGET): $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	rm -f $(TARGET).elf

## works with optiboot which erases flash without being told (e.g. -e )
bootload: $(TARGET).hex
	avrdude -v -p $(MCU) -c arduino -P $(BOOT_PORT) -b 115200 -U flash:w:$(TARGET).hex

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@
	$(SIZE) -C --mcu=$(MCU) $@
	rm -f $(TARGET).o $(OBJECTS)

clean: 
	rm -f $(TARGET).hex $(TARGET).map
 
