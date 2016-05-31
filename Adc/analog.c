/*
analog is part of Adc, it returns Analog Conversions for channels which are provided in parse arguments, 
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
#include <ctype.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "analog.h"

#define SERIAL_PRINT_DELAY_MILSEC 60000
static unsigned long serial_print_started_at;

static uint8_t adc_arg_index;

/* return adc values */
void Analog(void)
{
    if ( (command_done == 10) )
    {
        // check that arguments are digit in the range 0..7
        for (adc_arg_index=0; adc_arg_index < arg_count; adc_arg_index++) 
        {
            if ( ( !( isdigit(arg[adc_arg_index][0]) ) ) || (atoi(arg[adc_arg_index]) < 0) || (atoi(arg[adc_arg_index]) > ADC_CHANNELS) )
            {
                printf_P(PSTR("{\"err\":\"AdcChOutOfRng\"}\r\n"));
                initCommandBuffer();
                return;
            }
        }
        // print in steps otherwise the serial buffer will fill and block the program from running
        serial_print_started_at = millis();
        printf_P(PSTR("{"));
        adc_arg_index= 0;
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { // use the channel as an index in the JSON reply
        if ( (adc_arg_index) == 0) 
        {
            printf_P(PSTR("\"ADC%s\":"),arg[adc_arg_index]);
            command_done = 12;
        }
        else
        {
            uint8_t arg_indx_channel =atoi(arg[adc_arg_index]);
            if (arg_indx_channel < 6)
            {
                printf_P(PSTR("\"ADC%s\":"),arg[adc_arg_index]);
            }
            if (arg_indx_channel == 6)
            {
                printf_P(PSTR("\"PV_IN\":"));
            }
            if (arg_indx_channel == 7)
            {
                printf_P(PSTR("\"PWR\":"));
            }
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        uint8_t arg_indx_channel =atoi(arg[adc_arg_index]);
        
        if (arg_indx_channel < 6)
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(arg_indx_channel)*5.0/1023.0));
        }
        
        // RPUno has a 432k and 100k voltage divider from the solar input. The PV goes through a 432k  to ADC6 and a 100k to ground.
        if (arg_indx_channel == 6) 
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(arg_indx_channel)*(5.0/1023.0)*(532.0/100.0)));
        }

        // RPUno has a 100 and 200k voltage divider from the battery(PWR). The PWR goes through a 100k  to ADC7 and a 200k to ground.
        if (arg_indx_channel == 7) 
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(arg_indx_channel)*(5.0/1023.0)*(3.0/2.0)));
        }

        if ( (adc_arg_index+1) >= arg_count) 
        {
            printf_P(PSTR("}\r\n"));
            // initCommandBuffer(); /* This stops the output after one loop*/
            command_done = 13;
        }
        else
        {
            printf_P(PSTR(","));
            adc_arg_index++;
            command_done = 11;
        }
    }
    else if ( (command_done == 13) ) 
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
        {
            command_done = 10; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"AdcCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

