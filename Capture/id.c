/*
id is part of Capture, it uses a command line and optinal arguemnt to provide device info, 
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
#include "id.h"

void Id(void)
{ 
    // /id? name 
    // /id?
    if ( (command_done == 10) && ( (arg_count == 0) || ( (arg_count == 1) && (strcmp_P( arg[0], PSTR("name")) == 0) ) ) )
    {
        printf_P(PSTR("{\"id\":{\"name\":\"Capture\"}}\r\n"));
        initCommandBuffer();
    }
    // /id? desc
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("desc")) == 0) )
    {
        printf_P(PSTR("{\"id\":{\"desc\":\"RPUno Board /w " ));
        command_done = 11;
    }
    else if ( (command_done == 11) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("desc")) == 0) )
    {
        printf_P(PSTR("ATmega328p and LT3652\"}}\r\n"));
        initCommandBuffer();
    }
    // /id? avr-gcc
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("avr-gcc")) == 0) )
    {
        printf_P(PSTR("{\"id\":{\"avr-gcc\":\"%d.%d\"}}\r\n"),__GNUC__,__GNUC_MINOR__);
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"idBadArg_%s\"}\r\n"),arg[0]);
        initCommandBuffer();
    }
}

