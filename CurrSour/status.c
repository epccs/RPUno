/*
status is part of CurrSour it uses a serial command interface.
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
#include "status.h"

uint8_t show_status;

// pinMode( arg[0] )
void ShowStatus(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a ('ON' or 'OFF')
        if ( !( (strcmp_P( arg[0], PSTR("ON")) == 0) || (strcmp_P( arg[0], PSTR("OFF")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"STATarg1NaMode\"}\r\n"));
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"SHOW\":\""));
        if (strcmp_P( arg[0], PSTR("ON")) == 0 ) 
        {
            show_status = 1;
        }
        else
        {
            show_status = 0;
        }
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {
        printf( arg[0] );
        printf_P(PSTR("\"}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"CSPCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}