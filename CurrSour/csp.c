/*
csp is part of CurrSour, a serial command interface to control current sources for ICP.
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
#include "csp.h"

#define SERIAL_PRINT_DELAY_MILSEC 10000
static unsigned long serial_print_started_at;

// show the current source pin number used
void echo_csp_pin_in_json_rply(uint8_t csp)
{
    switch(csp) 
    {
        case 0 : 
            printf_P(PSTR("ICP1")); // CSP0
            break;
    }
}

// pinMode( arg[0], arg[1] )
void CurrSourICP(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"CSParg0NaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 0..0
        uint8_t csp = atoi(arg[0]);
        if ( ( csp < 0) || (csp > 0) )
        {
            printf_P(PSTR("{\"err\":\"CSParg0OutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // also arg[1] is not ('INPUT' or 'OUTPUT')
        if ( !( (strcmp_P( arg[1], PSTR("ON")) == 0) || (strcmp_P( arg[1], PSTR("OFF")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"CSParg1NaMode\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        printf_P(PSTR("{\""));
        if (strcmp_P( arg[1], PSTR("ON")) == 0 ) 
        {
            pinMode(csp_pin_map[csp].pin, OUTPUT);
            digitalWrite(csp_pin_map[csp].pin, HIGH);
        }
        else
        {
            pinMode(csp_pin_map[csp].pin, OUTPUT);
            digitalWrite(csp_pin_map[csp].pin, LOW);
        }
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t csp = atoi(arg[0]);
        echo_csp_pin_in_json_rply( csp );
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
        printf_P(PSTR("{\"err\":\"CSPCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}