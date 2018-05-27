/*
alternat is part of Alternat. Enable Aux input.
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
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Adc/references.h"
#include "alternat.h"

#define SERIAL_PRINT_DELAY_MILSEC 5000UL
#define ALT_ENABLE_DELAY_MILSEC 22500UL
static unsigned long serial_print_started_at;
static unsigned long alt_tracking_started_at;

uint8_t alt_enable;

void EnableAlt(void)
{
    if ( (command_done == 10) )
    {
        alt_enable = !alt_enable; // Toggle
        alt_tracking_started_at = millis();
        serial_print_started_at = millis();
        printf_P(PSTR("{\"alt_en\":"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {
        if (alt_enable)
        {
            printf_P(PSTR("{\"ON\""));
        }
        else
        {
            digitalWrite(ALT_EN,LOW); // checking is blocked when !alt_enable so make sure the pin is off
            printf_P(PSTR("{\"OFF\""));
        }
        command_done = 12;
    }
    else if ( (command_done == 12) )
    { 
         printf_P(PSTR("}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"EnAltCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

/* mux_ch - adc mux channel to use e.g. not digital
     scale - each channel may need to be scalled, use 1.0 if lacking voltage divider 
     limit - turn off the alternat power input */
void check_if_alt_should_be_on(uint8_t mux_ch, float scale, float limit)
{
    if ( ref_loaded )
    {
        if (alt_enable)
        {
            float battery = analogRead(mux_ch)*((ref_extern_avcc_uV/1.0E6)/1024.0)*scale;
            if (battery >= limit)
            {
                digitalWrite(ALT_EN,LOW);
            }
            else if (battery < 0.99*limit)
            {
                digitalWrite(ALT_EN,HIGH);
            }
        }
        else 
        {
            digitalWrite(ALT_EN,LOW);
        }
    }
    else
    {
        LoadAnalogRefFromEEPROM();
    }
}
