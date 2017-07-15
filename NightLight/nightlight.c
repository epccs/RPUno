/*
nightlight is part of NightLight, it is a serial command interface to some light control functions, 
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
#include "nightlight.h"

// IO on RPUno used by the K3 board
#define E3 3
#define A0 10
#define A1 11
#define A2 12
// 13 is used to blink I2C status in main()
// 4 is used to blink Day-Night status in main()

//The EEPROM memory usage is as follows. 
#define EE_LED_BASE_ADDR 200
// each LED 1..3 has an array of settings offset by
#define EE_LED_ARRAY_OFFSET 20
// each setting is at this byte from the array offset
#define EE_LED_ID 0
#define EE_LED_DLY_STRT 2
#define EE_LED_RUNTIME 6
#define EE_LED_DELAY 10
#define EE_LED_MAHR_STP 14
#define EE_LED_CYCLES 18

// index zero is not a solenoid (e.g. 74HC238 outputs Y0 is discharge and Y1 is boost)
#define DISCHARGE 0
#define BOOST 0

#define BOOST_TIME 650
#define PWR_HBRIDGE 50
#define SOLENOID_CLOSE 1000

#define SEC_IN_HR 3600UL
#define SEC_IN_6HR 21600UL
#define SEC_IN_DAY 86400UL

#define MAHR_NOT_SET 0xFFFFFFFFUL

typedef struct {
    unsigned long started_at;  // holds the start time of the present operation
    unsigned long delay_start_sec; // delay befor first set operation (one time)
    unsigned long runtime_sec; // delay after set operation and befor reset
    unsigned long delay_sec;   // delay after reset operation and befor next set operation if (cycles > 0)
    uint8_t cycle_state;
    uint8_t cycles; // keep cycling until zero
    uint32_t mahr_stop; // mAHr usage after start at which to stop the led
    uint32_t flow_cnt_start; // pulse count when set occured (e.g. ICP1 capture events)
    uint32_t flow_cnt_stop; // pulse count when reset occured
    uint32_t flow_cnt_bank; // pulse count accumulate or store
    // uint8_t use_flow_meter; // the defaut is for all solenoids to use the flow meter, but what if I want to inject fertilizers into flow
    unsigned long cycle_millis_start;
    unsigned long cycle_millis_stop;
    unsigned long cycle_millis_bank; // millis count accumulate or store
}  ledTimer;

static ledTimer led[LEDSTRING_COUNT];

typedef struct {
  uint8_t dio;  
} Led_Map;

//  LED STRING1 is at ledMap index zero not one.
static const Led_Map ledMap[LEDSTRING_COUNT] = {
    [0] = { .dio=3 }, // LED1 is controled with Digital IO3
    [1] = { .dio=10 }, // LED2 is controled with Digital IO10
    [2] = { .dio=11 }, // LED3 is controled with Digital IO11
    [3] = { .dio=12 }, // LED4 is controled with Digital IO12
};

static uint8_t boostInUse; // 0 if free, 1 thru LEDSTRING_COUNT and is the solenoid using boost
static uint8_t flowInUse; // 0 only one solenoid can be active so its flow count can be measured

uint8_t uint8_from_arg0 (void)
{
    // check that arg[0] is a digit 
    if ( ( !( isdigit(arg[0][0]) ) ) )
    {
        printf_P(PSTR("{\"err\":\"%sArg0_NaN\"}\r\n"),command[1]);
        return 0;
    }
    uint8_t arg0 = atoi(arg[0]);
    if ( ( arg0 < 1) || (arg0 > LEDSTRING_COUNT) )
    {
        printf_P(PSTR("{\"err\":\"%sArg0_OutOfRng\"}\r\n"),command[1]);
        return 0;
    }
    return arg0;
}

unsigned long ul_from_arg1 (unsigned long max)
{
    // check that arg[1] is a digit 
    if ( ( !( isdigit(arg[1][0]) ) ) )
    {
        printf_P(PSTR("{\"err\":\"%s1NaN\"}\r\n"),command[1]);
        return 0;
    }
    unsigned long ul = strtoul(arg[1], (char **)NULL, 10);
    if ( ( ul < 1) || (ul > max) )
    {
        printf_P(PSTR("{\"err\":\"%s1OtOfRng\"}\r\n"),command[1]);
        return 0;
    }
    return ul;
}

// arg[0] is led, arg[1] is delay_start
void DelayStart(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_6HR 
        unsigned long delay_start = ul_from_arg1(SEC_IN_6HR);
        if (! delay_start)
        {
            initCommandBuffer();
            return;
        }
        // don't change a LED that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].delay_start_sec = delay_start;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\""),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"DlyStCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is runtime
void RunTime(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_6HR 
        unsigned long runtime = ul_from_arg1(SEC_IN_6HR);
        if (! runtime)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].runtime_sec = runtime;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"RunTmCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is delay
void Delay(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_DAY 
        unsigned long delay = ul_from_arg1(SEC_IN_DAY);
        if (! delay)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].delay_sec = delay;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\""),(led[led_arg0-1].delay_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"DelayCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is mahr_stop
void FlowStop(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..MAHR_NOT_SET 
        unsigned long mahr_stop = ul_from_arg1(MAHR_NOT_SET);
        if (! mahr_stop)
        {
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].mahr_stop = mahr_stop;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"mahr_stop\":\"%lu\","),(led[led_arg0-1].mahr_stop));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"FlwStpCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is solenoid, [arg[1] is cycles]
void Run(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        
        uint8_t cycles = led[led_arg0-1].cycles;
        if (arg[1]!=NULL)
        {
            // and arg[1] value is 1..0xFF 
            cycles = (uint8_t) (ul_from_arg1(0xFF));
            if (! cycles)
            {
                initCommandBuffer();
                return;
            }
        }

         // don't run a solenoid that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].cycle_state = 1;
        led[led_arg0-1].cycles = cycles;
        led[led_arg0-1].flow_cnt_bank = 0;
        led[led_arg0-1].cycle_millis_bank = 0;
        led[led_arg0-1].started_at = millis(); //delay_start_sec is timed from now
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(led[led_arg0-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\""),(led[led_arg0-1].cycles));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
        {
            printf_P(PSTR(",\"mahr_stop\":\"%lu\""),(led[led_arg0-1].mahr_stop));
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
        printf_P(PSTR("{\"err\":\"RunCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is cycles
void Save(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..0xFF 
        unsigned long cycles = ul_from_arg1(0xFF);
        if (! cycles)
        {
            initCommandBuffer();
            return;
        }
         // don't save a solenoid that is in use
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        if ( eeprom_is_ready() )
        {
            led[led_arg0-1].cycle_state = 0;
            led[led_arg0-1].cycles = (uint8_t)cycles;
            led[led_arg0-1].flow_cnt_bank = 0;
            led[led_arg0-1].cycle_millis_bank = 0;
            uint16_t value = ((uint16_t) (led_arg0)) + 0x4B30; //ascii bytes for 'K1', 'K2'...
            eeprom_write_word( (uint16_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_ID), value);
            printf_P(PSTR("{\"LED%d\":{"),led_arg0);
            command_done = 11;
        }
    }
    else if ( (command_done == 11) )
    {  
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].delay_start_sec;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DLY_STRT), value);
            printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(value));
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].runtime_sec;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_RUNTIME), value);
            printf_P(PSTR("\"runtime_sec\":\"%lu\","),(value));
            command_done = 13;
        }
    }
    else if ( (command_done == 13) )
    {  
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].delay_sec;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DELAY), value);
            printf_P(PSTR("\"delay_sec\":\"%lu\","),(value));
            command_done = 14;
        }
    }
    else if ( (command_done == 14) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].mahr_stop;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_MAHR_STP), value);
            if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
            {
                printf_P(PSTR("\"mahr_stop\":\"%lu\","),(value));
            }
            command_done = 15;
        }
    }
    else if ( (command_done == 15) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint8_t value = led[led_arg0-1].cycles;
            eeprom_write_byte( (uint8_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_CYCLES), value);
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
        printf_P(PSTR("{\"err\":\"SavCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is solenoid
void Load(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }

         // don't load a solenoid that is in use
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        if ( eeprom_is_ready() )
        {
            if (LoadLedControlFromEEPROM(led_arg0))
            {
                led[led_arg0-1].flow_cnt_bank = 0;
                led[led_arg0-1].cycle_millis_bank = 0;
                printf_P(PSTR("{\"LED%d\":{"),led_arg0);
                command_done = 11;
            }
            else
            {
                printf_P(PSTR("{\"err\":\"LdFailLed%dnoEEP\"}\r\n"),led_arg0);
                initCommandBuffer();
                return;
            }
        }
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(led[led_arg0-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
        {
            printf_P(PSTR("\"mahr_stop\":\"%lu\","),(led[led_arg0-1].mahr_stop));
        }
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\""),(led[led_arg0-1].cycles));
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"LdCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led
void Time(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycle_state\":\"%d\","),(led[led_arg0-1].cycle_state));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%d\","),(led[led_arg0-1].cycles));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycle_millis\":\"%lu\""),(led[led_arg0-1].cycle_millis_bank));
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
void Stop(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].cycle_state)
        {
            led[led_arg0-1].delay_start_sec = 1;
            led[led_arg0-1].runtime_sec = 1; 
            led[led_arg0-1].delay_sec = 1;
            led[led_arg0-1].mahr_stop = MAHR_NOT_SET;
            led[led_arg0-1].cycles = 1;
            printf_P(PSTR("\"stop_time_sec\":\"3\""));
        }
        else
        {
            printf_P(PSTR("\"stop_time_sec\":\"0\""));
        }
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"StpCmdDnWTF\"}\r\n"));
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
    1 = active, wait for start time (delay_start_sec), then start a Boost charge and setup to measure flow.
    2 = wait for BOOST_TIME to finish, then set solenoid.
    3 = wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
    4 = wait for runTime, then select state 7. or if within 16M counts of mahr_stop select state 5. 
    5 = Not needed: wait for boostInUse, then start a Boost charge
    6 = wait for mahr_stop, then reset solenoid and measure flow and slect state 9.
    7 = Not needed: wait for boostInUse, then start a Boost charge
    8 = wait for BOOST_TIME to finish, then reset solenoid and measure flow rate.
    9 = wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
    10 = wait for SOLENOID_CLOSE time, then measure flow count, if cycles is set then state 0.
    11 = wait for delay time, then loop to cycle_state = 1 (backdate so delay_start_sec is not used in each loop).
*/ 
void SolenoidControl() {
    for(int i = 0; i < LEDSTRING_COUNT; i++){
        // active, wait for start time (delay_start_sec), then setup to measure mAHr.
        if ((led[i].cycle_state == SOLENOID_STATE_PRESTART) && !flowInUse) 
        {
            unsigned long kRuntime= millis() - led[i].started_at;
            if ((kRuntime) > ((unsigned long)led[i].delay_start_sec * 1000)) 
            {
                reset_solenoid(BOOST);
                led[i].cycle_state = SOLENOID_STATE_BOOST_BEFOR_SET;
                led[i].started_at = millis(); // boost started
                flowInUse = i+1; //allow this solenoid to use flow meter
                break;
            }
        }
        if (flowInUse == i+1) // only let the solenoid that has been allocated use of the flow meter run
        {
            // wait for BOOST_TIME to finish, then select solenoid control line to drive.
            if ((led[i].cycle_state == SOLENOID_STATE_BOOST_BEFOR_SET)) 
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if ((kRuntime) > ((unsigned long)BOOST_TIME)) 
                {
                    led[i].flow_cnt_start =  icp1.count;
                    set_solenoid(i+1);
                    led[i].started_at = millis(); //start time to wait for H-bridge to power a SET
                    led[i].cycle_state = SOLENOID_STATE_SET_HBRIDGE;
                    break;
                }
            }

            // wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
            if ((led[i].cycle_state == SOLENOID_STATE_SET_HBRIDGE)) 
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if ((kRuntime) > ((unsigned long)PWR_HBRIDGE)) 
                {
                    bridge_off();
                    led[i].started_at = millis(); //start runtime e.g. solenoid is set
                    led[i].cycle_millis_start = millis(); 
                    led[i].flow_cnt_start =  icp1.count;
                    led[i].cycle_state = SOLENOID_STATE_RUNTIME;
                    break;
                }
            }

            // wait for runTime, then select STATE_RUNDONE_BOOST_BEFOR_RESET. or if not MAHR_NOT_SET counts select state STATE_FLOWDONE_BOOST_BEFOR_RESET.
            if (led[i].cycle_state == SOLENOID_STATE_RUNTIME) 
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if ((kRuntime) > ((unsigned long)led[i].runtime_sec * 1000)-1) 
                {
                    led[i].cycle_millis_stop = millis(); // correction of -1 millis was added so timer shows expected value
                    led[i].cycle_state = SOLENOID_STATE_RUNDONE_BOOST_BEFOR_RESET;
                }
                if (led[i].mahr_stop != MAHR_NOT_SET) 
                {  
                    if ( (icp1.count - led[i].flow_cnt_start) >= led[i].mahr_stop) 
                    {
                        led[i].cycle_state = SOLENOID_STATE_FLOWDONE_BOOST_BEFOR_RESET;
                        break;
                    }
                }
            }

            // wait for not boostInUse, then start a Boost charge
            if ((led[i].cycle_state == SOLENOID_STATE_RUNDONE_BOOST_BEFOR_RESET) && !boostInUse) 
            {
                reset_solenoid(BOOST);
                led[i].cycle_state = SOLENOID_STATE_RUNDONE_RESET;
                led[i].started_at = millis(); // boost started
                boostInUse = i+1;
                break;
            }

            // wait for BOOST_TIME to finish, then reset solenoid and measure flow.
            if ((led[i].cycle_state == SOLENOID_STATE_RUNDONE_RESET) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if (((kRuntime) > ((unsigned long)BOOST_TIME)) ) 
                {
                    reset_solenoid(i+1);
                    led[i].started_at = millis(); //start time that H-bridge RESET has power
                    led[i].cycle_state = SOLENOID_STATE_RESET_HBRIDGE;
                    break;
                }
            }

            // wait for not boostInUse, then start a Boost charge
            if ((led[i].cycle_state == SOLENOID_STATE_FLOWDONE_BOOST_BEFOR_RESET) && !boostInUse)
            {
                reset_solenoid(BOOST);
                led[i].cycle_state = SOLENOID_STATE_FLOWDONE_RESET;
                led[i].started_at = millis(); // boost started
                boostInUse = i+1;
                break;
            }

            // wait for BOOST_TIME to finish, then reset solenoid and measure flow.
            if ((led[i].cycle_state == SOLENOID_STATE_FLOWDONE_RESET) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if ((kRuntime) > ((unsigned long)BOOST_TIME)) 
                {
                    reset_solenoid(i+1);
                    led[i].started_at = millis(); //start time that H-bridge RESET has power
                    led[i].cycle_state = SOLENOID_STATE_RESET_HBRIDGE;
                    break;
                }
            }
            
            // wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
            if ((led[i].cycle_state == SOLENOID_STATE_RESET_HBRIDGE) && (boostInUse == i+1))
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if ((kRuntime) > ((unsigned long)PWR_HBRIDGE)) 
                {
                    boostInUse = 0;
                    bridge_off();
                    led[i].cycle_millis_bank += (led[i].cycle_millis_stop - led[i].cycle_millis_start);
                    led[i].started_at = millis(); 
                    led[i].cycle_state = SOLENOID_STATE_FLOWCOUNT;
                    break;
                }
            }
              
            // wait for SOLENOID_CLOSE time, then measure flow count.
            if ((led[i].cycle_state == SOLENOID_STATE_FLOWCOUNT)) 
            {
                unsigned long kRuntime= millis() - led[i].started_at;
                if ((kRuntime) > ((unsigned long)SOLENOID_CLOSE)) 
                {
                    flowInUse = 0; // this will allow the next solenoid cycle_state to progress 
                    led[i].flow_cnt_stop = icp1.count; //record the flow meter pulse count after solenoid has closed
                    led[i].flow_cnt_bank += (led[i].flow_cnt_stop - led[i].flow_cnt_start);
                    if (led[i].cycles)
                    {
                        led[i].cycles = led[i].cycles -1;
                    }
                    if (led[i].cycles)
                    {
                        led[i].started_at = millis();
                        led[i].cycle_state = SOLENOID_STATE_DELAY;
                        break;
                    }
                    else
                    {
                        led[i].started_at = 0;
                        led[i].cycle_state = SOLENOID_STATE_NOT_ACTIVE;
                        break;
                    }
                }
            }
        }
        
        // delay after reset operation and befor next set operation
        if ((led[i].cycle_state == SOLENOID_STATE_DELAY)) 
        {
            unsigned long kRuntime= millis() - led[i].started_at;
            if ( (kRuntime) > ((unsigned long)led[i].delay_sec*1000UL) ) 
            {
                reset_solenoid(BOOST);
                led[i].cycle_state = SOLENOID_STATE_BOOST_BEFOR_SET;
                led[i].started_at = millis(); // boost started
                boostInUse = i+1; //allow this solenoid to use boost
                flowInUse = i+1; //allow this solenoid to use flow meter
                break;
            }
        }
    }
}

