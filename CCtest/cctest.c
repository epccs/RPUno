/*
cctest is part of CCtest, it is a test for RPUno's battery charge control and PV power, 
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
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "cctest.h"

#define LDTST_PRINT_DELAY_MILSEC 2000
#define ABSORPTION_DELAY_MILSEC 10800000UL
static unsigned long serial_print_started_at;
static unsigned long absorption_started_at;
static uint8_t absorption;
static uint8_t runtest;

// ADC channels: 7 is battery, 6 is PV, 5 is not-used 4 is battery discharge, 3 is battery charge, 2 is PV_I.
#define START_CHANNEL 1
#define END_CHANNEL 7
static uint8_t adc_index;

static uint8_t step_index;
static uint8_t start_ld_step;
static uint8_t end_ld_step;

// Voltage at which discharge stops and charging starts is in miliVolt
#define MAX_DISCHARGE 6200
#define FIRST_DISCHARGE 6550
#define DISCHARGE_STEP 50
static uint16_t bat_discharge;

float pv_v;
float pv_i;
float pwr_v;
float chrg_i;
float dischrg_i;
float bat_report;

//absorption debuging
static uint8_t absorption_progress;

void init_load(void)
{
    // set digital lines to contorl load and turn it off
    pinMode(LD0,OUTPUT);
    digitalWrite(LD0, LOW);
    pinMode(LD1,OUTPUT);
    digitalWrite(LD1, LOW);
    pinMode(LD2,OUTPUT);
    digitalWrite(LD2, LOW);
    pinMode(LD3,OUTPUT);
    digitalWrite(LD3, LOW);
}

void init_pv(void)
{
    // set digital lines to contorl the LT3652 PV MPPT buck converter and turn it on
    // note R4 may have been removed if JTAG was used
    pinMode(SHUTDOWN,OUTPUT);
    digitalWrite(SHUTDOWN, LOW);
}

void load_step(uint8_t step)
{
    // set LD0 bit
    if(step & (1<<0) )
    {
        digitalWrite(LD0, HIGH);
    }
    else
    {
        digitalWrite(LD0, LOW);
    }
    
    // set LD1 bit
    if(step & (1<<1) )
    {
        digitalWrite(LD1, HIGH);
    }
    else
    {
        digitalWrite(LD1, LOW);
    }
    
    // set LD2 bit
    if(step & (1<<2) )
    {
        digitalWrite(LD2, HIGH);
    }
    else
    {
        digitalWrite(LD2, LOW);
    }
    
    // set LD3 bit
    if(step & (1<<3) )
    {
        digitalWrite(LD3, HIGH);
    }
    else
    {
        digitalWrite(LD3, LOW);
    }
}


void CCtest(void)
{
    if ( command_done == 10 )
    {
        if (arg_count == 0)
        {
            start_ld_step = START_LD_STEP;
            end_ld_step = END_LD_STEP;
            step_index = start_ld_step;
            bat_discharge = MAX_DISCHARGE;
            load_step(step_index);
            runtest = 0;
            absorption = 0;
            command_done = 11;
        }
        
        // arg must be a load step setting
        if (arg_count == 1)
        {
            // check that argument[0] is in the range 0..15
            if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 0) || (atoi(arg[0]) > END_LD_STEP) )
            {
                printf_P(PSTR("{\"err\":\"LdStpSize %d\"}\r\n"), END_LD_STEP);
                initCommandBuffer();
                return;
            }
            
            // run at a fixed load with RUNTEST time delay.
            start_ld_step = atoi(arg[0]);
            end_ld_step = start_ld_step;
            step_index = start_ld_step;
            bat_discharge = MAX_DISCHARGE;
            load_step(step_index);
            runtest = 1;
            absorption = 0;
            
            //Shutdown the LT3652
            digitalWrite(SHUTDOWN, HIGH);
            command_done = 11;
        }
        
        // arg[0] is start_step, arg[1] is end_step, and arg[2] is the battery voltage (in mV) to switch from discharging to charging
        if (arg_count == 3)
        {
            // check that argument[0] is in the range 0..15
            if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 0) || (atoi(arg[0]) > END_LD_STEP) )
            {
                printf_P(PSTR("{\"err\":\"LdStartMax %d\"}\r\n"), END_LD_STEP);
                initCommandBuffer();
                return;
            }

            // check that argument[1] is in the range 0..15
            if ( ( !( isdigit(arg[1][0]) ) ) || (atoi(arg[1]) < 0) || (atoi(arg[1]) > END_LD_STEP) )
            {
                printf_P(PSTR("{\"err\":\"LdStopMax %d\"}\r\n"), END_LD_STEP);
                initCommandBuffer();
                return;
            }

            // check that argument[2] is in the range 6000..6500
            if ( ( !( isdigit(arg[2][0]) ) ) || (atoi(arg[2]) < 6000) || (atoi(arg[2]) > 6500) )
            {
                printf_P(PSTR("{\"err\":\"LdVolt6V-6V5\"}\r\n"));
                initCommandBuffer();
                return;
            }

            // run at a fixed load with RUNTEST time delay.
            start_ld_step = (uint8_t) (atoi(arg[0]));
            end_ld_step = (uint8_t) (atoi(arg[1]));
            step_index = start_ld_step;
            bat_discharge = (uint16_t) (atoi(arg[2]));
            load_step(step_index);
            runtest = 0;
            absorption = 0;
            command_done = 11;
        }
        pv_v = analogRead(PV_V)*(5.0/1024.0)*(532.0/100.0);
        pv_i = analogRead(PV_I)*(5.0/1024.0)/(0.068*50.0);
        pwr_v = analogRead(PWR_V)*(5.0/1024.0)*(3.0/2.0);
        chrg_i = analogRead(CHRG_I)*(5.0/1024.0)/(0.068*50.0);
        dischrg_i = analogRead(DISCHRG_I)*(5.0/1024.0)/(0.068*50.0);
    }

    if ( command_done == 11 )
    {
        serial_print_started_at = millis();
        printf_P(PSTR("{"));
        adc_index= START_CHANNEL;
        command_done = 12;
    }
    
    else if ( command_done == 12 )
    { // JSON index 

        if (adc_index == PV_I)
        {
            if (pv_i > 0.001)
            {
                printf_P(PSTR("\"PV_A\":"));
            }
        }

        if (adc_index == CHRG_I)
        {
            if (chrg_i > 0.002)
            {
                printf_P(PSTR("\"CHRG_A\":"));
            }
        }

        if (adc_index == DISCHRG_I) //ADC3
        {
            if (dischrg_i > 0.002)
            {
                printf_P(PSTR("\"DIS_A\":"));
            }
        }

        //skip ADC4 and ADC5 they are for I2C

        if (adc_index == PV_V) // ADC6
        {
            printf_P(PSTR("\"PV_V\":"));
        }

        if (adc_index == PWR_V) // ADC7
        {
            printf_P(PSTR("\"PWR_V\":"));
        }
        command_done = 13;
    }

    else if ( command_done == 13 )
    {
        if (adc_index == PV_I) // CCtest board current sense that can be connected to ADC1.
        {
            if (pv_i > 0.001)
            {
                printf_P(PSTR("\"%1.3f\","),pv_i);
            }
        }

        if (adc_index == CHRG_I) // RPUno has ADC2 connected to high side current sense to measure battery charging.
        {
            if (chrg_i > 0.002)
            {
                printf_P(PSTR("\"%1.3f\","),chrg_i);
            }
        }

        if (adc_index == DISCHRG_I) // RPUno has ADC3 connected to high side current sense to measure battery discharg.
        {
            if (dischrg_i > 0.002)
            {
                printf_P(PSTR("\"%1.3f\","),dischrg_i);
            }
        }

        // RPUno does have ADC4 or ADC5, they are for I2C with the RPUadpt/RPUftdi/RPUpi shields
        
        if (adc_index == PV_V) // RPUno has ADC6 connected to a voltage divider from the solar input.
        {
            printf_P(PSTR("\"%1.2f\","),pv_v);
        }

        if (adc_index == PWR_V) // RPUno has ADC7 connected a voltage divider from the battery (PWR).
        {
            printf_P(PSTR("\"%1.2f\","),pwr_v);
        }

        if ( (adc_index+1) > END_CHANNEL) 
        {
            command_done = 14;
        }
        else
        {
            adc_index++;
            command_done = 12;
        }
    }

    else if ( command_done == 14 )
    {
        printf_P(PSTR("\"TIME\":"));
        command_done = 15;
    }

    else if ( command_done == 15 )
    {
        printf_P(PSTR("\"%lu\","),millis());
        command_done = 16;
    }

    else if ( command_done == 16 )
    {
        printf_P(PSTR("\"LD\":"));
        command_done = 17;
    }

    else if ( command_done == 17 )
    {
        printf_P(PSTR("\"%d\""),step_index);
        printf_P(PSTR("}\r\n"));
        command_done = 18;
    }

    // check if at a load change condition
    else if ( command_done == 18 )
    {
        pwr_v = analogRead(PWR_V)*(5.0/1024.0)*(3.0/2.0);
        pv_v = analogRead(PV_V)*(5.0/1024.0)*(532.0/100.0);
        pv_i = analogRead(PV_I)*(5.0/1024.0)/(0.068*50.0);
        chrg_i = analogRead(CHRG_I)*(5.0/1024.0)/(0.068*50.0);
        dischrg_i = analogRead(DISCHRG_I)*(5.0/1024.0)/(0.068*50.0);
        
        // absorption
        if (absorption)
        {
            command_done = 21;
        }
        
        // when at the highest load setting check if the load needs turned off, and charging enabled.  
        else if ( (step_index+1) > end_ld_step) 
        {
            // max load needs to end when battery voltage has discharged to the setpoint 
            if ( ( pwr_v < (bat_discharge/1000.0) ) && (pwr_v > 4.0) )
            {
                step_index = start_ld_step;
                load_step(step_index);
                
                // switch form loadtest delay to runtest delay
                runtest =1;
                
                //Enable the LT3652
                digitalWrite(SHUTDOWN, LOW);
                
                // verfiy that we have MPPT mode
                command_done = 19;
            }

            // report at voltage levels
            else if ( ( pwr_v < bat_report ) && (pwr_v > 4.0) )
            {
                command_done = 20;
                bat_report = bat_report - DISCHARGE_STEP/1000.0;
            }
        }
        else
        {
            if (arg_count == 1)
            {
                command_done = 20;
            }                
            // If LT3652 is enabled then the MPPT mode can be used to wait
            // for CV (aka float) mode
            else if ( (pv_v > 18.0 ) && ( (arg_count == 0) || (arg_count == 3) ) )
            {
                // shutdown PV so the the full discharge load is shown for each step
                if ( (step_index == start_ld_step) && (digitalRead(SHUTDOWN) == LOW) && (!runtest) )
                {
                    //Shutdown the LT3652 (note R4 may have been removed if JTAG was used)
                    digitalWrite(SHUTDOWN, HIGH);
                    bat_report = FIRST_DISCHARGE/1000.0;
                }
                else if (!runtest)
                { // skip step through the load settings 
                    step_index++;
                    load_step(step_index);
                    command_done = 20;
                }
                else
                {
                    // absorption cycle, note the LT3652 is not yet shutdown
                    absorption_started_at = millis();
                    printf_P(PSTR("{\"rpt\":\"AbsorptionBeg\"}\r\n"));
                    absorption = 1;
                    absorption_progress = 0;
                    command_done = 21;
                }
            }
        }
    }

    else if ( command_done == 19 ) 
    { 
        if ( (arg_count == 0) || (arg_count == 3) )
        {
            // verify LT3652 gets into MPPT mode so that charging is not skiped
            // note PV_I and CHRG_I, are huge when MPPT mode starts on my bench supply because it has a large bult capacitor to pull down.
            // also DISCHRG is nil when MPPT has got going, but I want to report the old value
            pv_v = analogRead(PV_V)*(5.0/1024.0)*(532.0/100.0);
            if ( (pv_v > 16.0 ) &&  (pv_v < 17.0 ))
            {
                printf_P(PSTR("{\"rpt\":\"VerifyMPPT\"}\r\n"));
                command_done = 20;
            }
        }
        else
        {
            command_done = 20;
        }
    }

    else if ( command_done == 20 ) 
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;

        if ((kRuntime) > ((unsigned long)LDTST_PRINT_DELAY_MILSEC))
        {
            command_done = 11; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }

    else if ( command_done == 21 ) // absorption cycle
    {
        unsigned long kRuntime= millis() - absorption_started_at;

        if ((kRuntime) > ((unsigned long)ABSORPTION_DELAY_MILSEC))
        {
            absorption = 0;
            printf_P(PSTR("{\"rpt\":\"AbsorptionEnd\"}\r\n"));
            command_done = 22; // verify that it is night befor starting load
        }
        else
        {
            // used for debuging the absorption cycle
            uint8_t progress = ( (kRuntime/2000) % 60 );
            if (absorption_progress < progress )
            {
                printf_P(PSTR("*"));
                absorption_progress = progress;
            }
            else if ( (progress == 0) && (absorption_progress > 0) )
            {
                printf_P(PSTR("\r\n"));
                absorption_progress = 0;
            }
        }
    }

    else if ( command_done == 22 ) //verify night
    { 
        if ( (arg_count == 0) || (arg_count == 3) )
        {
            // wait for night to start load
            pv_v = analogRead(PV_V)*(5.0/1024.0)*(532.0/100.0);
            if ( (pv_v > 0.0 ) &&  (pv_v < 5.0 ))
            {
                printf_P(PSTR("{\"rpt\":\"VerifyNight\"}\r\n"));
                
                // set the load
                step_index = end_ld_step;
                load_step(step_index);
                
                //Shutdown the LT3652 
                digitalWrite(SHUTDOWN, HIGH);
                bat_report = FIRST_DISCHARGE/1000.0;
                command_done = 20;
            }
        }
        else
        {
            command_done = 20;
        }
    }

    else
    {
        printf_P(PSTR("{\"err\":\"AdcCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

