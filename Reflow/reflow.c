/*
reflow is part of Reflow
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
#include <math.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Eeprom/ee.h"
#include "../Adc/references.h"
#include "reflow.h"

#define REFLOW_ZONE_DELAY_MILSEC 2000UL
static unsigned long reflow_zone_started_at;

//SSR is a Solid State Relay
#define SSR CS3_EN
#define BUZZER DIO15
#define REFLOW_PWM_EE_START 42


static uint16_t eeprom_offset;
static uint8_t pwm;
static uint8_t last_pwm;

/* if the UART is found to have char avvailable turn everything off*/
void initReflow(void)
{
    pinMode(SSR, OUTPUT);
    digitalWrite(SSR, LOW);
    
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
}


/* run reflow profile */
void Reflow(void)
{

    if ( (command_done == 10) )
    {
        eeprom_offset = REFLOW_PWM_EE_START;
        pwm =0;
        last_pwm =0;
        reflow_zone_started_at = millis();
        
        pinMode(SSR, OUTPUT);
        digitalWrite(SSR, LOW);
        
        pinMode(BUZZER, OUTPUT);
        digitalWrite(BUZZER, LOW);
        
        // laod reference calibration or show an error if they are not in eeprom
        if ( ! LoadAnalogRefFromEEPROM() )
        {
            printf_P(PSTR("{\"err\":\"AdcRefNotInEeprom\"}\r\n"));
            initCommandBuffer();
            return;
        }

        command_done = 11;
    }
    
    else if ( (command_done == 11) )
    { 
        pwm = eeprom_read_byte( (uint8_t *) (eeprom_offset) ); 
        printf_P(PSTR("{\"millis\":\"%lu\","),reflow_zone_started_at);
        
        // Turn on the SSR if pwm is between 1 thru 254 (255 is used for the buzzer)
        if ( (pwm > 0) && (pwm < 255) )
        {
            digitalWrite(SSR, HIGH);
        }
        if (pwm == 255)
        {
            digitalWrite(BUZZER, HIGH);
        }
        command_done = 12;
    }

    else if ( (command_done == 12) )
    {
        unsigned long now = millis();
        unsigned long kRuntime= now - reflow_zone_started_at;
        if (  (kRuntime) > ( (unsigned long)( 0.5 + (pwm/255.0)*REFLOW_ZONE_DELAY_MILSEC) )  )
        {
            digitalWrite(BUZZER, LOW);
            digitalWrite(SSR, LOW);
            printf_P(PSTR("\"pwm\":\"%u\","),pwm);
            command_done = 13;
        }
        else
        {
            command_done = 12;
        }
    }

    else if ( (command_done == 13) )
    {
        // analog channel 0 may be connected to the Fluke 80TK Thermocouple Module set in deg F per mV
        float Adc0_mV = analogRead(0)*((ref_intern_1v1_uV/1.0E3)/1024.0);
        //printf_P(PSTR("\"deg_c\":\"%1.2f\""),(Adc0_mV -32.0) * (5.0/9.0) );
        printf_P(PSTR("\"ADC0_mV\":\"%1.2f\""),(Adc0_mV) );
        command_done = 24;
    }
    
    else if ( (command_done == 24) )
    {
        printf_P(PSTR("}\r\n"));
        command_done = 25;
    }
    
    else if ( (command_done == 25) )
    { 
        unsigned long now = millis();
        unsigned long kRuntime= now - reflow_zone_started_at;
        if ((kRuntime) > ((unsigned long)REFLOW_ZONE_DELAY_MILSEC))
        {
            if (eeprom_offset < EEPROM_SIZE) 
            {
                if ( (pwm == 255) && (last_pwm == 255) )
                {
                    // two consecutive buzzer valuses terminate the profile.
                    initCommandBuffer();
                }
                else
                {
                    eeprom_offset++;
                    last_pwm = pwm;
                    reflow_zone_started_at += REFLOW_ZONE_DELAY_MILSEC; /* advance start time an exact amount*/
                    command_done = 11; /* loop through all eeprom values */
                }
            }
            else
            {
                initCommandBuffer();
            }
        }
    }
    
    else
    {
        printf_P(PSTR("{\"err\":\"ReflowCmdWTF\"}\r\n"));
        initCommandBuffer();
    }
}