// warning this sets all LED for 1 Second operation, 1 cycle, and sets cycle_state so they will run
void Reset_All_LED() {
    for(uint8_t i = 0; i < LEDSTRING_COUNT; i++)
    {
        led[i].started_at = millis(); // used to delay start
        led[i].delay_start_sec = i+i+i+1; // every 3 sec starting at 1, then 4, and 7
        led[i].runtime_sec = 1; 
        led[i].delay_sec = SEC_IN_HR;
        led[i].mahr_stop = MAHR_NOT_SET;
        led[i].cycles = 1;
        led[i].cycle_state = 1;
        led[i].flow_cnt_bank = 0;
        led[i].cycle_millis_bank = 0;
    }
}

uint8_t LoadLedControlFromEEPROM(uint8_t solenoid) 
{
    uint16_t i = solenoid-1;
    if (!led[i].cycle_state)
    {
        uint16_t id = eeprom_read_word((uint16_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_ID));
        if (id == (i+0x4B31) ) // 'K' is 0x4B and '1' is 0x31, thus K1, K2...
        {
            led[i].started_at = millis(); // delay start is from now.
            led[i].delay_start_sec =eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DLY_STRT)); 
            led[i].runtime_sec = eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_RUNTIME));  
            led[i].delay_sec = eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DELAY)); 
            led[i].mahr_stop = eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_MAHR_STP)); 
            led[i].cycles = eeprom_read_byte((uint8_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_CYCLES)); 
            led[i].flow_cnt_bank = 0;
            led[i].cycle_millis_bank = 0;
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
uint8_t Live(uint8_t solenoid) 
{
    uint16_t i = solenoid-1;
    if (i<LEDSTRING_COUNT)
    {
        return led[i].cycle_state;
    }
    else return 0xFF; // not a valid solenoid 
}

// start the LED if it is not running and return cycle state
uint8_t StartLed(uint8_t solenoid) 
{
    uint16_t i = solenoid-1;
    if (i<LEDSTRING_COUNT )
    {
        if (! led[i].cycle_state) led[i].cycle_state = 1;
        return led[i].cycle_state;
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

