/*
count is part of Capture, it returns the ICP1 event count, 
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
#include <stdio.h>
#include <stdlib.h> 
#include "../lib/parse.h"
#include "../lib/icp1.h"
#include "../lib/timers.h"
#include "count.h"

#define SERIAL_PRINT_DELAY_SEC 5
static unsigned long serial_print_started_at;

/* event count of ICP1 
    no artuments needed*/
void Count(void)
{
    if ( (command_done == 10) )
    {
        serial_print_started_at = millis();
        // the count must not change while reading chCount
        ATOMIC_BLOCK ( ATOMIC_RESTORESTATE )
        {
            // printf conversion specification for an unsigned long is %lu
            printf_P(PSTR("{\"icp1\":{\"count\":\"%lu\"}}\r\n"),icp1_event_count);
        }
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_SEC * 1000))
        {
            command_done = 10; /* This keeps looping the output forever (until a Rx char anyway) */
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"CntCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

