/*
Reflow is a command line controlled demonstration that controls an SSR to modulate power into an IR oven
Copyright (C) 2017 Ronald Sutherland

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

For a copy of the GNU General Public License use
http://www.gnu.org/licenses/gpl-2.0.html
*/
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include "../lib/uart.h"
#include "../lib/parse.h"
#include "../lib/timers.h"
#include "../lib/adc.h"
#include "../lib/twi.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Uart/id.h"
#include "../i2c-debug/i2c-scan.h"
#include "../i2c-debug/i2c-cmd.h"
#include "../Eeprom/ee.h"
#include "reflow.h"

#define ADC_DELAY_MILSEC 100UL
static unsigned long adc_started_at;

#define BLINK_DELAY 1000UL
static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("Reflow"); // ../Uart/id.c
    }
    if ( (strcmp_P( command, PSTR("/reflow?")) == 0) &&  (arg_count == 0 ) )
    {
        Reflow(); // reflow.c
    }
    if ( (strcmp_P( command, PSTR("/ee?")) == 0) && (arg_count == 1 ) )
    {
        EEread(); // ../Eeprom/ee.c
    }
    if ( (strcmp_P( command, PSTR("/ee")) == 0) && (arg_count == 2 ) )
    {
        EEwrite(); // ../Eeprom/ee.c
    }
    if ( (strcmp_P( command, PSTR("/iscan?")) == 0) && (arg_count == 0) )
    {
        I2c_scan(); // ../i2c-debug/i2c-scan.c
    }
    if ( (strcmp_P( command, PSTR("/iaddr")) == 0) && (arg_count == 1) )
    {
        I2c_address(); // ../i2c-debug/i2c-cmd.c
    }
    if ( (strcmp_P( command, PSTR("/ibuff")) == 0) )
    {
        I2c_txBuffer(); // ../i2c-debug/i2c-cmd.c
    }
    if ( (strcmp_P( command, PSTR("/ibuff?")) == 0) && (arg_count == 0) )
    {
        I2c_txBuffer(); // ../i2c-debug/i2c-cmd.c
    }
    if ( (strcmp_P( command, PSTR("/iwrite")) == 0) && (arg_count == 0) )
    {
        I2c_write(); // ../i2c-debug/i2c-cmd.c
    }
    if ( (strcmp_P( command, PSTR("/iread?")) == 0) && (arg_count == 1) )
    {
        I2c_read(); // ../i2c-debug/i2c-cmd.c
    }
}

void setup(void) 
{
	// RPUuno has no LED, but the LED_BUILTIN is defined as digital 13 (SCK).
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    
    // RPUno current sources. see ../lib/pins_board.h
    pinMode(CS_EN,OUTPUT);
    digitalWrite(CS_EN,HIGH);

    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(INTERNAL_1V1); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    // put ADC in Auto Trigger mode and fetch an array of channels
    enable_ADC_auto_conversion(BURST_MODE);
    adc_started_at = millis();

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Initialize I2C. note: I2C scan will stop without a pull-up on the bus */
    twi_init(TWI_PULLUP);

    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();

    // Enable global interrupts to start TIMER0 and UART ISR's
    sei(); 
    
    blink_started_at = millis();
    
    rpu_addr = get_Rpu_address();
    blink_delay = BLINK_DELAY;
    
    // blink fast if a default address from RPU manager not found
    if (rpu_addr == 0)
    {
        rpu_addr = '0';
        blink_delay = BLINK_DELAY/4;
    }
    
    // this start up command runs a reflow profile after reset.
    if (uart0_available() == 0)
    {
        strcpy_P(command_buf, PSTR("/0/reflow?"));
        command_buf[1] = rpu_addr; // hack the correct address onto the buffer
        command_done = 1;
        echo_on = 1;
        printf_P(PSTR("%s\r\n"), command_buf);
    }
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        digitalToggle(LED_BUILTIN);
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

void adc_burst(void)
{
    unsigned long kRuntime= millis() - adc_started_at;
    if ((kRuntime) > ((unsigned long)ADC_DELAY_MILSEC))
    {
        enable_ADC_auto_conversion(BURST_MODE); // ../lib/adc.c
        adc_started_at += ADC_DELAY_MILSEC; 
    } 
}

int main(void) 
{    
    setup();

    while(1) 
    {
        // use LED_BUILTIN to show if I2C has a bus manager
        blink();

        // delay between ADC reading
        adc_burst();
        
        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar());

            // address is the ascii value for '0' note: a null address will terminate the command string. 
            StartEchoWhenAddressed(rpu_addr);
        }
        
        // check if a character is available, and if so flush transmit buffer and nuke the command in process.
        // A multi-drop bus can have another device start transmitting after getting an address byte so
        // the first byte is used as a warning, it is the onlly chance to detect a possible collision.
        if ( command_done && uart0_available() )
        {
            // stop the reflow profile if someone has the comm
            initReflow();
            
            // dump the transmit buffer to limit a collision 
            uart0_flush(); 
            initCommandBuffer();
        }

        // finish echo of the command line befor starting a reply (or the next part of a reply)
        if ( command_done && (uart0_availableForWrite() == UART_TX0_BUFFER_SIZE) )
        {
            if ( !echo_on  )
            { // this happons when the address did not match 
                initCommandBuffer();
            }
            else
            {
                if (command_done == 1)  
                {
                    findCommand();
                    command_done = 10;
                }
                
                // do not overfill the serial buffer since that blocks looping, e.g. process a command in 32 byte chunks
                if ( (command_done >= 10) && (command_done < 250) )
                {
                     ProcessCmd();
                }
                else 
                {
                    initCommandBuffer();
                }
            }
         }
    }        
    return 0;
}
