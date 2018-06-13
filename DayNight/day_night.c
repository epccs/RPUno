/*
day-night is part of DayNight, it is usd to track the day and night for control functions, 
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
#include "../lib/pins_board.h"
#include "day_night.h"

// analog reading of 5*5.0/1024.0 is about 0.024V
// analog reading of 10*5.0/1024.0 is about 0.049V
#define MORNING_THRESHOLD 10
#define STARTUP_DELAY 1000UL
#define EVENING_THRESHOLD 5
// 900,000 millis is 15 min
#define EVENING_DEBOUCE 900000UL
#define MORNING_DEBOUCE 900000UL
#define DAYNIGHT_TO_LONG 72000000UL
// 72 Meg millis() is 20hr, which is past the longest day or night so somthing has went wrong.
/* note the UL is a way to tell the compiler that a numerical literal is of type unsigned long */

#if MORNING_THRESHOLD > 0x3FF
#   error ADC maxim size is 0x3FF 
#endif

#if EVENING_THRESHOLD > 0x3FF
#   error ADC maxim size is 0x3FF 
#endif

#if MORNING_THRESHOLD - EVENING_THRESHOLD < 0x04
#   error ADC hysteresis of 4 should be allowed
#endif

int morning_threshold = MORNING_THRESHOLD;
int evening_threshold = EVENING_THRESHOLD;
unsigned long evening_debouce = (unsigned long)EVENING_DEBOUCE;
unsigned long morning_debouce = (unsigned long)MORNING_DEBOUCE;
uint8_t dayState = 0; 
unsigned long dayTmrStarted;

// used to initalize the PointerToWork functions in case they are not used.
void callback_default(void)
{
    return;
}

typedef void (*PointerToWork)(void);
static PointerToWork dayState_atDayWork = callback_default;
static PointerToWork dayState_atNightWork = callback_default;

static unsigned long serial_print_started_at;

/* register (i.e. save a referance to) a function callback that does somthing
 */
void Day_AttachWork( void (*function)(void)  )
{
    dayState_atDayWork = function;
}

void Night_AttachWork( void (*function)(void)  )
{
    dayState_atNightWork = function;
}

