/*
alternat is a library that enables the Alternat power input.
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
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Adc/references.h"
#include "alternat.h"

#define ALT_LIMIT_DELAY_MILSEC 5000UL
static unsigned long alt_limit_started_at;

uint8_t alt_enable;
unsigned int alt_count; // full charge when this has a count


void EnableAlt(void)
{
    if ( (command_done == 10) )
    {
        alt_enable = !alt_enable; // Toggle
        printf_P(PSTR("{\"alt_en\":"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {
        if (alt_enable)
        {
            printf_P(PSTR("\"ON\""));
            alt_count = 0;
        }
        else
        {
            digitalWrite(ALT_EN,LOW); // checking is blocked when !alt_enable so make sure the pin is off
            printf_P(PSTR("\"OFF\""));
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
        initCommandBuffer();
    }
}

void AltCount(void)
{
    if ( (command_done == 10) )
    {
        printf_P(PSTR("{\"alt_count\":"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {
        printf_P(PSTR("\"%u\""), alt_count); 
        command_done = 12;
    }
    else if ( (command_done == 12) )
    { 
         printf_P(PSTR("}\r\n"));
        initCommandBuffer();
    }
    else
    {
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
                unsigned long kRuntime= millis() - alt_limit_started_at;
                if ((kRuntime) > ((unsigned long)ALT_LIMIT_DELAY_MILSEC))
                {
                    if (digitalRead(ALT_EN))
                    {
                        digitalWrite(ALT_EN,LOW);
                        alt_count += 1; // count the number of times the battery is at the charge limit
                    }
                }
            }
            else if (battery < 0.99*limit)
            {
                digitalWrite(ALT_EN,HIGH);
                alt_limit_started_at = millis();
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
