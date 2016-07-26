/*
Ht is a command line controled demonstration of the HT sensor usage
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
#include "../Uart/id.h"
#include "../Eeprom/ee.h"
#include "reflow.h"

// running the ADC burns power, which can be saved by delaying its use
#define ADC_DELAY_MILSEC 1000
static unsigned long adc_started_at;

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("Reflow");
    }
    if ( (strcmp_P( command, PSTR("/reflow?")) == 0) &&  (arg_count == 0 ) )
    {
        Reflow();
    }
    if ( (strcmp_P( command, PSTR("/ee?")) == 0) && (arg_count == 1 ) )
    {
        EEread();
    }
    if ( (strcmp_P( command, PSTR("/ee")) == 0) && (arg_count == 2 ) )
    {
        EEwrite();
    }
}

int main(void) 
{    
    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c 
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(INTERNAL_1V1); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup
    
    // setup()

    // put ADC in Auto Trigger mode and fetch an array of channels
    enable_ADC_auto_conversion();
    adc_started_at = millis();
    
    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();
    
    // Enable global interrupts starts TIMER0, UART0, ICP1 and other ISR's
    sei(); 
    
    // this start up command should run a reflow profile, e.g. after a reset.
    if (uart0_available() == 0)
    {
        strcpy_P(command_buf, PSTR("/0/reflow?"));
        command_done = 1;
        echo_on = 1;
        printf_P(PSTR("%s\r\n"), command_buf);
    }
    
    // loop() /* I am tyring to use non-blocking code */
    while(1) 
    {
        unsigned long kRuntime;
        
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
            // stop the reflow profile if someone has the comm
            initReflow();
            
            // dump the transmit buffer to limit a collision 
            uart0_flush(); 
            initCommandBuffer();
        }
        
        // delay between ADC reading
        kRuntime= millis() - adc_started_at;
        if ((kRuntime) > ((unsigned long)ADC_DELAY_MILSEC))
        {
            enable_ADC_auto_conversion();
            adc_started_at = millis();
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
