/*
status is a library that allows a current source to be used to show status.
Copyright (C) 2019 Ronald Sutherland

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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