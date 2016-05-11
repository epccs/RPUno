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
#include "count.h"

/* event count of ICP1 
    no artuments needed*/
void Count(void)
{
    if ( (command_done == 10) )
    {
        // the count must not change while reading chCount
        ATOMIC_BLOCK ( ATOMIC_RESTORESTATE )
        {
            // printf conversion specification for an unsigned long is %lu
            printf_P(PSTR("{\"icp1\":{\"count\":\"%lu\"}}\r\n"),icp1_event_count);
        }
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"CntCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

