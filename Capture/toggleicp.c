/*
toggleicp is part of Capture, it uses a command line arguemnt to Toggle the CS_ICP1_EN a number of times to test the ICP1 input
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
#include <stdio.h>
#include <stdlib.h> 
#include "../lib/parse.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "toggleicp.h"

int toggle_count;

void ToggleICP(void)
{
    if (arg_count == 2)
    {
        
        if ( (command_done == 10) )
        {
            if ( (atoi(arg[1]) >=0) && (atoi(arg[1]) <= 255) )
            {
                if (strcmp_P( arg[0], PSTR("icp1")) == 0)
                {
                    toggle_count = atoi(arg[1]);
                    pinMode(CS_ICP1_EN,OUTPUT);
                    digitalWrite(CS_ICP1_EN,LOW);
                    printf_P(PSTR("{\"icp1\":{\"toggle_CS_ICP1_EN\":\"%d\"}}\r\n"),toggle_count);
                    command_done = 11;
                }
            }
            else
            {
                printf_P(PSTR("{\"err\":\"toggleicpRange_%d\"}\r\n"),toggle_count);
                initCommandBuffer();
            }
        }
        else if ( (command_done == 11) )
        {
            if (toggle_count) 
            {
                digitalToggle(CS_ICP1_EN);
                --toggle_count;
            }
            else
            {
                digitalWrite(CS_ICP1_EN,LOW);
                initCommandBuffer();
            }
        }
        else
        {
           initCommandBuffer(); 
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"toggleicpNeed2Arg\"}\r\n"));
        initCommandBuffer();
    }
}

