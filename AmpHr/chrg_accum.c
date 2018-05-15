/*
chrg_accum is part of AmpHr, it is to track how much current is used.
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
#include "chrg_accum.h"

static unsigned long chrgTmrStartOfAccum[ADC_CHANNELS];
static unsigned long chrgTmrStarted[ADC_CHANNELS];
#define CHRG_ACCUMULATION_DELAY_MILSEC 20UL

static unsigned long chrg_accum[ADC_CHANNELS];
static unsigned long chrg_accum_fine[ADC_CHANNELS];

static unsigned long serial_print_started_at;

// scale accumulated adc*time to mAHr
static float adc_accum2mAHr[ADC_CHANNELS] = {
    [0] = ((1/1.0E6)/1024.0)/(100.0)/3.6, // 100 ohm resistor, e.g. for a 4 to 20mA sensor
    [1] = ((1/1.0E6)/1024.0)/(100.0)/3.6, // 100 ohm resistor, e.g. for a 4 to 20mA sensor
    [2] = ((1/1.0E6)/1024.0)/(100.0)/3.6, // 100 ohm resistor, e.g. for a 4 to 20mA sensor
    [3] = ((1/1.0E6)/1024.0)/(100.0)/3.6, // 100 ohm resistor, e.g. for a 4 to 20mA sensor
    [4] = 0, //used for SDA of I2C
    [5] = 0, //used for SCL of I2C
    [6] = ((1/1.0E6)/1024.0)/(0.068*50.0)/3.6, // PWR_I is 0.068 ohm sense resistor with gain of 50
    [7] = 0 // PWR_V
};

// Accumulated Charge in mAHr
float ChargeAccum(uint8_t channel)
{
    float temp = 0.0;
    if (channel < ADC_CHANNELS) 
    {
        temp = adc_accum2mAHr[channel] * ref_extern_avcc_uV * chrg_accum[channel];
    }
    return temp;
}

void Charge(unsigned long serial_print_delay_milsec)
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
        printf_P(PSTR("\"%1.2f\","),ChargeAccum(PWR_I) );
        command_done = 16;
    }
    else if ( (command_done == 16) )
    { 
        printf_P(PSTR("\"ACCUM_Sec\":"));
        command_done = 17;
    }
    else if ( (command_done == 17) )
    { 
        printf_P(PSTR("\"%1.2f\""),((chrgTmrStarted[PWR_I] - chrgTmrStartOfAccum[PWR_I])/1000.0));
        printf_P(PSTR("}\r\n"));
        command_done = 18;
    }
    else if ( (command_done == 18) ) 
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > (serial_print_delay_milsec))
        {
            command_done = 10; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
}

uint8_t ResetChargeAccum(void)
{  
    // command_done in one step so no need check for which step to work on

    // laod reference calibration or show an error if they are not in eeprom
    if ( ! LoadAnalogRefFromEEPROM() )
    {
        printf_P(PSTR("{\"err\":\"AdcRefNotInEeprom\"}\r\n"));
        initCommandBuffer();
        return 0;
    }
    if (!init_ChargAccumulation(PWR_I)) // ../AmpHr/chrg_accum.c
    {
        printf_P(PSTR("{\"err\":\"init_ChargAccum\"}\r\n"));
        initCommandBuffer();
        return 0;
    }
    serial_print_started_at = millis();
    printf_P(PSTR("{\"init_ChrgAccum\":\"OK\"}\r\n"));
    initCommandBuffer();
    return 1;
}

/* update charge accumulation
    charge accumulation is an integer value of the ADC*(millis()/1000) and will need scaled
*/
void CheckChrgAccumulation(uint8_t channel) 
{
    unsigned long kRuntime= millis() - chrgTmrStarted[channel];
    if ((kRuntime) > ((unsigned long)CHRG_ACCUMULATION_DELAY_MILSEC))
    {
        chrg_accum_fine[channel] += (analogRead(channel) * CHRG_ACCUMULATION_DELAY_MILSEC); 
        chrgTmrStarted[channel] += CHRG_ACCUMULATION_DELAY_MILSEC;
    }
    else
    {
        // check if fine accumulator has enough to add it to the Sec based accumulater
        if (chrg_accum_fine[channel] > 10000) 
        {
            chrg_accum[channel] +=10;
            chrg_accum_fine[channel] -= 10000;
        }
       else if (chrg_accum_fine[channel] > 1000) 
        {
            ++chrg_accum[channel];
            chrg_accum_fine[channel] -= 1000;
        }
    }
}

/* For when the charge accumulation values need to be zeroed
*/
uint8_t init_ChargAccumulation(uint8_t channel) 
{
    chrg_accum[channel] = 0;
    chrg_accum_fine[channel] = 0;
    chrgTmrStarted[channel] = millis();
    chrgTmrStartOfAccum[channel] = chrgTmrStarted[channel];
    
    // laod reference calibration from eeprom
    if ( ! LoadAnalogRefFromEEPROM() )
    {
        return 0;
    }
    
    return 1;
}