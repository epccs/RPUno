/*
Capture is a command line controled demonstration of Input Capture Unit
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
#include "../lib/adc.h"
#include "../lib/icp1.h"
#include "../Uart/id.h"
#include "pwm.h"
#include "capture.h"

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("Capture");
    }
    if ( (strcmp_P( command, PSTR("/pwm")) == 0) )
    {
        Pwm();
    }
    if ( (strcmp_P( command, PSTR("/count?")) == 0) &&  ( (arg_count == 0) || ( (arg_count == 1) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        Count();
    }
    if ( (strcmp_P( command, PSTR("/capture?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        Capture();
    }
    if ( (strcmp_P( command, PSTR("/event?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        Event();
    }
    if ( (strcmp_P( command, PSTR("/initICP")) == 0) && ( ( (arg_count == 3) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        InitICP();
    }
}

int main(void) 
{    
    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c 
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup
    
    // setup()

    /* Initialize Input Capture Unit 1 */
    initIcp1(TRACK_BOTH, ICP1_MCUCLOCK) ;
    
    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();

    sei(); // Enable global interrupts starts TIMER0, UART0, ICP1 and other ISR's
    
    // loop() /* I am tyring to use non-blocking code */
    while(1) 
    {
        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar());

            // address is the ascii value for '0' note: a null address will terminate the command string. 
            StartEchoWhenAddressed('0');
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
    }        
    return 0;
}
