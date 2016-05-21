/*
capture is part of Capture, it returns the ICP1 timing counts for the high and low signal state, 
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
#include "../lib/parse.h"
#include "../lib/icp1.h"
#include "../lib/timers.h"
#include "capture.h"

#define SERIAL_PRINT_DELAY_SEC 10
static unsigned long serial_print_started_at;

static uint32_t low;
static uint32_t high;

// copy of the event buffer for serial output
static uint8_t cpy_event_Byt0[EVENT_BUFF_SIZE];
static uint8_t cpy_event_Byt1[EVENT_BUFF_SIZE];
static uint8_t cpy_event_Byt2[EVENT_BUFF_SIZE];
static uint8_t cpy_event_Byt3[EVENT_BUFF_SIZE];
static uint8_t cpy_event_rising[EVENT_BUFF_SIZE];
static uint8_t cpy_icp1_head;
static uint32_t cpy_icp1_event_count;

static int event_pair;
static int event_pair_output;

/* return ICP1 timer count for both low and high signal timing.
    Low value is the counts between a falling edge and a rising edge.
    High value is the counts between a rising edge and falling edge.
    The ratio may be used to determine the duty.
    No artuments needed*/
void Capture(void)
{
    if ( (command_done == 10) )
    {
        if (arg_count == 0)
        {
            event_pair = 1;
        }
        else if (arg_count == 2)
        { // arg[0] should say icp1, other mcu's may have icp3...
            event_pair = atoi(arg[1]);
        }
        else
        {
            printf_P(PSTR("{\"err\":\"IcpArg_icp1,%s\"}\r\n"),arg[0]);
            initCommandBuffer();   
        }
        
        if ((event_pair < 1) || (event_pair >= (EVENT_BUFF_SIZE/2)) )
        {
            printf_P(PSTR("{\"err\":\"IcpMaxArg%d\"}\r\n"),(EVENT_BUFF_SIZE/2)-1);
            initCommandBuffer();
        }
        else
        {
            // event_pair should have a valid value for how many high-low capture pairs to print, but I need a counter to track up to it.
            event_pair_output =0;
            
            // buffer has enough readings
            if (icp1_event_count > ((event_pair * 2) +1) ) 
            { 
                // the event buffer must not change while a copy is made
                ATOMIC_BLOCK ( ATOMIC_RESTORESTATE )
                {
                    cpy_icp1_head = icp1_head;
                    cpy_icp1_event_count = icp1_event_count;
                    for (uint8_t i=0; i <EVENT_BUFF_SIZE;i++)
                    {
                        cpy_event_Byt0[i] = event_Byt0[i];
                        cpy_event_Byt1[i] = event_Byt1[i];
                        cpy_event_Byt2[i] = event_Byt2[i];
                        cpy_event_Byt3[i] = event_Byt3[i];
                        cpy_event_rising[i] = event_rising[i];
                    }
                }
                
                // used to delay serial printing
                serial_print_started_at = millis();
                
                command_done = 11;
            }
            else
            {
                printf_P(PSTR("{\"err\":\"IcpEvntCnt@%d\"}\r\n"), icp1_event_count);
                initCommandBuffer();
            }
        }
    }
    else if ( (command_done == 11) )
    { // use the event_count for indexing the time stamps
        printf_P(PSTR("{\"icp1\":{\"count\":\"%lu\","),(cpy_icp1_event_count - 2*(event_pair_output) ) );
        command_done = 12;
    }

    else if ( (command_done == 12) )
    {
        // cast to int to use two's complement math then cast back into a uint8_t and mask to the buffer size.
        uint8_t high2low_event_index = ((uint8_t)(((int8_t)cpy_icp1_head) - ((int8_t)2*(event_pair_output)))) & (EVENT_BUFF_MASK);
        uint8_t low2high_event_index = ((uint8_t)(((int8_t)high2low_event_index) - 1)) & (EVENT_BUFF_MASK);
        uint8_t period_event_index = ((uint8_t)(((int8_t)low2high_event_index) - 1)) & (EVENT_BUFF_MASK);

        // the event time is keep in four byte arrays to make the ISR fast and 
        // allow up to 32 events with quick access of the AVR ldd instruction.
        uint32_t high2low_event;
        uint32_t low2high_event;
        uint32_t period_event;
        high2low_event = (((uint32_t)cpy_event_Byt3[high2low_event_index]) << 24 ) + (((uint32_t)cpy_event_Byt2[high2low_event_index]) << 16 ) + \
                                (((uint32_t)cpy_event_Byt1[high2low_event_index]) <<8) + ((uint32_t)cpy_event_Byt0[high2low_event_index]);
        low2high_event = (((uint32_t)cpy_event_Byt3[low2high_event_index]) <<24) + (((uint32_t)cpy_event_Byt2[low2high_event_index]) << 16 ) + \
                                (((uint32_t)cpy_event_Byt1[low2high_event_index]) <<8) + ((uint32_t)cpy_event_Byt0[low2high_event_index]);
        period_event = (((uint32_t)cpy_event_Byt3[period_event_index]) <<24) + (((uint32_t)cpy_event_Byt2[period_event_index]) <<16) + \
                                (((uint32_t)cpy_event_Byt1[period_event_index]) <<8) + ((uint32_t)cpy_event_Byt0[period_event_index]);

        // Now find counts between events while ICP1 was low and then when it was high
        // cast to int causes two's complement math to be used which gives correct result through a roll over
        low = (uint32_t)((int32_t)high2low_event - (int32_t)low2high_event);
        high = (uint32_t)((int32_t)low2high_event - (int32_t)period_event);
        
        // if the last capture was on a falling event, swap low and high count
        if (event_rising[high2low_event_index] == 0)
        { 
            uint32_t temp = low;
            low = high;
            high = temp;
        }
        
        // print in  steps otherwise the buffer will fill and block the program from running
        printf_P(PSTR("\"low\":\"%lu\","),low);
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {
        printf_P(PSTR("\"high\":\"%lu\"}}\r\n"),high);
        if ( (++event_pair_output) >= event_pair) 
        {
            command_done = 14;
        }
        else
        {
            command_done = 11;
        }
    }
    else if ( (command_done == 14) )
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_SEC * 1000))
        {
            command_done = 10; /* This keeps looping the output forever (until a Rx char anyway) */
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"IcpCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

