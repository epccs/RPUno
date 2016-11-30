/*
Solenoid is a command line controled demonstration of ATmega328p ... solenoid control
Copyright (C) 2016 Ronald Sutherland

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
#include "../lib/twi.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Uart/id.h"
#include "solenoid.h"

#define BLINK_DELAY 1000UL
static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t solenoids_initalized;

void ProcessCmd()
{ 
    if (solenoids_initalized) 
    {
        if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
        {
            Id("Solenoid");
        }
        if ( (strcmp_P( command, PSTR("/runtime")) == 0) && ( (arg_count == 2 ) ) )
        {
            RunTime();
        }
        if ( (strcmp_P( command, PSTR("/delay")) == 0) && ( (arg_count == 2 ) ) )
        {
            Delay();
        }
        if ( (strcmp_P( command, PSTR("/run")) == 0) && ( (arg_count == 2 ) ) )
        {
            Run();
        }
    }
    else
    {
            printf_P(PSTR("{\"err\":\"NotFinishKinit\"}\r\n"));
            initCommandBuffer();
            return;
    }
}

void setup(void) 
{
	// RPUuno has no LED, but the LED_BUILTIN is defined as digital 13 (SCK) anyway.
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    
    // Initialize Timers and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Initialize I2C, with the internal pull-up 
        note: I2C scan will stop without a pull-up on the bus */
    twi_init(1);

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
    
    // setup solenoid control
    init_K();
    
    // solenoids may have been previously latched so this
    // loads settings that will run a fast cycle
    Reset_All_K();
    solenoids_initalized = 0;
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

int main(void) 
{
    setup();

    while(1) 
    { 
        // use LED to show if I2C has a bus manager
        blink();
        
        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar());

            // address is an ascii value, warning: a null address would terminate the command string. 
            StartEchoWhenAddressed(rpu_addr);
        }
        
        // check if a character is available, and if so flush transmit buffer and nuke the command in process.
        // A multi-drop bus can have another device start transmitting after getting an address byte so
        // the first byte is used as a warning, it is the onlly chance to detect a possible collision.
        if ( command_done && uart0_available() )
        {
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
        
        // Solenoid Control is a function that moves them through different states that are timed with millis() or icp1 flow count.
        SolenoidControl();
        if (!solenoids_initalized)
        {
            uint8_t sleeping = 1;
            for(uint8_t i = 1; i <= SOLENOID_COUNT; i++)
            {
                if (Live(i))
                {                    
                    sleeping =0;
                    break;
                }
            }
            if (sleeping) 
            {
                solenoids_initalized = 1;
                // load setting from eeprom
            }
        }
    }        
    return 0;
}
