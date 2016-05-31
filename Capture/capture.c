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

#define SERIAL_PRINT_DELAY_MILSEC 1000
static unsigned long serial_print_started_at;

static uint32_t low;
static uint32_t high;

// copy of the event buffer for serial output
static uint8_t cpy_event_Byt0[EVENT_BUFF_SIZE];
static uint8_t cpy_event_Byt1[EVENT_BUFF_SIZE];
static uint8_t cpy_event_Byt2[EVENT_BUFF_SIZE];
static uint8_t cpy_event_Byt3[EVENT_BUFF_SIZE];
static uint8_t cpy_event_BytChk[EVENT_BUFF_SIZE];
static uint8_t cpy_event_rising[EVENT_BUFF_SIZE];
static uint8_t cpy_icp1_head;
static uint32_t cpy_icp1_event_count;

static int event_pair;
static int event_pair_output;

static int event;
static int event_output;
static uint8_t low2high;

/* return ICP1 timer count for both low and high signal timing.
    Low value is the counts between a falling edge and a rising edge.
    High value is the counts between a rising edge and falling edge.
    The ratio may be used to determine the duty.*/
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
            printf_P(PSTR("{\"err\":\"IcpArgCnt@%d!=0|2\"}\r\n"),arg_count);
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
                uint8_t num_of_events_needed=((event_pair * 2) +1);
                
                // copy at least enough events for the report
                // cast to integers allows use of two's-complement math
                cpy_icp1_head =(uint8_t)( (int8_t)(icp1_head) - (int8_t)(num_of_events_needed+1) ) & (EVENT_BUFF_MASK);
                
                // now copy the event buffer, icp1_head may advance but when we catch up to it the copy is done
                {
                    uint8_t i=0;
                    while ((cpy_icp1_head != icp1_head) | (i < num_of_events_needed) ) 
                    {
                        cpy_icp1_head = (cpy_icp1_head +1 ) & (EVENT_BUFF_MASK);
                        cpy_event_Byt0[cpy_icp1_head] = event_Byt0[cpy_icp1_head];
                        cpy_event_Byt1[cpy_icp1_head] = event_Byt1[cpy_icp1_head];
                        cpy_event_Byt2[cpy_icp1_head] = event_Byt2[cpy_icp1_head];
                        cpy_event_Byt3[cpy_icp1_head] = event_Byt3[cpy_icp1_head];
                        cpy_event_BytChk[cpy_icp1_head] = (cpy_event_Byt3[cpy_icp1_head]^ cpy_event_Byt2[cpy_icp1_head]^cpy_event_Byt1[cpy_icp1_head]^cpy_event_Byt0[cpy_icp1_head]);
                        cpy_event_rising[cpy_icp1_head] = event_rising[cpy_icp1_head];
                        cpy_icp1_event_count = icp1_event_count;
                        if (i < num_of_events_needed) i++;
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

        // check if a byte has changed
        // somthing is going out of bounds and messing up the buffer copy
        if ( (cpy_event_Byt3[high2low_event_index]^ cpy_event_Byt2[high2low_event_index]^cpy_event_Byt1[high2low_event_index]^cpy_event_Byt0[high2low_event_index]) != cpy_event_BytChk[high2low_event_index])
        {
            printf_P(PSTR("{\"err\":\"IcpChkByt@[%d]\"}\r\n"),high2low_event_index);
            initCommandBuffer();  
            return;
        }
        if ( (cpy_event_Byt3[low2high_event_index]^ cpy_event_Byt2[low2high_event_index]^cpy_event_Byt1[low2high_event_index]^cpy_event_Byt0[low2high_event_index]) != cpy_event_BytChk[low2high_event_index])
        {
            printf_P(PSTR("{\"err\":\"IcpChkByt@[%d]\"}\r\n"),low2high_event_index);
            initCommandBuffer();  
            return;
        }
        if ( (cpy_event_Byt3[period_event_index]^ cpy_event_Byt2[period_event_index]^cpy_event_Byt1[period_event_index]^cpy_event_Byt0[period_event_index]) != cpy_event_BytChk[period_event_index])
        {
            printf_P(PSTR("{\"err\":\"IcpChkByt@[%d]\"}\r\n"),period_event_index);
            initCommandBuffer();  
            return;
        }
        
        // Now find counts between events while ICP1 was low and then when it was high
        // cast to int causes two's complement math to be used which gives correct result through a roll over
        low = (uint32_t)((int32_t)high2low_event - (int32_t)low2high_event);
        high = (uint32_t)((int32_t)low2high_event - (int32_t)period_event);
        
        // if the last capture was on a falling event, swap low and high count
        if (cpy_event_rising[high2low_event_index] == 0)
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
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
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

/* return ICP1 timer count for each event and the timer value.
    Event value is an unsigned 32 bit count at the time of the event.*/
void Event(void)
{
    if ( (command_done == 10) )
    {
        if (arg_count == 0)
        {
            event = 1;
        }
        else if (arg_count == 2)
        { // arg[0] should say icp1, other mcu's may have icp3...
            event = atoi(arg[1]);
        }
        else
        {
            printf_P(PSTR("{\"err\":\"IcpArgCnt@%d!=0|2\"}\r\n"),arg_count);
            initCommandBuffer();   
        }

        if ((event < 1) || (event >= (EVENT_BUFF_SIZE)) )
        {
            printf_P(PSTR("{\"err\":\"IcpMaxEvents@%d\"}\r\n"),(EVENT_BUFF_SIZE-1));
            initCommandBuffer();
        }
        else
        {
            // event_pair should have a valid value for how many high-low capture pairs to print, but I need a counter to track up to it.
            event_output =0;
            
            // buffer has enough readings
            if (icp1_event_count > event) 
            { 
                // copy at least enough events for the report
                // cast to integers allows use of two's-complement math
                cpy_icp1_head =(uint8_t)( (int8_t)(icp1_head) - (int8_t)(event+1) ) & (EVENT_BUFF_MASK);
                
                // now copy the event buffer, icp1_head may advance but when we catch up to it the copy is done
                {
                    uint8_t i=0;
                    while ((cpy_icp1_head != icp1_head) | (i < event) ) 
                    {
                        cpy_icp1_head = (cpy_icp1_head +1 ) & (EVENT_BUFF_MASK);
                        cpy_event_Byt0[cpy_icp1_head] = event_Byt0[cpy_icp1_head];
                        cpy_event_Byt1[cpy_icp1_head] = event_Byt1[cpy_icp1_head];
                        cpy_event_Byt2[cpy_icp1_head] = event_Byt2[cpy_icp1_head];
                        cpy_event_Byt3[cpy_icp1_head] = event_Byt3[cpy_icp1_head];
                        cpy_event_rising[cpy_icp1_head] = event_rising[cpy_icp1_head];
                        cpy_icp1_event_count = icp1_event_count;
                        if (i < event) i++;
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
        printf_P(PSTR("{\"icp1\":{\"count\":\"%lu\","),(cpy_icp1_event_count - event_output) );
        command_done = 12;
    }

    else if ( (command_done == 12) )
    {
        // cast to int to use two's complement math then cast back into a uint8_t and mask to the buffer size.
        uint8_t event_index = ( (uint8_t)(((int8_t)cpy_icp1_head) - ((int8_t)(event_output))) ) & (EVENT_BUFF_MASK);

        // the event time is keep in four byte arrays to make the ISR fast and 
        // allow up to 32 events with quick access of the AVR ldd instruction.
        uint32_t uint32_event;
        uint32_event = (((uint32_t)cpy_event_Byt3[event_index]) << 24 ) + (((uint32_t)cpy_event_Byt2[event_index]) << 16 ) + \
                                (((uint32_t)cpy_event_Byt1[event_index]) <<8) + ((uint32_t)cpy_event_Byt0[event_index]);

        low2high = cpy_event_rising[event_index] ;

        // print in  steps otherwise the buffer will fill and block the program from running
        printf_P(PSTR("\"event\":\"%lu\","),uint32_event);
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {
        printf_P(PSTR("\"rising\":\"%d\"}}\r\n"),low2high);
        if ( (++event_output) >= event) 
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
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
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