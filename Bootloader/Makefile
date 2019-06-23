# Makefile
TARGET = optiboot_rpuno
OBJECTS = main.o

## Chip and project-specific global definitions
MCU = atmega328p
F_CPU = 16000000UL
BAUD = 38400UL
LED = B5
LED_START_FLASHES = 0
BOOT_FLASH_SECTION_START = 0x7e00
VERSION_PLACED_AT_FLASH_LOCATION = 0x7ffe

# the efuse uses the lower three bits, so some tools read it as 0x05.
# also the lockbit uses the lower five bits, so some tools read unlock as 0xFF and lock as 0xEF
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p&LOW=FF&HIGH=DE&EXTENDED=05&LOCKBIT=3F
LOW_FUSE = 0xFF
HIGH_FUSE = 0xDE
EXTENDED_FUSE = 0xFD
# avrdude -v <= 6.2 
#UNLOCK_FUSE = 0x3F
#LOCK_FUSE = 0x2F
# avrdude -v >= 6.3 : "1" is returned for unused bits for -c stk500v1
UNLOCK_FUSE = 0xFF
LOCK_FUSE = 0xEF

CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)
CPPFLAGS += -DLED_START_FLASHES=$(LED_START_FLASHES)
CPPFLAGS += $(LED_DATA_FLASH_CMD) -DLED=$(LED) 

# BIGBOOT adds extra features, but takes up 1K.
# CPPFLAGS += -DBIGBOOT=1

# SOFT_UART: AVR305 soft-UART instead of hardware UART
# CPPFLAGS += -DSOFT_UART=1

# LED_DATA_FLASH: Flash LED when transferring data
# CPPFLAGS += -DLED_DATA_FLASH=1

# UART: number (0..n) for devices with more than  one hardware uart (644P, 1284P, etc)
# CPPFLAGS += -DUART=0


## Cross-compilation
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

# FTDI's USB to serial bridge shows as /dev/ttyUSB0, 
# Uno's serial bridge (an ATmega16U2) shows as /dev/ttyACM0  (a modem,?)
# Pi Zero on chip hardware serial shows as /dev/ttyAMA0 (hardware UART on a Linux system)
ISP_PORT = /dev/ttyACM0

## set the environment variable used for compiler and linker options for a baudcheck to be done
# https://wiki.gentoo.org/wiki/GCC_optimization
CFLAGS  = -g -Wall -Os -fno-split-wide-types -mrelax -fno-caller-saves 

TARGET_ARCH = -mmcu=$(MCU)

# bootloader flags, e.g. omit the interrupt vector table and optiboot does not use a stack frame
# https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html#AVR-Options
LDFLAGS = -Wl,--section-start=.text=$(BOOT_FLASH_SECTION_START) -Wl,--section-start=.version=$(VERSION_PLACED_AT_FLASH_LOCATION) -Wl,--relax -nostartfiles -nostdlib
#-Wl,--gc-sections
# tbd: does the new option "-flto" have advantage over such things as "-ffunction-sections -fdata-sections" with "-Wl,--gc-sections -Wl,--print-gc-sections"
# issue with stack on some compiler versions: https://github.com/Optiboot/optiboot/issues/160

all: $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).lst

# use the C preprocessor to make a shell script which bash can run to check the baudrate
baudcheck: 
	- @$(CC) --version
	- @$(CC) $(CFLAGS) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -E baudcheck.c -o baudcheck.tmp.sh
	- @bash baudcheck.tmp.sh
	rm -f baudcheck.tmp.sh

# platforms support EEPROM and large bootloaders need the eeprom functions that
# are defined in libc (e.g. the -lc option), even though they were explicity remove with 
# -nostdlib to save space.
%.elf: $(OBJECTS) baudcheck
	$(CC) $(LDFLAGS) $(TARGET_ARCH) -o $@ $< -lc
	$(SIZE) -C --mcu=$(MCU) $@

fuse: 
	avrdude -v -p $(MCU) -c stk500v1 -P /dev/ttyACM0 -b 19200 -e -U lock:w:$(UNLOCK_FUSE):m -U lfuse:w:$(LOW_FUSE):m -U hfuse:w:$(HIGH_FUSE):m -U efuse:w:$(EXTENDED_FUSE):m

## set fuse like they are from factory, i.e. 8MHz internal, B.O.D. disabled.
oem: 
	avrdude -v -p atmega328p -c stk500v1 -P /dev/ttyACM0 -b 19200 -e -U lock:w:$(UNLOCK_FUSE):m -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

# the Uno (with ArduinoISP sketch) is used as ICSP tool
# https://github.com/arduino/Arduino/blob/master/build/shared/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino
# save the eeprom so it can be put back after bootloader install. Note the -e will set all EEPROM addresses to '0xFF'.
isp:
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -U eeprom:r:$(TARGET)_$(MCU)_eeprom.hex:i
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -e -U flash:w:$(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex -U lock:w:$(LOCK_FUSE):m

# I am not sure where the avrdude in raspian is from but it may be this one
# https://github.com/facchinm/avrdude
# Raspian version is missing this change sudo for now 
# https://github.com/facchinm/avrdude/commit/aa0d28f774147fb03f8b18cd065586c7a93a2b0b
linuxspi:
	sudo avrdude -v -p $(MCU) -c linuxspi -P /dev/spidev0.0 -U eeprom:r:$(TARGET)_$(MCU)_eeprom.hex:i
	sudo avrdude -v -p $(MCU) -c linuxspi -P /dev/spidev0.0 -e -U flash:w:$(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex -U lock:w:$(LOCK_FUSE):m

# recover a saved eeprom
recoveree:
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -U eeprom:w:$(TARGET)_$(MCU)_eeprom.hex
	rm $(TARGET)_$(MCU)_eeprom.hex
	
ree_linuxspi:
	avrdude -v -p $(MCU) -c linuxspi -P /dev/spidev0.0 -U eeprom:w:$(TARGET)_$(MCU)_eeprom.hex
	rm $(TARGET)_$(MCU)_eeprom.hex

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
