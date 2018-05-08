/*
digital is part of Digital, it is a serial command interface to some digital Wiring like functions, 
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
#include <stdbool.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/twi.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "power.h"

// Use integers at 10 uAmps per count, e.g. 7000 is 70mA
#define DISCHRG_I_10uA_CNT_AFTER_HAULT 7000L

#define SERIAL_PRINT_DELAY_MILSEC 10000
static unsigned long serial_print_started_at;

#define HAULT_DELAY_MILSEC 30000UL
static unsigned long hault_started_at;

#define WEARLEVEL_CHECK_MILSEC 1000UL
static unsigned long wearlevel_check_started_at;

#define WEARLEVEL_CHECKS 5
static uint8_t stable_means_notwearleveling;
static int last_wearlevel;
uint8_t stable_power_needed;

void VinPwr(void)
{
    if ( (command_done == 10) )
    {
        // check arg[0] is not ('UP' or 'DOWN')
        if ( !( (strcmp_P( arg[0], PSTR("UP")) == 0) || (strcmp_P( arg[0], PSTR("DOWN")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"VinNaMode\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        if (strcmp_P( arg[0], PSTR("UP")) == 0 ) 
        {
            digitalWrite(SHLD_VIN_EN,HIGH);
            pinMode(SHLD_VIN_EN, OUTPUT);
            printf_P(PSTR("{\"VIN\":\"UP\"}\r\n"));
            initCommandBuffer();
        }
        else
        {
            // should alt power turn off?
            // digitalWrite(ALT_EN,LOW);
            // pinMode(ALT_EN, OUTPUT);
            printf_P(PSTR("{\"VIN\":\"ALT_DWN?\"}\r\n"));
            command_done = 11;
        }
    }
    else if ( (command_done == 11) )
    {  
        if( set_Rpu_shutdown() ) // ../lib/rpu_mgr.h 
        {
            printf_P(PSTR("{\"VIN\":\"I2C_HAULT\"}\r\n"));
            stable_power_needed = 1;
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        // check that current is less than the hault threshold
        // adc_output = analogRead(PWR_I) is a value from 0 to 1023
        // Discharge_10uA_Cnt =  adc_output * 100000*(5.0/1024.0)/(0.068*50.0) 
        // which is to say each ADC count has a value of about 144 * 10uA  or 1.44mA
        long Discharge_10uA_Cnt = analogRead(PWR_I) * 144;
        if (Discharge_10uA_Cnt < DISCHRG_I_10uA_CNT_AFTER_HAULT)
        {
            // If 70mA is the hault value, then the ADC*144 needs to be less
            printf_P(PSTR("{\"VIN\":\"AT_HAULT_CURR\"}\r\n"));
            hault_started_at = millis();
            command_done = 13;
        }
    }
    else if ( (command_done == 13) )
    {
        // wait for some time befor checking for ware leveling
        unsigned long kRuntime= millis() - hault_started_at;
        if ((kRuntime) > ((unsigned long)HAULT_DELAY_MILSEC))
        {
            printf_P(PSTR("{\"VIN\":\"DELAY\"}\r\n"));
            last_wearlevel = 0;
            stable_means_notwearleveling = 0;
            command_done = 14; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
    else if ( (command_done == 14) )
    {
        // check that discharge current is less than the hault threshold and has been stable for a few readings
        if (! last_wearlevel)
        {
            last_wearlevel = analogRead(PWR_I);
            wearlevel_check_started_at = millis();
        }
        else
        {
            unsigned long kRuntime= millis() - wearlevel_check_started_at;
            if ((kRuntime) > ((unsigned long)WEARLEVEL_CHECK_MILSEC))
            {
                int new_wearlevel = analogRead(PWR_I);
                //printf_P(PSTR("{\"VIN\":\"adc %d\"}\r\n"),new_wearlevel);
                if ( (new_wearlevel < (last_wearlevel + 2) ) && (new_wearlevel > (last_wearlevel - 2) ) )
                {
                    stable_means_notwearleveling +=1;
                    if (stable_means_notwearleveling > WEARLEVEL_CHECKS)
                    {
                        printf_P(PSTR("{\"VIN\":\"WEARLEVELINGCLEAR\"}\r\n"));
                        command_done = 15;
                    }
                }
                else
                {
                    last_wearlevel = new_wearlevel;
                    stable_means_notwearleveling = 0;
                }
                wearlevel_check_started_at = millis();
            }
        }
    }
    else if ( (command_done == 15) )
    {
        stable_power_needed = 0;
        digitalWrite(SHLD_VIN_EN,LOW);
        pinMode(SHLD_VIN_EN, OUTPUT);
        uint8_t shutdown_detected = detect_Rpu_shutdown(); 
        if (shutdown_detected)
        {
            printf_P(PSTR("{\"VIN\":\"DOWN\"}\r\n"));
        }
        else
        {
            printf_P(PSTR("{\"VIN\":\"DOWN_I2CBAD\"}\r\n"));
        }
        // should ALT power turned on
        // digitalWrite(ALT_EN,HIGH);
        // pinMode(ALT_EN, OUTPUT);
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"VinCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

void ShutdownDetected(void)
{
    if ( (command_done == 10) )
    {
        // zero arguments
        uint8_t shutdown_detected = detect_Rpu_shutdown(); 
        if (shutdown_detected)
        {
            printf_P(PSTR("{\"SHUTDOWN\":\"DETECTED\"}\r\n"));
        }
        else
        {
            printf_P(PSTR("{\"SHUTDOWN\":\"CLEAR\"}\r\n"));
        }
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"FTCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}
