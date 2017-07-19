/*
power_storage is part of AmpHr, it is usd to track how much power is saved and used, 
Copyright (C) 2017 Ronald Sutherland

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
#include "../Adc/references.h"
#include "power_storage.h"

// RPUno has an input for 36 cell PV which is feed into a LT3652 charge controler (CC).
// The CC is connected to a node named PWR on the board and is connected 
// through a high side current sense to the battery, it is used to measure charging and discharging
// ADC channel 3 (DISCHRG_I) is converted to current with analogRead(DISCHRG_I)*(5.0/1024.0)/(0.068*50.0))
// ADC channel 2 (CHRG_I) is converted to current with analogRead(CHRG_I)*(5.0/1024.0)/(0.068*50.0))
// the fine accumulators are for the adc reading * millis counts (e.g.  it is in mSec and needs scaled with above)

static unsigned long chrgTmrStartOfAccum;
static unsigned long chrgTmrStarted;
#define CHRG_ACCUMULATION_DELAY_MILSEC 20UL

static unsigned long chrg_accum;
static unsigned long chrg_accum_fine;
static unsigned long dischrg_accum;
static unsigned long dischrg_accum_fine;
static long remaining_accum;

#define SERIAL_PRINT_DELAY_MILSEC 60000UL
static unsigned long serial_print_started_at;

// Accumulated Charge in mAHr
float ChargeAccum(void)
{
    return chrg_accum * ((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0)/3.6;
}

// Accumulated Discharge in mAHr
float DischargeAccum(void)
{
    return dischrg_accum * ((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0)/3.6;
}

// Remaining charge in mAHr from the last init (e.g. the previous day)
float RemainingAccum(void)
{
    return remaining_accum * ((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0)/3.6;
}

void Charge(void)
{  
    if ( (command_done == 10) )
    {
        // laod reference calibration or show an error if they are not in eeprom
        if ( ! LoadAnalogRefFromEEPROM() )
        {
            printf_P(PSTR("{\"err\":\"AdcRefNotInEeprom\"}\r\n"));
            initCommandBuffer();
            return;
        }
        
        serial_print_started_at = millis();
        printf_P(PSTR("{\"CHRG_mAHr\":"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { 
        printf_P(PSTR("\"%1.2f\","),ChargeAccum() );
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("\"DCHRG_mAHr\":"));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    { 
        printf_P(PSTR("\"%1.2f\","),DischargeAccum() );
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        printf_P(PSTR("\"RMNG_mAHr\":"));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    { 
        printf_P(PSTR("\"%1.2f\","),RemainingAccum() );
        command_done = 16;
    }
    else if ( (command_done == 16) )
    { 
        printf_P(PSTR("\"ACCUM_Sec\":"));
        command_done = 17;
    }
    else if ( (command_done == 17) )
    { 
        printf_P(PSTR("\"%1.2f\""),((chrgTmrStarted - chrgTmrStartOfAccum)/1000.0));
        printf_P(PSTR("}\r\n"));
        command_done = 18;
    }
    else if ( (command_done == 18) ) 
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
        {
            command_done = 10; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
}

/* check power accumulation
    charge accumulation is an integer value of the ADC*(millis()/1000) and will need scaled
*/
void CheckChrgAccumulation(void) 
{
    unsigned long kRuntime= millis() - chrgTmrStarted;
    if ((kRuntime) > ((unsigned long)CHRG_ACCUMULATION_DELAY_MILSEC))
    {
        chrg_accum_fine += (analogRead(CHRG_I) * CHRG_ACCUMULATION_DELAY_MILSEC); 
        dischrg_accum_fine += (analogRead(DISCHRG_I) * CHRG_ACCUMULATION_DELAY_MILSEC);
        chrgTmrStarted += CHRG_ACCUMULATION_DELAY_MILSEC;
    }
    else
    {
        // keeps the work load low for each loop
        if (chrg_accum_fine > 1000) 
        {
            ++chrg_accum;
            chrg_accum_fine -= 1000;
        }
        else if (dischrg_accum_fine > 1000) 
        {
            ++dischrg_accum;
            dischrg_accum_fine -= 1000;
        }
    }
}

/* The charge and discharge accumulation values need to be zeroed at the start of the day
*/
uint8_t init_ChargAccumulation(void) 
{
    remaining_accum = chrg_accum - dischrg_accum;
    chrg_accum = 0;
    chrg_accum_fine = 0;
    dischrg_accum = 0;
    dischrg_accum_fine = 0;
    chrgTmrStarted = millis();
    chrgTmrStartOfAccum = chrgTmrStarted;
    
    // laod reference calibration from eeprom
    if ( ! LoadAnalogRefFromEEPROM() )
    {
        return 0;
    }
    
    return 1;
}