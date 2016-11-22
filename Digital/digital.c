/*
digital is part of Digital, it is a serial command interface to some digital Wiring like functions, 
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
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "digital.h"

#define SERIAL_PRINT_DELAY_MILSEC 10000
static unsigned long serial_print_started_at;

/* e.g. pinMode( arg[0], arg[1] )
    a = arg[0]
    if ( a < 3) or (a > 12) or ( (a > 4) and (a < 10) ) 
        error
    b = arg[1]
    if not ( (b == "INPUT") or (b == "OUTPUT") )
        error
*/
void Mode(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"pModeNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 3|4|10|11|12 
        uint8_t a = atoi(arg[0]);
        if ( ( a < 3) || (a > 12) || ( (a > 4) && (a < 10) ) )
        {
            printf_P(PSTR("{\"err\":\"pModeOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // also arg[1] is not ('INPUT' or 'OUTPUT')
        if ( !( (strcmp_P( arg[1], PSTR("INPUT")) == 0) || (strcmp_P( arg[1], PSTR("OUTPUT")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"pModeNaMode\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        if (strcmp_P( arg[1], PSTR("OUTPUT")) == 0 ) 
        {
            pinMode(a, OUTPUT);
        }
        else
        {
            pinMode(a, INPUT);
        }
        
        printf_P(PSTR("{\""));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        if (atoi(arg[0]) == 3) //IO3
        {
            printf_P(PSTR("PD3"));
        }
        if (atoi(arg[0]) == 4) //IO4
        {
            printf_P(PSTR("PD4"));
        }
        if (atoi(arg[0]) == 10) //nSS
        {
            printf_P(PSTR("PB2"));
        }
        if (atoi(arg[0]) == 11) //MOSI
        {
            printf_P(PSTR("PB3"));
        }
        if (atoi(arg[0]) == 12) //MISO
        {
            printf_P(PSTR("PB4"));
        }
        if (atoi(arg[0]) == 13) //SCK
        {
            printf_P(PSTR("PB5"));
        }
        printf_P(PSTR("\":\""));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf( arg[1] );
        printf_P(PSTR("\"}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"pModeCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

/* e.g. digitalWrite( arg[0], arg[1] )
    a = arg[0]
    if ( a < 3) or (a > 12) or ( (a > 4) and (a < 10) ) 
        error
    b = arg[1]
    if not ( (b == "HIGH") or (b == "LOW") )
        error
*/
void Write(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"dWrtNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 3|4|10|11|12 
        uint8_t a = atoi(arg[0]);
        if ( ( a < 3) || (a > 12) || ( (a > 4) && (a < 10) ) )
        {
            printf_P(PSTR("{\"err\":\"dWrtOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // also arg[1] is not ('HIGH' or 'LOW')
        if ( !( (strcmp_P( arg[1], PSTR("HIGH")) == 0) || (strcmp_P( arg[1], PSTR("LOW")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"dWrtNaState\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        if (strcmp_P( arg[1], PSTR("HIGH")) == 0 ) 
        {
            digitalWrite(a, HIGH);
        }
        else
        {
            digitalWrite(a, LOW);
        }
        
        printf_P(PSTR("{\""));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        if (atoi(arg[0]) == 3) //IO3
        {
            printf_P(PSTR("PD3"));
        }
        if (atoi(arg[0]) == 4) //IO4
        {
            printf_P(PSTR("PD4"));
        }
        if (atoi(arg[0]) == 10) //nSS
        {
            printf_P(PSTR("PB2"));
        }
        if (atoi(arg[0]) == 11) //MOSI
        {
            printf_P(PSTR("PB3"));
        }
        if (atoi(arg[0]) == 12) //MISO
        {
            printf_P(PSTR("PB4"));
        }
        if (atoi(arg[0]) == 13) //SCK
        {
            printf_P(PSTR("PB5"));
        }
        printf_P(PSTR("\":\""));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t a = atoi(arg[0]);
        if ( (a > (NUM_DIGITAL_PINS-1)) ) // the badPinCheck will barf at compile time without testing the value ... amazing
        {
            return;
        }
        bool pin = digitalRead(a);
        if (pin)
        {
            printf_P(PSTR("HIGH"));
        }
        else
        {
            printf_P(PSTR("LOW"));
        }
        printf_P(PSTR("\"}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"dWrtCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

/* e.g. digitalToggle( arg[0] )
    a = arg[0]
    if ( a < 3) or (a > 12) or ( (a > 4) and (a < 10) ) 
        error
*/
void Toggle(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"dTogNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 3|4|10|11|12 
        uint8_t a = atoi(arg[0]);
        if ( ( a < 3) || (a > 12) || ( (a > 4) && (a < 10) ) )
        {
            printf_P(PSTR("{\"err\":\"dTogOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        digitalToggle(a);
        
        printf_P(PSTR("{\""));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        if (atoi(arg[0]) == 3) //IO3
        {
            printf_P(PSTR("PD3"));
        }
        if (atoi(arg[0]) == 4) //IO4
        {
            printf_P(PSTR("PD4"));
        }
        if (atoi(arg[0]) == 10) //nSS
        {
            printf_P(PSTR("PB2"));
        }
        if (atoi(arg[0]) == 11) //MOSI
        {
            printf_P(PSTR("PB3"));
        }
        if (atoi(arg[0]) == 12) //MISO
        {
            printf_P(PSTR("PB4"));
        }
        if (atoi(arg[0]) == 13) //SCK
        {
            printf_P(PSTR("PB5"));
        }
        printf_P(PSTR("\":\""));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t a = atoi(arg[0]);
        if ( (a > (NUM_DIGITAL_PINS-1)) ) // the badPinCheck will barf at compile time without testing the value ... amazing
        {
            return;
        }
        bool pin = digitalRead(a);
        if (pin)
        {
            printf_P(PSTR("HIGH"));
        }
        else
        {
            printf_P(PSTR("LOW"));
        }
        printf_P(PSTR("\"}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"dTogCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

/* e.g. digitalRead( arg[0] )
    a = arg[0]
    if ( a < 3) or (a > 12) or ( (a > 4) and (a < 10) ) 
        error
*/
void Read(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] is a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"dRdNaN\"}\r\n"));
            initCommandBuffer();
            return;
        }
        // and arg[0] value is 3|4|10|11|12 
        uint8_t a = atoi(arg[0]);
        if ( ( a < 3) || (a > 12) || ( (a > 4) && (a < 10) ) )
        {
            printf_P(PSTR("{\"err\":\"dRdOutOfRng\"}\r\n"));
            initCommandBuffer();
            return;
        }
        serial_print_started_at = millis();
        
        printf_P(PSTR("{\""));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        if (atoi(arg[0]) == 3) //IO3
        {
            printf_P(PSTR("PD3"));
        }
        if (atoi(arg[0]) == 4) //IO4
        {
            printf_P(PSTR("PD4"));
        }
        if (atoi(arg[0]) == 10) //nSS
        {
            printf_P(PSTR("PB2"));
        }
        if (atoi(arg[0]) == 11) //MOSI
        {
            printf_P(PSTR("PB3"));
        }
        if (atoi(arg[0]) == 12) //MISO
        {
            printf_P(PSTR("PB4"));
        }
        if (atoi(arg[0]) == 13) //SCK
        {
            printf_P(PSTR("PB5"));
        }
        printf_P(PSTR("\":\""));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t a = atoi(arg[0]);
        if ( (a > (NUM_DIGITAL_PINS-1)) ) // the badPinCheck will barf at compile time without testing the value ... amazing
        {
            return;
        }
        bool pin = digitalRead(a);
        if (pin)
        {
            printf_P(PSTR("HIGH"));
        }
        else
        {
            printf_P(PSTR("LOW"));
        }
        printf_P(PSTR("\"}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"dRdCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}