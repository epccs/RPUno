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
#define E3 3
#define A0 10
#define A1 11
#define A2 12
// note 13 is used to blink I2C status in main()

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
    unsigned long delay_start; // delay befor first set operation (one time)
    unsigned long runtime_sec; // delay after set operation and befor reset
    unsigned long delay_sec;   // delay after reset operation and befor next set operation if (cycles > 0)
    uint8_t cycle_state;
    uint8_t cycles; // keep cycling until zero
    uint32_t flow_stop; // pulse counts after start at which to reset the solenoid
    uint32_t flow_cnt_start; // pulse count when set occured (e.g. ICP1 capture events)
    uint32_t flow_cnt_stop; // pulse count when reset occured
    // uint8_t use_flow_meter; // the defaut is for all solenoids to use the flow meter, but what if I want to inject fertilizers into flow
}  solenoidTimer;

static solenoidTimer k[SOLENOID_COUNT];

static uint8_t boostInUse; // 0 if free, 1 thru SOLENOID_COUNT and is the solenoid using boost
static uint8_t flowInUse; // 0 only one solenoid can be active so its flow count can be measured

// arg[0] is solenoid, arg[1] is runtime
void RunTime(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"RunTmKNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 1..3 
        uint8_t k_solenoid = atoi(arg[0]);
        if ( ( k_solenoid < 1) || (k_solenoid > 3) )
        {
            printf_P(PSTR("{\"err\":\"RunTmKOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // check that arg[1] is a digit 
        if ( ( !( isdigit(arg[1][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"RunTmNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 0..21600 
        unsigned long runtime = atol(arg[1]);
        if ( ( runtime < 0) || (runtime > SEC_IN_6HR) )
        {
            printf_P(PSTR("{\"err\":\"RunTmOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"RunTmInUse\"}\r\n"));
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].delay_start = 0;
        k[k_solenoid-1].runtime_sec = runtime;
        k[k_solenoid-1].delay_sec = SEC_IN_HR;
        k[k_solenoid-1].flow_stop = FLOW_NOT_SET;
        k[k_solenoid-1].cycles = 1;
        printf_P(PSTR("{\"K%d\":{\"day\":\"true\","),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"runtime_Sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 12;
    }
    else if ( (command_done == 13) )
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

// arg[0] is solenoid, arg[1] is delay
void Delay(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"DlyKNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 1..3 
        uint8_t k_solenoid = atoi(arg[0]);
        if ( ( k_solenoid < 1) || (k_solenoid > 3) )
        {
            printf_P(PSTR("{\"err\":\"DlyKOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // check that arg[1] is a digit 
        if ( ( !( isdigit(arg[1][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"DlyNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 0..86400 
        unsigned long delay = atol(arg[1]);
        if ( ( delay < 0) || (delay > SEC_IN_DAY) )
        {
            printf_P(PSTR("{\"err\":\"DlyOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // don't change a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"DlyInUse\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // do not change delay_start 
        // do not change runtime
        k[k_solenoid-1].delay_sec = delay;
        // do not change flow_stop
        // do not change cycles ;
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_Sec\":\"%lu\""),(k[k_solenoid-1].delay_sec));
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

// arg[0] is solenoid
void Run(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"RunKNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 1..3 
        uint8_t k_solenoid = atoi(arg[0]);
        if ( ( k_solenoid < 1) || (k_solenoid > 3) )
        {
            printf_P(PSTR("{\"err\":\"RunKOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // don't run a solenoid that is in use it needs to be stopped first
        if (k[k_solenoid-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"RunInUse\"}\r\n"));
            initCommandBuffer();
            return;
        }
        k[k_solenoid-1].cycle_state = 1;
        k[k_solenoid-1].started_at = millis(); //delay_start is timed from now
        printf_P(PSTR("{\"K%d\":{"),k_solenoid);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_Sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"runtime_Sec\":\"%lu\","),(k[k_solenoid-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t k_solenoid = atoi(arg[0]);
        printf_P(PSTR("\"delay_Sec\":\"%lu\","),(k[k_solenoid-1].delay_sec));
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
        printf_P(PSTR("{\"err\":\"RunCmdDnWTF\"}\r\n"));
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

/* operate the solenoid states without blocking
    cycle_state 
    0 = solenoid not active
    1 = active, wait for start time (delay_start), then start a Boost charge and setup to measure flow.
    2 = wait for BOOST_TIME to finish, then set solenoid.
    3 = wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
    4 = wait for runTime, then select state 7. or if within 16M counts of flow_stop select state 5. 
    5 = wait for boostInUse, then start a Boost charge
    6 = wait for flow_stop, then reset solenoid and measure flow and slect state 9.
    7 = wait for boostInUse, then start a Boost charge
    8 = wait for BOOST_TIME to finish, then reset solenoid and measure flow rate.
    9 = wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
    10 = wait for SOLENOID_CLOSE time, then measure flow count, if cycles is set then state 0.
    11 = wait for delay time, then loop to cycle_state = 1 (backdate so delay_start is not used in each loop).
*/ 
void SolenoidControl() {
    for(int i = 0; i < SOLENOID_COUNT; i++){
        // active, wait for start time (delay_start), then start a Boost charge and setup to measure pulse count.
        if ((k[i].cycle_state == 1) && !boostInUse && !flowInUse) 
        {
            unsigned long kRuntime= millis() - k[i].started_at;
            if ((kRuntime) > ((unsigned long)k[i].delay_start * 1000)) 
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = 2;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1; //allow this solenoid to use boost
                flowInUse = i+1; //allow this solenoid to use flow meter
                k[i].flow_cnt_start = icp1.count; //pulse count at start of run
                break;
            }
        }
        if (flowInUse == i+1) // only let the solenoid that has been allocated use of the flow meter run
        {
            // wait for BOOST_TIME to finish, then select solenoid control line to drive.
            if ((k[i].cycle_state == 2) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)BOOST_TIME)) 
                {
                    set_solenoid(i+1);
                    k[i].started_at = millis(); //start H-bridge SET has power
                    k[i].cycle_state = 3;
                    break;
                }
            }

            // wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
            if ((k[i].cycle_state == 3) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)PWR_HBRIDGE)) 
                {
                    boostInUse = 0;
                    bridge_off();
                    k[i].started_at = millis(); //start runtime e.g. solenoid is set
                    k[i].cycle_state = 4;
                    break;
                }
            }

            // wait for runTime, then select state 7. or if not FLOW_NOT_SET counts select state 5.
            if (k[i].cycle_state == 4) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)k[i].runtime_sec * 1000)) 
                {
                    k[i].cycle_state = 7;
                }
                if (k[i].flow_stop != FLOW_NOT_SET) 
                {  
                    if ( (icp1.count - k[i].flow_cnt_start) >= k[i].flow_stop) 
                    { // need to stop now
                        k[i].cycle_state = 5;
                        break;
                    }
                }
            }

            // wait for not boostInUse, then start a Boost charge
            if ((k[i].cycle_state == 5) && !boostInUse) 
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = 6;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1;
                break;
            }

            // wait for boost charge, then reset solenoid, measure flow  and select state 9.
            if ((k[i].cycle_state == 6) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if (((kRuntime) > ((unsigned long)BOOST_TIME)) ) 
                {
                    reset_solenoid(i+1);
                    k[i].started_at = millis(); //start H-bridge SET has power
                    k[i].cycle_state = 9;
                    break;
                }
            }

            // wait for not boostInUse, then start a Boost charge
            if ((k[i].cycle_state == 7) && !boostInUse)
            {
                reset_solenoid(BOOST);
                k[i].cycle_state = 8;
                k[i].started_at = millis(); // boost started
                boostInUse = i+1;
                break;
            }

            // wait for BOOST_TIME to finish, then reset solenoid and measure flow rate.
            if ((k[i].cycle_state == 8) && (boostInUse == i+1)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)BOOST_TIME)) 
                {
                    reset_solenoid(i+1);
                    k[i].started_at = millis(); //start H-bridge SET has power
                    k[i].cycle_state = 9;
                    break;
                }
            }
            
            // wait for PWR_HBRIDGE time to drive solenoid, then trun off H-bridge.
            if ((k[i].cycle_state == 9) && (boostInUse == i+1))
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)PWR_HBRIDGE)) 
                {
                    boostInUse = 0;
                    bridge_off();
                    k[i].started_at = millis(); // Solenoid may need some time to finish closing befor recording the flow count
                    k[i].cycle_state = 10;
                    break;
                }
            }
              
            // wait for SOLENOID_CLOSE time, then measure flow count.
            if ((k[i].cycle_state == 10)) 
            {
                unsigned long kRuntime= millis() - k[i].started_at;
                if ((kRuntime) > ((unsigned long)SOLENOID_CLOSE)) 
                {
                    flowInUse = 0; // this will allow the next solenoid cycle_state to progress 
                    k[i].flow_cnt_stop = icp1.count; //record the flow meter pulse count after solenoid has closed
                    if (k[i].cycles)
                    {
                        k[i].started_at = millis();
                        k[i].cycle_state = 11;
                        k[i].cycles = k[i].cycles -1;
                        break;
                    }
                    else
                    {
                        k[i].started_at = 0;
                        k[i].cycle_state = 0;
                        break;
                    }
                }
            }
        }
        
        // delay after reset operation and befor next set operation
        if ((k[i].cycle_state == 11)) 
        {
            unsigned long kRuntime= millis() - k[i].started_at;
            if ( (kRuntime) > ((unsigned long)k[i].delay_sec*1000UL) ) 
            {
                // back date by delay_start since it should only be used once
                k[i].started_at = millis()-((unsigned long)k[i].delay_start*1000UL);
                k[i].cycle_state = 1;
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
        k[i].delay_start = i+i+i+1; // every 3 sec starting at 1, then 4, and 7
        k[i].runtime_sec = 1; 
        k[i].delay_sec = 0;
        k[i].flow_stop = FLOW_NOT_SET;
        k[i].cycles = 1;
        k[i].cycle_state = 1; 
    }
}

// return the solenoid cycle_state (e.g. it is running)
uint8_t Live(uint8_t i) 
{
    if (i && (i<=SOLENOID_COUNT))
    {
        return k[i-1].cycle_state;
    }
    else
        return 0xFF; // not a valid solenoid
}

// only use init at setup() not durring loop()
void init_K(void) {
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