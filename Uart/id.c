/*
id is part of RPUno, it adds the identify command to an RPU_BUS command line interface, 
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
#include <stdio.h>
#include <stdlib.h> 
#include "../lib/parse.h"
#include "../lib/rpu_mgr.h"
#include "id.h"

void Id(char name[])
{ 
    // /id? 
    if ( (command_done == 10) && (arg_count == 0) )
    {
        printf_P(PSTR("{\"id\":{"));
        command_done = 11;
    }
    // /id? name 
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("name")) == 0) ) 
    {
        printf_P(PSTR("{\"id\":{"));
        command_done = 11;
    }
    // /id? desc
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("desc")) == 0) )
    {
        printf_P(PSTR("{\"id\":{" ));
        command_done = 12;
    }
    // /id? avr-gcc
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("avr-gcc")) == 0) )
    {
        printf_P(PSTR("{\"id\":{"));
        command_done = 14;
    }
    else if ( command_done == 11 )
    {
        printf_P(PSTR("\"name\":\"%s\"" ),name);
        if (arg_count == 1) 
        { 
            command_done = 15;  
        }
        else 
        { 
            printf_P(PSTR("," ));
            command_done = 12; 
        }
    }
    else if ( command_done == 12 )
    {
        printf_P(PSTR("\"desc\":\"RPUno (14140^7) " ));
        command_done = 13;
    }
    else if ( command_done == 13 )
    {
        printf_P(PSTR("Board /w atmega328p\""));
        if (arg_count == 1) 
        { 
            command_done = 15; 
        }
        else 
        { 
            printf_P(PSTR("," ));
            command_done = 14; 
        }
    }
    else if ( command_done == 14 )
    {
        printf_P(PSTR("\"avr-gcc\":\"%d.%d\""),__GNUC__,__GNUC_MINOR__);
        command_done = 15; 
    }
    else if ( command_done == 15 )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"idBadArg_%s\"}\r\n"),arg[0]);
        initCommandBuffer();
    }
}

