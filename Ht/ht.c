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
#include <math.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/icp_buf.h"
#include "../lib/icp1.h"
#include "../lib/timers.h"
#include "../Capture/capture.h"
#include "ht.h"

#define SERIAL_PRINT_DELAY_MILSEC 600000UL
static unsigned long serial_print_started_at;

static uint16_t low;
static uint16_t high;
static uint32_t low_sum;
static uint32_t high_sum;
static uint8_t sample_size;

static double sensor_temp;

static int event_pair;
static int event_pair_output;

/* Return RH and Temp using ICP1 capture, also return PV_IN (solar input voltage) and PWR (battery voltage). */
void Ht(void)
{
    // only works if both edges are tracked and prescaler is set to CPU clock.
    if ( (icp1_edge_mode != TRACK_BOTH) || ( (TCCR1B & 0x07) != 0x01) )
    {
        printf_P(PSTR("{\"err\":\"IcpWrongMode\"}\r\n"));
        initCommandBuffer();
        return;
    }
    if ( (command_done == 10) )
    {
        event_pair = 15;

        if ((event_pair < 1) || (event_pair >= (ICP_EVENT_BUFF_SIZE/2)) )
        {
            printf_P(PSTR("{\"err\":\"IcpMaxArg%d\"}\r\n"),(ICP_EVENT_BUFF_SIZE/2)-1);
            initCommandBuffer();
        }
        else
        {
            // event_pair should have a valid value for how many high-low capture pairs to print, but I need a counter to track up to it.
            event_pair_output =0;
            
            // buffer has enough readings
            if (icp1.count > ((event_pair * 2) +1) ) 
            { 
                uint8_t num_of_events_needed=((event_pair * 2) +1);
                
                // copy from icp1 to icp1_db
                double_buffer_copy(&icp1, &icp1_db, num_of_events_needed);
                
                // used to delay serial printing
                serial_print_started_at = millis();
                
                command_done = 11;
            }
            else
            {
                printf_P(PSTR("{\"err\":\"IcpEvntCnt@%d\"}\r\n"), icp1.count);
                initCommandBuffer();
            }
        }
    }
    
    else if ( (command_done == 11) )
    { // use the event_count for indexing the time stamps
        sample_size = 0;
        low_sum = 0;
        high_sum = 0;
        printf_P(PSTR("{\"count\":\"%lu\","),icp1_db.count );
        command_done = 12;
    }

    else if ( (command_done == 12) )
    {
        // cast to int to use two's complement math then cast back into a uint8_t and mask to the buffer size.
        uint8_t high2low_event_index = ((uint8_t)(((int8_t)icp1_db.head) - ((int8_t)2*(event_pair_output)))) & (ICP_EVENT_BUFF_MASK);
        uint8_t low2high_event_index = ((uint8_t)(((int8_t)high2low_event_index) - 1)) & (ICP_EVENT_BUFF_MASK);
        uint8_t period_event_index = ((uint8_t)(((int8_t)low2high_event_index) - 1)) & (ICP_EVENT_BUFF_MASK);

        // the event time is keep in two byte arrays to make the ISR fast and 
        // allow up to 32 events with quick access of the AVR ldd instruction.
        uint16_t high2low_event;
        uint16_t low2high_event;
        uint16_t period_event;
        high2low_event = (((uint16_t)icp1_db.event.Byt1[high2low_event_index]) <<8) + ((uint16_t)icp1_db.event.Byt0[high2low_event_index]);
        low2high_event = (((uint16_t)icp1_db.event.Byt1[low2high_event_index]) <<8) + ((uint16_t)icp1_db.event.Byt0[low2high_event_index]);
        period_event = (((uint16_t)icp1_db.event.Byt1[period_event_index]) <<8) + ((uint16_t)icp1_db.event.Byt0[period_event_index]);

        // Now find counts between events while ICP1 was low and then when it was high
        // cast to int causes two's complement math to be used which gives correct result through a roll over
        low = (uint16_t)((int16_t)high2low_event - (int16_t)low2high_event);
        high = (uint16_t)((int16_t)low2high_event - (int16_t)period_event);
        
        // if the last capture was on a falling event, swap low and high count
        if ( (icp1_db.event.status[high2low_event_index] & (1<<RISING)) == 0)
        { 
            uint16_t temp = low;
            low = high;
            high = temp;
        }
        
        // the high and low timer counts are times between events
        // When HT is hot the high count can be less than 300 which may have some 
        // skipping. This is due to the other ISR's running and not letting the capture  
        // ISR do its job fast enough. A skip will cause the high count to be larger than the low.
        if ( high < low ) // this will not work bellow about -30C or -22F
        {
            sample_size += 1;
            low_sum += low;
            high_sum += high;
        }
        
        if ( (++event_pair_output) >= event_pair) 
        {
            command_done = 13;
        }
        else
        {
            command_done = 12;
        }
    }
    
    else if ( (command_done == 13) )
    {
        printf_P(PSTR("\"smpl_sz\":\"%u\""),(unsigned int)sample_size);
        if (sample_size > 10)
        {
            printf_P(PSTR(","));
            command_done = 14;
        }
        else
        {
            command_done = 24;
        }
        
    }

    else if ( (command_done == 14) )
    {
        // a fixed 1% resistor that discharges the timing capacitor, it does not very with temperature
        float R7 = 1500000.0;
        
        //R9 (an NTC thermistor) is measured from ratio of its charge rate and the R7 discharge rate
        float R9 = R7 * high_sum/low_sum;
        
        // Room Temp in K
        float RoomTemp = 25.0 + 273.15;
        
        // R9 is an NTC with Bata(K) 4582
        float Beta = 4582.0;
        
        //R9 at RoomTemp (the actual value is within 5% of this)
        float R9rt = 470000.0;
        
        //sensor Temp in K
        sensor_temp = ((RoomTemp)*Beta/log(R9rt/R9))/(Beta/log(R9rt/R9)-RoomTemp);
        
        // print in deg F
        printf_P(PSTR("\"deg_f\":\"%1.2f\","),(sensor_temp - 273.15)*9/5 + 32);
        command_done = 15;
    }
    
    else if ( (command_done == 15) )
    {
        // a fixed 1% resistor that discharges the timing capacitor, it does not very with temperature
        float R7 = 1500000.0;
        
        // Room Temp in K
        float RoomTemp = 25.0 + 273.15;
        
        // 555 IC supply
        float V555 = 3.58;
        
        // the voltage over which the ramp time occures, which is 1/3 of the 555 IC supply
        float V = V555/3;
        
        // see Robert A. Pease "What's All This VBE Stuff, Anyhow?"
        float Vbe = 0.675 - (0.00175 *(sensor_temp - RoomTemp) );
        
        // the discharge current is mirrored from a 1.5Meg resistor (it is about 2uA at room temp)
        float i = (V555-Vbe)/R7;
        
        // sensor capacatance
        float C = i * (1.0/F_CPU) * low_sum/(sample_size*V);
        
        // C@55%RH, note I think the circuit adds to this e.g. 5pf or more.
        float  Crt = 180.0*1E-12;
        
        // ratio used for reverse polynomial response of HS1101LF
        float X = C/Crt;
        
        // reverse polynomial response of HS1101LF from datasheet
        float RH = (-3.4656*1E3*X*X*X) + (1.0732*1E4*X*X) + (-1.0457*1E4*X) + (3.2459*1E3);
        
        printf_P(PSTR("\"%%RH\":\"%1.2f\","),RH);
        command_done = 16;
    }
    
    else if ( (command_done == 16) )
    {
        // analog channel 6 is connected to the solar power input (PV_IN)
        float PV_IN = analogRead(6)*(5.0/1023.0)*(532.0/100.0);
        printf_P(PSTR("\"PV_IN\":\"%1.2f\""),PV_IN);
        command_done = 17;
    }

    else if ( (command_done == 17) )
    {
        // analog channel 7 is connected to the battery power (PWR)
        float PWR = analogRead(7)*(5.0/1023.0)*(3.0/2.0);
        printf_P(PSTR("\"PWR\":\"%1.2f\""),PWR);
        command_done = 24;
    }
    
    else if ( (command_done == 24) )
    {
        printf_P(PSTR("}\r\n"));
        command_done = 25;
    }
    
    else if ( (command_done == 25) )
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