void Day(unsigned long serial_print_delay_milsec)
{
    if ( (command_done == 10) )
    {
        serial_print_started_at = millis();
        printf_P(PSTR("{\"day_state\":"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { 
        uint8_t state = DayState();

        if (state == DAYNIGHT_START_STATE) 
        {
            printf_P(PSTR("\"STRT\""));
        }

        if (state == DAYNIGHT_DAY_STATE) 
        {
            printf_P(PSTR("\"DAY\""));
        }

        if (state == DAYNIGHT_EVENING_DEBOUNCE_STATE) 
        {
            printf_P(PSTR("\"EVE\""));
        }

       if (state == DAYNIGHT_NIGHTWORK_STATE) 
        {
            printf_P(PSTR("\"NEVT\""));
        }

        if (state == DAYNIGHT_NIGHT_STATE) 
        {
            printf_P(PSTR("\"NGHT\""));
        }

        if (state == DAYNIGHT_MORNING_DEBOUNCE_STATE) 
        {
            printf_P(PSTR("\"MORN\""));
        }

        if (state == DAYNIGHT_DAYWORK_STATE) 
        {
            printf_P(PSTR("\"DEVT\""));
        }

        if (state == DAYNIGHT_FAIL_STATE) 
        {
            printf_P(PSTR("\"FAIL\""));
        }
        printf_P(PSTR("}\r\n"));
        command_done = 12;
    }
    else if ( (command_done == 12) ) 
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > (serial_print_delay_milsec))
        {
            command_done = 10; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
}

/* check for daytime state durring program looping  
    adc_ch_with_red_led_sensor: ADC channel
    dayState: range 0..7
    0 = default at startup, if above Evening Threshold set day, else set night.
    1 = day: wait for evening threshold, set for evening debounce.
    2 = evening_debounce: wait for debounce time, do night_work, however if debouce fails set back to day.
    3 = night_work: do night callback and set night.
    4 = night: wait for morning threshold, set morning_debounce.
    5 = morning_debounce: wait for debounce time, do day_work, however if debouce fails set back to night.
    6 = day_work: do day callback and set for day.
    7 = fail: fail state.
*/
void CheckDayLight(uint8_t adc_ch_with_red_led_sensor) 
{ 
    int sensor_val = analogRead(adc_ch_with_red_led_sensor);
    if(dayState == DAYNIGHT_START_STATE) 
    { 
        unsigned long kRuntime= millis() - dayTmrStarted;
        if ((kRuntime) > ((unsigned long)STARTUP_DELAY)) 
        {
            if(sensor_val > evening_threshold ) 
            {
                dayState = DAYNIGHT_DAY_STATE; 
                dayTmrStarted = millis();
            } 
            else 
            {
                dayState = DAYNIGHT_NIGHT_STATE;
                dayTmrStarted = millis();
            }
        }
        return;
    } 
  
    if(dayState == DAYNIGHT_DAY_STATE) 
    { //day
        if (sensor_val < evening_threshold ) 
        {
            dayState = DAYNIGHT_EVENING_DEBOUNCE_STATE;
            dayTmrStarted = millis();
        }
        unsigned long kRuntime= millis() - dayTmrStarted;
        if ((kRuntime) > ((unsigned long)DAYNIGHT_TO_LONG)) 
        {
            dayState = DAYNIGHT_FAIL_STATE;
            dayTmrStarted = millis();
        }
        return;
    }
  
    if(dayState == DAYNIGHT_EVENING_DEBOUNCE_STATE) 
    { //evening_debounce
        if (sensor_val < evening_threshold ) 
        {
            unsigned long kRuntime= millis() - dayTmrStarted;
            if ((kRuntime) > (evening_debouce)) 
            {
                dayState = DAYNIGHT_NIGHTWORK_STATE;
                dayTmrStarted = millis();
            } 
        } 
        else 
        {
            dayState = DAYNIGHT_DAY_STATE;
            dayTmrStarted = millis();
        }
        return;
    }

    if(dayState == DAYNIGHT_NIGHTWORK_STATE) 
    { //do the night work callback, e.g. load night light settings at the start of a night
        if (dayState_atNightWork != NULL) dayState_atNightWork();
        dayState = DAYNIGHT_NIGHT_STATE;
        return;
    }

    if(dayState == DAYNIGHT_NIGHT_STATE) 
    { //night
        if (sensor_val > morning_threshold ) 
        {
            dayState = DAYNIGHT_MORNING_DEBOUNCE_STATE;
            dayTmrStarted = millis();
        }
        unsigned long kRuntime= millis() - dayTmrStarted;
        if ((kRuntime) > ((unsigned long)DAYNIGHT_TO_LONG)) 
        {
            dayState = DAYNIGHT_FAIL_STATE;
            dayTmrStarted = millis();
        }
        return;
    }

    if(dayState == DAYNIGHT_MORNING_DEBOUNCE_STATE) 
    { //morning_debounce
        if (sensor_val > morning_threshold ) 
        {
            unsigned long kRuntime= millis() - dayTmrStarted;
            if ((kRuntime) > (morning_debouce)) 
            {
                dayState = DAYNIGHT_DAYWORK_STATE;
            }
        }
        else 
        {
            dayState = DAYNIGHT_NIGHT_STATE;
        }
        return;
    }

    if(dayState == DAYNIGHT_DAYWORK_STATE) 
    { //do the day work callback, e.g. load irrigation settings at the start of a day
        if (dayState_atDayWork != NULL) dayState_atDayWork();
        dayState = DAYNIGHT_DAY_STATE;
        return;
    }

    //index out of bounds? 
    if(dayState > DAYNIGHT_FAIL_STATE) 
    { 
        dayState = DAYNIGHT_FAIL_STATE;
    }
}

uint8_t DayState(void) 
{
    return dayState;
}
