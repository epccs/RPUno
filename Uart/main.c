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
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "../lib/uart.h"

// command buffer e.g. /0/id?
#define COMMAND_BUFFER_SIZE 16
#define COMMAND_BUFFER_MASK (COMMAND_BUFFER_SIZE - 1)

int main(void) {    

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
                
    char input;
    char command[COMMAND_BUFFER_SIZE];
    command[1] = '\0';  // best to set address as null value at startup
    uint8_t command_head =0;
    uint8_t command_done = 0;
    
    uint8_t echo_on =0;

    sei(); // Enable global interrupts
    
    // Arduino has an F() macro that cast the avr-libc PSTR() macro from pgmspace.h into a C++ string_literal
    // PSTR() gives a static pointer to a C string so that is what I want.
    // printf_P and puts_P are also found in avr-libc and used when the string is in program memory. 
    printf_P(PSTR("I only know the command \"/0/id?\"\r\n")); 
    printf_P(PSTR("echo will start after \"/0\" which is my address\r\n")); 
    
    // this will wait until the TX buffer can accept more input. Blocking 
    // happons when uart0_putc has to wait for free space in the buffer.
    // Arduino and Wring do similar
    printf_P(PSTR("only %d byte is availableForWrite so I've been blocking the program\r\nLets Start\r\n"), uart0_availableForWrite()); //debug

    while(1) 
    {
        if ( (!command_done) && uart0_available() ) // check if a character is available to, e.g. non-blocking
        {
            // get a character from stdin
            input = getchar(); 
        
            // a return or new-line finishes the command (or starts a new command)
            if ( (input == '\r') || (input == '\n') ) // pressing enter in picocom sends a \r
            {
                //echo both carrage return and newline.
                if (echo_on) printf("\r\n");
                
                // finish command as a null terminated string
                if (command_head < (COMMAND_BUFFER_SIZE - 1) )
                {
                    command[command_head] = '\0';
                    ++command_head;
                }
                else // command is bad
                {
                    command[1] = '\0';  // best to set address as null value
                    command_done = 0;
                    command_head =0;
                    echo_on = 0;
                }
                //printf("cmd: %s\r\n", command); //debuging
                command_done = 1;                     
            }
            else
            {
                //echo the input  
                if (echo_on) printf("%c", input);

                // assemble the command
                if (command_head < (COMMAND_BUFFER_SIZE - 1) )
                {
                    command[command_head] = input;
                    ++command_head;
                }
                else // command is bad
                {
                    command[1] = '\0';
                    if (echo_on) printf("\r\n");
                    echo_on = 0;
                }
            }
            if ( (!echo_on) && (command[0] == '/') && (command[1] == '0') )
            {
                echo_on = 1;
                printf_P(PSTR("%c%c"),command[0], command[1]);
            }
        }
        
        // finish echo of the command befor starting a reply (or the next part of reply)
        if ( command_done && (uart0_availableForWrite() == UART_TX0_BUFFER_SIZE) )
        {
            
            // is my command /0/id?
            if ( (command[0] == '/') &&  (command[1] == '0') && (command[2] == '/') && (command[3] == 'i') &&  (command[4] == 'd') && (command[5] == '?') )
            {
                if ( (command_done == 1) )
                {
                    printf_P(PSTR("{ \"Hello\": { \"id\": \"RPUno\", \"d"));
                    command_done = 2;
                }
                else if ( (command_done == 2) )
                {
                    printf_P(PSTR("ev\": \"solar power and ATmega32"));
                    command_done = 3;
                }
                else if ( (command_done == 3) )
                {
                    printf_P(PSTR("8p\" } }\r\n"));
                    command[1] = '\0';  // best to set address as null value
                    command_done = 0;
                    command_head =0;
                    echo_on = 0;
                }
                else 
                {
                    command[1] = '\0';  // best to set address as null value
                    command_done = 0;
                    command_head =0;
                    echo_on = 0;
                }
            }
            else
            {
                command[1] = '\0';  // best to set address as null value
                command_done = 0;
                command_head =0;
                echo_on = 0;
            }
         }
    }        
    return 0;
}
