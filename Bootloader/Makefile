# Makefile
TARGET = optiboot_rpuno
OBJECTS = main.o

## Chip and project-specific global definitions
MCU = atmega328p
F_CPU = 16000000L
BAUD = 115200
LED = B5
LED_START_FLASHES = 3
BOOT_FLASH_SECTION_START = 0x7e00
VERSION_PLACED_AT_FLASH_LOCATION = 0x7ffe
LOW_FUSE = 0xFF
HIGH_FUSE = 0xDE
EXTENDED_FUSE = 0xFD
UNLOCK_FUSE = 0x3F
LOCK_FUSE = 0x2F

## Cross-compilation
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

## programing ports. the FTDI on my board shows as ttyUSBx, while an Uno shows as ttyACMx
BOOT_PORT = /dev/ttyUSB0
ISP_PORT = /dev/ttyACM0

## Compiler/linker options
COMMON_OPTIONS = -DBAUD=$(BAUD) -DLED_START_FLASHES=$(LED_START_FLASHES)
COMMON_OPTIONS += $(LED_DATA_FLASH_CMD) -DLED=$(LED) 

# BIGBOOT adds extra features, but takes up 1K.
# COMMON_OPTIONS += -DBIGBOOT=1

# SOFT_UART: AVR305 soft-UART instead of hardware UART
# COMMON_OPTIONS += -DSOFT_UART=1

# LED_DATA_FLASH: Flash LED when transferring data
# COMMON_OPTIONS += -DLED_DATA_FLASH=1

# UART: number (0..n) for devices with more than  one hardware uart (644P, 1284P, etc)
# COMMON_OPTIONS += -DUART=0

CFLAGS  = -g -Wall -Os -fno-split-wide-types -mrelax -fno-caller-saves -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += $(COMMON_OPTIONS)

LDFLAGS = -Wl,--section-start=.text=$(BOOT_FLASH_SECTION_START) -Wl,--section-start=.version=$(VERSION_PLACED_AT_FLASH_LOCATION) -Wl,--relax -nostartfiles -nostdlib
#-Wl,--gc-sections


# Note about fuses:
# the efuse should really be 0xf8; since, however, only the lower
# three bits of that byte are used on the atmega168, avrdude gets
# confused if you specify 1's for the higher bits.
#
# similarly, the lock bits should be 0xff instead of 0x3f (to
# unlock the bootloader section) and 0xcf instead of 0x2f (to
# lock it), but since the high two bits of the lock byte are
# unused, avrdude would get confused.

all: $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).lst

FORCE:

baudcheck: FORCE
	- @$(CC) --version
	- @$(CC) $(CFLAGS) -E baudcheck.c -o baudcheck.tmp.sh
	- @bash baudcheck.tmp.sh
	rm -f baudcheck.tmp.sh

# platforms support EEPROM and large bootloaders need the eeprom functions that
# are defined in libc (e.g. the -lc option), even though it was explicity remove it with 
# -nostdlib to save space.
%.elf: $(OBJECTS) baudcheck FORCE
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -lc
	$(SIZE) $@

fuse: 
	avrdude -v -p $(MCU) -c stk500v1 -P /dev/ttyACM0 -b 19200 -e -U lock:w:$(UNLOCK_FUSE):m -U lfuse:w:$(LOW_FUSE):m -U hfuse:w:$(HIGH_FUSE):m -U efuse:w:$(EXTENDED_FUSE):m

# ISCP tool is a Uno with ArduinoISP sketch: https://github.com/arduino/Arduino/blob/master/build/shared/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino
isp: $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -e -U flash:w:$(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex -U lock:w:$(LOCK_FUSE):m

clean:
	rm -f *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex *.tmp.sh

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O binary $< $@
