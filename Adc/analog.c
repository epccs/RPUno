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
#include <avr/eeprom.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pins_board.h"
#include "analog.h"
#include "references.h"

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
        // laod reference calibration or show an error if they are not in eeprom
        if ( ! LoadAnalogRefFromEEPROM() )
        {
            printf_P(PSTR("{\"err\":\"AdcRefNotInEeprom\"}\r\n"));
            initCommandBuffer();
            return;
        }

        // print in steps otherwise the serial buffer will fill and block the program from running
        serial_print_started_at = millis();
        printf_P(PSTR("{"));
        adc_arg_index= 0;
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { // use the channel as an index in the JSON reply
        uint8_t arg_indx_channel =atoi(arg[adc_arg_index]);
        
        if (arg_indx_channel == 0) //ADC0
        {
            printf_P(PSTR("\"ADC%s\":"),arg[adc_arg_index]);
        }

        if (arg_indx_channel == 1) //ADC1
        {
            printf_P(PSTR("\"ADC%s\":"),arg[adc_arg_index]);
        }
        
        if (arg_indx_channel == CHRG_I) //ADC2
        {
            printf_P(PSTR("\"CHRG_A\":"));
        }

        if (arg_indx_channel == DISCHRG_I) //ADC3
        {
            printf_P(PSTR("\"DISCHRG_A\":"));
        }

        if (arg_indx_channel == 4)
        {
            printf_P(PSTR("\"ADC4\":"));
        }

        if (arg_indx_channel == 5)
        {
            printf_P(PSTR("\"ADC5\":"));
        }

        if (arg_indx_channel == PV_V) //ADC6
        {
            printf_P(PSTR("\"PV_V\":"));
        }
        
        if (arg_indx_channel == PWR_V) //ADC7
        {
            printf_P(PSTR("\"PWR_V\":"));
        }
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t arg_indx_channel =atoi(arg[adc_arg_index]);

        // There are values from 0 to 1023 for 1024 slots where each reperesents 1/1024 of the reference. Last slot has issues
        // https://forum.arduino.cc/index.php?topic=303189.0 
        if (arg_indx_channel == 0)
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(0)*(ref_extern_avcc_uV/1.0E6)/1024.0));
        }

        if (arg_indx_channel == 1)
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(0)*(ref_extern_avcc_uV/1.0E6)/1024.0));
        }

        if (arg_indx_channel == CHRG_I) // RPUno has ADC2 connected to high side current sense to measure battery charging.
        {
            printf_P(PSTR("\"%1.3f\""),(analogRead(CHRG_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0)));
        }

        if (arg_indx_channel == DISCHRG_I) // RPUno has ADC3 connected to high side current sense to measure battery discharg.
        {
            printf_P(PSTR("\"%1.3f\""),(analogRead(DISCHRG_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0)));
        }

        if (arg_indx_channel == 4) // RPUno has ADC4 is used for I2C SDA function
        {
            printf_P(PSTR("\"SDA\""));
        }

        if (arg_indx_channel == 5) // RPUno has ADC5 is used for I2C SCL function
        {
            printf_P(PSTR("\"SCL\""));
        }

        if (arg_indx_channel == PV_V) // RPUno has ADC6 connected to a voltage divider from the solar input.
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(PV_V)*((ref_extern_avcc_uV/1.0E6)/1024.0)*(532.0/100.0)));
        }

        if (arg_indx_channel == PWR_V) // RPUno has ADC7 connected a voltage divider from the battery (PWR).
        {
            printf_P(PSTR("\"%1.2f\""),(analogRead(PWR_V)*((ref_extern_avcc_uV/1.0E6)/1024.0)*(3.0/1.0)));
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
