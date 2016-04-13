/*
Uart is a demonstration of stdio to UART redirect. 
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
#include <stdio.h>
#include "uart.h"

int main(void) {    

    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;
                
    char input;
    char command[5];
    uint8_t command_i =0;

    while(1) 
    {
        input = getchar();
        if ( (input == '\r') || (input == '\n') ) // pressing enter in picocom sends a \r
        {
            printf("\r\n");  //echo both a carrage return and new line 
            command_i = 0;
            if ( (command[0] == 'i') &&  (command[1] == 'd') && (command[2] == '?'))
            {
                puts("{ \"Hello\": { \"id\": \"RPUno\", ");
                puts("\"dev\": \"solar power atmega328\" } }");
                printf("\n"); 
            }
                
        }
        else
        {
            printf("%c", input);  //echo the input
            command[command_i] = input;
            if (command_i < 5 )
            {
                ++command_i;
            }
        }
    }        
    return 0;
}