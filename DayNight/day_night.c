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

// Use a red LED to measure the light on ADC2
#define RED_LED_SENSOR ADC2
// analog reading of 20*5.0/1024.0 is about 0.1V
// analog reading of 30*5.0/1024.0 is about 0.15V
#define MORNING_THRESHOLD 30
#define STARTUP_DELAY 1000UL
#define EVENING_THRESHOLD 20
// 900,000 millis is 15 min
#define EVENING_DEBOUCE 9000UL
#define MORNING_DEBOUCE 9000UL
#define DAYNIGHT_TO_LONG 72000000UL
// 72 Meg millis() is 20hr, which is past the longest day or night so somthing has went wrong.
/* note the UL is a way to tell the compiler that a numerical literal is of type unsigned long */

#if MORNING_THRESHOLD > 0x3FF
#   error ADC maxim size is 0x3FF 
#endif

#if EVENING_THRESHOLD > 0x3FF
#   error ADC maxim size is 0x3FF 
#endif

#if MORNING_THRESHOLD - EVENING_THRESHOLD < 0x09
#   error ADC hysteresis of 9 should be allowed
#endif


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

#define SERIAL_PRINT_DELAY_MILSEC 60000UL
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

void Day(void)
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
            printf_P(PSTR("\"START\""));
        }

        if (state == DAYNIGHT_DAY_STATE) 
        {
            printf_P(PSTR("\"DAY\""));
        }

        if (state == DAYNIGHT_EVENING_DEBOUNCE_STATE) 
        {
            printf_P(PSTR("\"EVENING\""));
        }

       if (state == DAYNIGHT_NIGHTWORK_STATE) 
        {
            printf_P(PSTR("\"NIGHTWORK\""));
        }

        if (state == DAYNIGHT_NIGHT_STATE) 
        {
            printf_P(PSTR("\"NIGHT\""));
        }

        if (state == DAYNIGHT_MORNING_DEBOUNCE_STATE) 
        {
            printf_P(PSTR("\"MORNING\""));
        }

        if (state == DAYNIGHT_DAYWORK_STATE) 
        {
            printf_P(PSTR("\"DAYWORK\""));
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
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
        {
            command_done = 10; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
}

/* check for daytime state durring program looping  
    dayState 
    0 = default at startup, if above Evening Threshold set day, else set night.
    1 = day: wait for evening threshold, set for evening debounce.
    2 = evening_debounce: wait for EVENING_DEBOUCE to finish, then set night, however if debouce fails set back to day.
    3 = night: wait for morning threshold, set morning_debounce.
    4 = morning_debounce: wait for MORNING_DEBOUCE to finish, then set irrigation, however if debouce fails set back to night.
    5 = work: do some work.
    6 = fail: fail state.
*/
void CheckDayLight(void) 
{
    int Morning_Threshold = MORNING_THRESHOLD;
    int Evening_Threshold = EVENING_THRESHOLD;
  
    if(dayState == DAYNIGHT_START_STATE) 
    { 
        unsigned long kRuntime= millis() - dayTmrStarted;
        if ((kRuntime) > ((unsigned long)STARTUP_DELAY)) 
        {
            if(analogRead(RED_LED_SENSOR) > Evening_Threshold ) 
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
        if (analogRead(RED_LED_SENSOR) < Evening_Threshold ) 
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
        if (analogRead(RED_LED_SENSOR) < Evening_Threshold ) 
        {
            unsigned long kRuntime= millis() - dayTmrStarted;
            if ((kRuntime) > ((unsigned long)EVENING_DEBOUCE)) 
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
    { 
        //do the work (if it got registered), e.g. load night light settings at the start of a night
        if (dayState_atNightWork != NULL) dayState_atNightWork();
        dayState = DAYNIGHT_NIGHT_STATE;
        return;
    }

    if(dayState == DAYNIGHT_NIGHT_STATE) 
    { //night
        if (analogRead(RED_LED_SENSOR) > Morning_Threshold ) 
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
        if (analogRead(RED_LED_SENSOR) > Morning_Threshold ) 
        {
            unsigned long kRuntime= millis() - dayTmrStarted;
            if ((kRuntime) > ((unsigned long)MORNING_DEBOUCE)) 
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
    { 
        //do the work (if it got registered),, e.g. load irrigation settings at the start of a day
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
