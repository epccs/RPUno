/*
solenoid is part of Solenoid, it is a serial command interface to some solenoid control functions, 
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
#include <stdbool.h>
#include "../lib/parse.h"
#include "../lib/timers.h"
#include "../lib/icp1.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "solenoid.h"

// IO on RPUno used by the K3 board
#define E3 DIO14
#define A0 DIO10
#define A1 DIO11
#define A2 DIO12
// DIO13 is used to blink I2C status in main()
// DIO17 is used to blink Day-Night status in main()

//The EEPROM memory usage is as follows. 
#define EE_SOLENOID_BASE_ADDR 40
// each solenoid K1..K3 has an array of settings offset by
#define EE_SOLENOID_ARRAY_OFFSET 20
// each setting is at this byte from the array offset
#define EE_SOLENOID_ID 0
#define EE_SOLENOID_DLY_STRT 2
#define EE_SOLENOID_RUNTIME 6
#define EE_SOLENOID_DELAY 10
#define EE_SOLENOID_FLW_STP 14
#define EE_SOLENOID_CYCLES 18

// index zero is not a solenoid (e.g. 74HC238 outputs Y0 is discharge and Y1 is boost)
#define DISCHARGE 0
#define BOOST 0

#define BOOST_TIME 650
#define PWR_HBRIDGE 50
#define SOLENOID_CLOSE 1000

#define SEC_IN_HR 3600UL
#define SEC_IN_6HR 21600UL
#define SEC_IN_DAY 86400UL

#define FLOW_NOT_SET 0xFFFFFFFFUL

typedef struct {
    unsigned long started_at;  // holds the start time of the present operation
    unsigned long delay_start_sec; // delay befor first set operation (one time)
    unsigned long runtime_sec; // delay after set operation and befor reset
    unsigned long delay_sec;   // delay after reset operation and befor next set operation if (cycles > 0)
    uint8_t cycle_state;
    uint8_t cycles; // keep cycling until zero
    uint32_t flow_stop; // pulse counts after start at which to reset the solenoid
    uint32_t flow_cnt_start; // pulse count when set occured (e.g. ICP1 capture events)
    uint32_t flow_cnt_stop; // pulse count when reset occured
    uint32_t flow_cnt_bank; // pulse count accumulate or store
    // uint8_t use_flow_meter; // the defaut is for all solenoids to use the flow meter, but what if I want to inject fertilizers into flow
    unsigned long cycle_millis_start;
    unsigned long cycle_millis_stop;
    unsigned long cycle_millis_bank; // millis count accumulate or store
}  solenoidTimer;

static solenoidTimer k[SOLENOID_COUNT];

static uint8_t boostInUse; // 0 if free, 1 thru SOLENOID_COUNT and is the solenoid using boost
static uint8_t flowInUse; // 0 only one solenoid can be active so its flow count can be measured

// arg[0] is solenoid, arg[1] is delay_start
void KDelayStart(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_6HR 
        unsigned long delay_start = is_arg_in_ul_range(1,1,SEC_IN_6HR);
        if (! delay_start)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].delay_start_sec = delay_start;
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\""),(k[k_solenoid-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid, arg[1] is runtime
void KRunTime(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_6HR 
        unsigned long runtime = is_arg_in_ul_range(1,1,SEC_IN_6HR);
        if (! runtime)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].runtime_sec = runtime;
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid, arg[1] is delay
void KDelay(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_DAY 
        unsigned long delay = is_arg_in_ul_range(1,1,SEC_IN_DAY);
        if (! delay)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].delay_sec = delay;
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\""),(k[k_solenoid-1].delay_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid, arg[1] is flow_stop
void KFlowStop(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..FLOW_NOT_SET 
        unsigned long flow_stop = is_arg_in_ul_range(1,1,FLOW_NOT_SET);
        if (! flow_stop)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].flow_stop = flow_stop;
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"flow_stop\":\"%lu\","),(k[k_solenoid-1].flow_stop));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid, [arg[1] is cycles]
void KRun(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        
        uint8_t cycles = k[k_solenoid-1].cycles;
        uint8_t arg1 = is_arg_in_uint8_range(1,1,0xFF);
        if (arg1)
        {
            // and arg[1] value is 1..0xFF 
            cycles =arg1;
            if (! cycles)
            {
                initCommandBuffer();
                return;
            }
        }

         // don't run a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].cycle_state = 1;
        k[k_solenoid-1].cycles = cycles;
        k[k_solenoid-1].flow_cnt_bank = 0;
        k[k_solenoid-1].cycle_millis_bank = 0;
        k[k_solenoid-1].started_at = millis(); //delay_start_sec is timed from now
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(k[k_solenoid-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(k[k_solenoid-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\""),(k[k_solenoid-1].cycles));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        if (k[k_solenoid-1].flow_stop != FLOW_NOT_SET)
        {
            printf_P(PSTR(",\"flow_stop\":\"%lu\""),(k[k_solenoid-1].flow_stop));
        }
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid, arg[1] is cycles
void KSave(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..0xFF 
        uint8_t cycles = is_arg_in_uint8_range(1,1,0xFF);
        if (! cycles)
        {
            initCommandBuffer();
            return;
        }
         // don't save a solenoid that is in use
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        if ( eeprom_is_ready() )
        {
            k[k_solenoid-1].cycle_state = 0;
            k[k_solenoid-1].cycles = cycles;
            k[k_solenoid-1].flow_cnt_bank = 0;
            k[k_solenoid-1].cycle_millis_bank = 0;
            uint16_t value = ((uint16_t) (k_solenoid)) + 0x4B30; //ascii bytes for 'K1', 'K2'...
            eeprom_write_word( (uint16_t *)((k_solenoid-1)*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_ID), value);
            printf_P(PSTR("{\"K%d\":{"),k_solenoid);
            command_done = 11;
        }
    }
    else if ( (command_done == 11) )
    {  
        if ( eeprom_is_ready() )
        {
            uint8_t k_solenoid = atoi(arg[0]);
            uint32_t value = k[k_solenoid-1].delay_start_sec;
            eeprom_write_dword( (uint32_t *)((k_solenoid-1)*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_DLY_STRT), value);
            printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(value));
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t k_solenoid = atoi(arg[0]);
            uint32_t value = k[k_solenoid-1].runtime_sec;
            eeprom_write_dword( (uint32_t *)((k_solenoid-1)*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_RUNTIME), value);
            printf_P(PSTR("\"runtime_sec\":\"%lu\","),(value));
            command_done = 13;
        }
    }
    else if ( (command_done == 13) )
    {  
        if ( eeprom_is_ready() )
        {
            uint8_t k_solenoid = atoi(arg[0]);
            uint32_t value = k[k_solenoid-1].delay_sec;
            eeprom_write_dword( (uint32_t *)((k_solenoid-1)*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_DELAY), value);
            printf_P(PSTR("\"delay_sec\":\"%lu\","),(value));
            command_done = 14;
        }
    }
    else if ( (command_done == 14) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t k_solenoid = atoi(arg[0]);
            uint32_t value = k[k_solenoid-1].flow_stop;
            eeprom_write_dword( (uint32_t *)((k_solenoid-1)*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_FLW_STP), value);
            if (k[k_solenoid-1].flow_stop != FLOW_NOT_SET)
            {
                printf_P(PSTR("\"flow_stop\":\"%lu\","),(value));
            }
            command_done = 15;
        }
    }
    else if ( (command_done == 15) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t k_solenoid = atoi(arg[0]);
            uint8_t value = k[k_solenoid-1].cycles;
            eeprom_write_byte( (uint8_t *)((k_solenoid-1)*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_CYCLES), value);
            printf_P(PSTR("\"cycles\":\"%d\""),(value));
            command_done = 16;
        }
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid
void KLoad(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }

         // don't load a solenoid that is in use
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"K%dInUse\"}\r\n"),k_solenoid);
            initCommandBuffer();
            return;
        }
        if ( eeprom_is_ready() )
        {
            if (LoadKControlFromEEPROM(k_solenoid))
            {
                k[k_solenoid-1].flow_cnt_bank = 0;
                k[k_solenoid-1].cycle_millis_bank = 0;
                printf_P(PSTR("{\"K%d\":{"),k_solenoid);
                command_done = 11;
            }
            else
            {
                printf_P(PSTR("{\"err\":\"LdFailK%dnoEEP\"}\r\n"),k_solenoid);
                initCommandBuffer();
                return;
            }
        }
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(k[k_solenoid-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(k[k_solenoid-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        uint8_t k_solenoid = atoi(arg[0]);
        if (k[k_solenoid-1].flow_stop != FLOW_NOT_SET)
        {
            printf_P(PSTR("\"flow_stop\":\"%lu\","),(k[k_solenoid-1].flow_stop));
        }
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\""),(k[k_solenoid-1].cycles));
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// arg[0] is solenoid
void KTime(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycle_state\":\"%d\","),(k[k_solenoid-1].cycle_state));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\","),(k[k_solenoid-1].cycles));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycle_millis\":\"%lu\""),(k[k_solenoid-1].cycle_millis_bank));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"TmCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is solenoid
void KFlow(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycle_state\":\"%d\","),(k[k_solenoid-1].cycle_state));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\","),(k[k_solenoid-1].cycles));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"flow_cnt\":\"%lu\""),(k[k_solenoid-1].flow_cnt_bank));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

// solenoid number one has values in its control array at index zero
void StopK(uint8_t solenoid_num)
{
    if (k[solenoid_num-1].cycle_state)
    {
        k[solenoid_num-1].delay_start_sec = 1;
        k[solenoid_num-1].runtime_sec = 1; 
        k[solenoid_num-1].delay_sec = 1;
        k[solenoid_num-1].flow_stop = FLOW_NOT_SET;
        k[solenoid_num-1].cycles = 1;
    }
}

// arg[0] is solenoid
void KStop(void)
{
    if ( (command_done == 10) )
    {
        uint8_t k_solenoid = is_arg_in_uint8_range(0,1,SOLENOID_COUNT);
        if (! k_solenoid)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        StopK(k_solenoid);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(k[k_solenoid-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(k[k_solenoid-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%u\""),(k[k_solenoid-1].cycles));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        if (k[k_solenoid-1].flow_stop != FLOW_NOT_SET)
        {
            printf_P(PSTR(",\"mahr_stop\":\"%lu\""),(k[k_solenoid-1].flow_stop));
        }
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        initCommandBuffer();
    }
}

void bridge_off() {
    digitalWrite(E3,LOW); // disables 74HC238 e.g. all the half bridge boost and discharge are off
}

// given a solenoid index config the bridges and apply the SET pulse. 
// A zero index will set discharge.
void set_solenoid(uint8_t k_indx) 
{
    digitalWrite(E3,LOW); // disables 74HC238 e.g. all the half bridge boost and discharge are off
    
    // each ndex has two operations
    // left shift for a value twice that of the index
    // 0 => 0<<1 => Y0 is DISCHARGE
    // 1 => 1<<1 => Y2 is SET-K1
    // 2 => 2<<1 => Y4 is SET-K2 
    // 3 => 3<<1 => Y6 is SET-K3
    uint8_t set = k_indx << 1; 
    
    // set A0:A1:A2 bit
    digitalWrite(A0, LOW); // A0 is always LOW, because the 74HC238 decode for SET is an even value 
    if( set & (1<<1) )
    {
        digitalWrite(A1, HIGH);
    }
    else
    {
        digitalWrite(A1, LOW);
    }
    if( set & (1<<2) )
    {
        digitalWrite(A2, HIGH);
    }
    else
    {
        digitalWrite(A2, LOW);
    }
    
    // with addrss set enable the 74HC238 chip
    digitalWrite(E3,HIGH);
}

// given a solenoid index config the bridge and apply a RESET pulse. 
// A zero index will trun on the SMPS boost and charge the storage cap.
void reset_solenoid(uint8_t k_indx) 
{
    digitalWrite(E3,LOW); // disables 74HC238 e.g. all the half bridge boost and discharge are off
    
    // each ndex has two operations
    // left shift for a value twice that of the index
    // 0 => (0<<1)+1 => Y1 is SMPS
    // 1 =>(1<<1)+1 => Y3 is RESET-K1
    // 2 => (2<<1)+1 => Y5 is RESET-K2 
    // 3 => (3<<1)+1 => Y7 is RESET-K3
    uint8_t set = k_indx << 1; 
    
    // set A0:A1:A2 bit
    digitalWrite(A0, HIGH); // A0 is always HIGH, because the 74HC238 decode for RESET is an odd value 
    if( set & (1<<1) )
    {
        digitalWrite(A1, HIGH);
    }
    else
    {
        digitalWrite(A1, LOW);
    }
    if( set & (1<<2) )
    {
        digitalWrite(A2, HIGH);
    }
    else
    {
        digitalWrite(A2, LOW);
    }
    
    // with addrss set enable the 74HC238 chip
    digitalWrite(E3,HIGH);
}

#define SOLENOID_STATE_NOT_ACTIVE 0
#define SOLENOID_STATE_PRESTART 1
#define SOLENOID_STATE_BOOST_BEFOR_SET 2
#define SOLENOID_STATE_SET_HBRIDGE 3
#define SOLENOID_STATE_RUNTIME 4
#define SOLENOID_STATE_RUNDONE_BOOST_BEFOR_RESET 5
#define SOLENOID_STATE_RUNDONE_RESET 6
#define SOLENOID_STATE_FLOWDONE_BOOST_BEFOR_RESET 7
#define SOLENOID_STATE_FLOWDONE_RESET 8
#define SOLENOID_STATE_RESET_HBRIDGE 9
#define SOLENOID_STATE_FLOWCOUNT 10
#define SOLENOID_STATE_DELAY 11
/* operate the solenoid states without blocking
    cycle_state 
    0 = solenoid not active
    1 = active, wait for start time (delay_start_sec), then start a Boost charge.
    2 = wait for BOOST_TIME to finish, then setup to measure flow and set solenoid.
    3 = wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
    4 = wait for runTime, then select state 7. or if within 16M counts of flow_stop select state 5. 
    5 = wait for boostInUse, then start a Boost charge
    6 = wait for reset (runtime) and slect state 9.
    7 = wait for boostInUse, then start a Boost charge
    8 = wait for reset (flow amount).
    9 = wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
    10 = wait for SOLENOID_CLOSE time, then measure flow count, if cycles is set then state 0.
    11 = wait for delay time, then loop to cycle_state = 1 (backdate so delay_start_sec is not used in each loop).
*/ 
void KControl() {
    for(int i = 0; i < SOLENOID_COUNT; i++){
        // active, wait for start time (delay_start_sec), then start a Boost charge.
        if ((k[i].cycle_state == SOLENOID_STATE_PRESTART) && !boostInUse && !flowInUse) 
        {
            unsigned long kRuntime= millis() - k[i].started_at;
            if ((kRuntime) > ((unsigned long)k[i].delay_start_sec * 1000)) 
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = SOLENOID_STATE_BOOST_BEFOR_SET;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1; //allow this solenoid to use boost
                flowInUse = i+1; //allow this solenoid to use flow meter
                break;
            }
        }
        if (flowInUse == i+1) // only let the solenoid that has been allocated use of the flow meter run
        {
            // wait for BOOST_TIME to finish, then  setup to measure pulse count and select solenoid control line to drive.
            if ((k[i].cycle_state == SOLENOID_STATE_BOOST_BEFOR_SET) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)BOOST_TIME)) 
                {
                    k[i].flow_cnt_start =  icp1.count;
                    set_solenoid(i+1);
                    k[i].started_at = millis(); //start time that H-bridge SET has power
                    k[i].cycle_state = SOLENOID_STATE_SET_HBRIDGE;
                    break;
                }
            }

            // wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
            if ((k[i].cycle_state == SOLENOID_STATE_SET_HBRIDGE) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)PWR_HBRIDGE)) 
                {
                    boostInUse = 0;
                    bridge_off();
                    k[i].started_at = millis(); //start runtime e.g. solenoid is set
                    k[i].cycle_millis_start = millis(); 
                    k[i].cycle_state = SOLENOID_STATE_RUNTIME;
                    break;
                }
            }

            // wait for runTime, then select state 7. or if not FLOW_NOT_SET counts select state 5.
            if (k[i].cycle_state == SOLENOID_STATE_RUNTIME) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)k[i].runtime_sec * 1000)-1) 
                {
                    k[i].cycle_millis_stop = millis(); // correction of -1 millis was added so timer shows expected value
                    k[i].cycle_state = SOLENOID_STATE_RUNDONE_BOOST_BEFOR_RESET;
                }
                if (k[i].flow_stop != FLOW_NOT_SET) 
                {  
                    if ( (icp1.count - k[i].flow_cnt_start) >= k[i].flow_stop) 
                    {
                        k[i].cycle_state = SOLENOID_STATE_FLOWDONE_BOOST_BEFOR_RESET;
                        break;
                    }
                }
            }

            // wait for not boostInUse, then start a Boost charge
            if ((k[i].cycle_state == SOLENOID_STATE_RUNDONE_BOOST_BEFOR_RESET) && !boostInUse) 
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = SOLENOID_STATE_RUNDONE_RESET;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1;
                break;
            }

            // wait for BOOST_TIME to finish, then reset solenoid and measure flow.
            if ((k[i].cycle_state == SOLENOID_STATE_RUNDONE_RESET) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if (((kRuntime) > ((unsigned long)BOOST_TIME)) ) 
                {
                    reset_solenoid(i+1);
                    k[i].started_at = millis(); //start time that H-bridge RESET has power
                    k[i].cycle_state = SOLENOID_STATE_RESET_HBRIDGE;
                    break;
                }
            }

            // wait for not boostInUse, then start a Boost charge
            if ((k[i].cycle_state == SOLENOID_STATE_FLOWDONE_BOOST_BEFOR_RESET) && !boostInUse)
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = SOLENOID_STATE_FLOWDONE_RESET;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1;
                break;
            }

            // wait for BOOST_TIME to finish, then reset solenoid and measure flow.
            if ((k[i].cycle_state == SOLENOID_STATE_FLOWDONE_RESET) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)BOOST_TIME)) 
                {
                    reset_solenoid(i+1);
                    k[i].started_at = millis(); //start time that H-bridge RESET has power
                    k[i].cycle_state = SOLENOID_STATE_RESET_HBRIDGE;
                    break;
                }
            }
            
            // wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
            if ((k[i].cycle_state == SOLENOID_STATE_RESET_HBRIDGE) && (boostInUse == i+1))
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)PWR_HBRIDGE)) 
                {
                    boostInUse = 0;
                    bridge_off();
                    k[i].cycle_millis_bank += (k[i].cycle_millis_stop - k[i].cycle_millis_start);
                    k[i].started_at = millis(); 
                    k[i].cycle_state = SOLENOID_STATE_FLOWCOUNT;
                    break;
                }
            }
              
            // wait for SOLENOID_CLOSE time, then measure flow count.
            if ((k[i].cycle_state == SOLENOID_STATE_FLOWCOUNT)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)SOLENOID_CLOSE)) 
                {
                    flowInUse = 0; // this will allow another solenoid cycle_state to progress 
                    k[i].flow_cnt_stop = icp1.count; //record the flow meter pulse count after solenoid has closed
                    k[i].flow_cnt_bank += (k[i].flow_cnt_stop - k[i].flow_cnt_start);
                    if (k[i].cycles)
                    {
                        k[i].cycles = k[i].cycles -1;
                    }
                    if (k[i].cycles)
                    {
                        k[i].started_at = millis();
                        k[i].cycle_state = SOLENOID_STATE_DELAY;
                        break;
                    }
                    else
                    {
                        k[i].started_at = 0;
                        k[i].cycle_state = SOLENOID_STATE_NOT_ACTIVE;
                        break;
                    }
                }
            }
        }
        
        // delay between cycles
        if ((k[i].cycle_state == SOLENOID_STATE_DELAY) && !boostInUse && !flowInUse)
        {
            unsigned long kRuntime= millis() - k[i].started_at;
            if ( (kRuntime) > ((unsigned long)k[i].delay_sec*1000UL) ) 
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = SOLENOID_STATE_BOOST_BEFOR_SET;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1; //allow this solenoid to use boost
                flowInUse = i+1; //allow this solenoid to use flow meter
                break;
            }
        }
    }
}

