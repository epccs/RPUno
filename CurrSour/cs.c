/*
cs is part of CurrSour, a serial command interface to control current sources.
Copyright (C) 2018 Ronald Sutherland

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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "../lib/parse.h"
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "cs.h"

#define SERIAL_PRINT_DELAY_MILSEC 10000
static unsigned long serial_print_started_at;

// show the current source pin number used
void echo_cs_pin_in_json_rply(uint8_t cs)
{
    switch(cs) 
    {
        case 0 : 
            printf_P(PSTR("PD5")); // CS0
            break;
        case 1 :
            printf_P(PSTR("PD6")); // CS1
            break;
        case 2 :
            printf_P(PSTR("PD3")); // CS2
            break;
        case 3 :
            printf_P(PSTR("PD4")); // CS3
            break;
    }
}

// pinMode( arg[0], arg[1] )
void CurrSour(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"CSarg0NaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 0..3
        uint8_t cs = atoi(arg[0]);
        if ( ( cs < 0) || (cs > 3) )
        {
            printf_P(PSTR("{\"err\":\"CSarg0OutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // also arg[1] is not ('INPUT' or 'OUTPUT')
        if ( !( (strcmp_P( arg[1], PSTR("ON")) == 0) || (strcmp_P( arg[1], PSTR("OFF")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"CSarg1NaMode\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        printf_P(PSTR("{\""));
        if (strcmp_P( arg[1], PSTR("ON")) == 0 ) 
        {
            pinMode(cs_pin_map[cs].pin, OUTPUT);
            digitalWrite(cs_pin_map[cs].pin, HIGH);
        }
        else
        {
            pinMode(cs_pin_map[cs].pin, OUTPUT);
            digitalWrite(cs_pin_map[cs].pin, LOW);
        }
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t cs = atoi(arg[0]);
        echo_cs_pin_in_json_rply( cs );
        printf_P(PSTR("\":\""));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf( arg[1] );
        printf_P(PSTR("\"}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"CSCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}