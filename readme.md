# Software for RPUno which has an ATmega328p

From <http://epccs.org/hg/open/RPUno>

The Board is at <http://epccs.org/indexes/Board/RPUno/>, it is basically an Arduino Uno with a solar charge controller. 

ATmega328 fuse setting

    # http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p&LOW=FF&HIGH=DE&EXTENDED=05&LOCKBIT=0F
    uno.bootloader.low_fuses=0xFF
    # [CKDIV8:CKOUT:SUT1:SUT0:CKSEL3:CKSEL2:CKSEL1:CKSEL0] 1 1 1 1 1111
    # Ext. Crystal Osc. >8 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 16mS
    uno.bootloader.high_fuses=0xDE
    # [RSTDISBL:DWEN:SPIEN:WDTON:EESAVE:BOOTSZ1:BOOTSZ0:BOOTRST] 1* 1* 0* 1 1 111
    # Boot flash size is 256 for optiboot, which may not fit when compiled from some versions of avr-gcc so check with care.
    # * changing these will brick the MCU as far as ISCP over SPI goes
    uno.bootloader.extended_fuses=0x05
    # Brown-out detection at 2V7
    uno.bootloader.unlock_bits=0x3F
    # 0x3F is same as 0xFF which is fully open
    uno.bootloader.lock_bits=0x0F
    # 0x0F prohibits LPM and SPM instruction in bootloader section
    uno.bootloader.file=optiboot/optiboot_atmega328.hex
    # https://github.com/Optiboot/optiboot

       
## TBD


    