// warning this sets all solenoids for 1 Second operation, 1 cycle, and sets cycle_state so they will run
void Reset_All_K() {
    for(uint8_t i = 0; i < SOLENOID_COUNT; i++)
    {
        k[i].started_at = millis(); // used to delay start
        k[i].delay_start_sec = i+i+i+1; // every 3 sec starting at 1, then 4, and 7
        k[i].runtime_sec = 1; 
        k[i].delay_sec = SEC_IN_HR;
        k[i].flow_stop = FLOW_NOT_SET;
        k[i].cycles = 1;
        k[i].cycle_state = 1;
        k[i].flow_cnt_bank = 0;
        k[i].cycle_millis_bank = 0;
    }
}

uint8_t LoadKControlFromEEPROM(uint8_t solenoid) 
{
    uint16_t i = solenoid-1;
    if (!k[i].cycle_state)
    {
        uint16_t id = eeprom_read_word((uint16_t*)(i*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_ID));
        if (id == (i+0x4B31) ) // 'K' is 0x4B and '1' is 0x31, thus K1, K2...
        {
            k[i].started_at = millis(); // delay start is from now.
            k[i].delay_start_sec =eeprom_read_dword((uint32_t*)(i*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_DLY_STRT)); 
            k[i].runtime_sec = eeprom_read_dword((uint32_t*)(i*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_RUNTIME));  
            k[i].delay_sec = eeprom_read_dword((uint32_t*)(i*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_DELAY)); 
            k[i].flow_stop = eeprom_read_dword((uint32_t*)(i*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_FLW_STP)); 
            k[i].cycles = eeprom_read_byte((uint8_t*)(i*EE_SOLENOID_ARRAY_OFFSET+EE_SOLENOID_BASE_ADDR+EE_SOLENOID_CYCLES)); 
            k[i].flow_cnt_bank = 0;
            k[i].cycle_millis_bank = 0;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

// return the solenoid cycle_state (e.g. tells if it is running)
uint8_t KLive(uint8_t solenoid) 
{
    uint16_t i = solenoid-1;
    if (i<SOLENOID_COUNT)
    {
        return k[i].cycle_state;
    }
    else return 0xFF; // not a valid solenoid 
}

// start the solenoid if it is not running and return cycle state
uint8_t StartK(uint8_t solenoid) 
{
    uint16_t i = solenoid-1;
    if (i<SOLENOID_COUNT )
    {
        if (! k[i].cycle_state) k[i].cycle_state = 1;
        return k[i].cycle_state;
    }
    else return 0x0; // not a valid solenoid
}

// only use init at setup() not durring loop()
void init_K(void) 
{
    boostInUse = 0;
    flowInUse = 0;
    pinMode(E3,OUTPUT);
    digitalWrite(E3,LOW);
    pinMode(A0,OUTPUT);
    digitalWrite(A0,LOW);
    pinMode(A1,OUTPUT);
    digitalWrite(A1,LOW);
    pinMode(A2,OUTPUT);
    digitalWrite(A2,LOW); 
}

