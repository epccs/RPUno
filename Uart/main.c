/*
Uart is a demonstration of Interrupt-Driven UART with stdio redirect. 
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

On Linux picocom can be used as a minimal serial terminal
https://github.com/npat-efault/picocom

picocom -b 9600 /dev/ttyUSB0
exit is C-a, C-x
*/
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include "../lib/uart.h"
#include "../lib/parse.h"

int main(void) {    

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    initCommandBuffer();
       
    sei(); // Enable global interrupts
    
    // non-blocking code in loop
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
        
        // check if the character is available, and if so stop transmit and the command in process.
        // a multi-drop bus can have another device start transmitting after the second received byte so
        // there is little time to detect a possible collision
        if ( command_done && uart0_available() )
        {
            // dump the transmit buffer to limit a collision 
            uart0_flush(); 
            initCommandBuffer();
        }
        
        // finish echo of the command line befor starting a reply (or the next part of reply)
        if ( command_done && (uart0_availableForWrite() == UART_TX0_BUFFER_SIZE) )
        {
            if ( !echo_on  )
            { // this happons when the address did not match
                initCommandBuffer();
            }
            else
            {
                // command is a pointer to string and arg[] is an array of pointers to strings
                // use findCommand to make them point to the correct places in the command line
                // this can only be done once, since spaces and delimeters are replaced with null termination
                if (command_done == 1)  
                {
                    findCommand();
                    command_done = 10;
                }
                
                // commands that would overflow the serial buffer are done in steps to prevent blocking
                // avr-libc has strcmp_P which is simular to strcmp but the second string is in program space.
                if ( (command_done >= 10) && (strcmp_P( command, PSTR("/id?")) == 0) )
                {
                    // /id? name 
                    // /id?
                    if ( (command_done == 10) && ( (arg_count == 0) || ( (arg_count == 1) && (strcmp_P( arg[0], PSTR("name")) == 0) ) ) )
                    {
                        printf_P(PSTR("{\"id\":{\"name\":\"Uart\"}}\r\n"));
                        initCommandBuffer();
                    }
                    // /id? desc
                    if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("desc")) == 0) )
                    {
                        printf_P(PSTR("{\"id\":{\"desc\":\"RPUno Board /w " ));
                        command_done = 11;
                    }
                    if ( (command_done == 11) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("desc")) == 0) )
                    {
                        printf_P(PSTR("ATmega328p and LT3652\"}}\r\n"));
                        initCommandBuffer();
                    }
                    // /id? avr-gcc
                    if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("avr-gcc")) == 0) )
                    {
                        printf_P(PSTR("{\"id\":{\"avr-gcc\":\"%d.%d\"}}\r\n"),__GNUC__,__GNUC_MINOR__);
                        initCommandBuffer();
                    }
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
