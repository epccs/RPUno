/*
ee is part of Eeprom, it is for handling EEPROM with parse arguments from an interactive command line, 
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
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "ee.h"

#if defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168__)
#define EEPROM_SIZE 512
#endif

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
#define EEPROM_SIZE 1024
#endif

#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define EEPROM_SIZE 4096
#endif

#ifndef EEPROM_SIZE
#   error your mcu is not supported
#endif

static int ee_mem;

/* /0/ee? 0..1023 */
void EEread(void)
{
    if ( (command_done == 10) )
    {
        // check that argument[0] is in the range 0..1023
        if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 0) || (atoi(arg[0]) >= EEPROM_SIZE) )
        {
            printf_P(PSTR("{\"err\":\"EeAddrSize %d\"}\r\n"), EEPROM_SIZE);
            initCommandBuffer();
            return;
        }
        
        printf_P(PSTR("{\"EE[%s]\",:"),arg[0]);
        ee_mem = -1;
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  //  checking if we can use eeprom, or just loop. There is a delay after a write.
        if ( eeprom_is_ready() ) 
        {
            ee_mem =(int) (eeprom_read_byte( (uint8_t *) (atoi(arg[0])) )  );
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("\"%d\"}\r\n"),ee_mem);
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"AdcCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

/* /0/ee 0..1023,0..255 */
void EEwrite(void)
{
    if ( (command_done == 10) )
    {
        // check that argument[0] is in the range 0..1023
        if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 0) || (atoi(arg[0]) >= EEPROM_SIZE) )
        {
            printf_P(PSTR("{\"err\":\"EeAddrSize %d\"}\r\n"), EEPROM_SIZE);
            initCommandBuffer();
            return;
        }
        
        // check that argument[1] is in the range 0..255
        if ( ( !( isdigit(arg[1][0]) ) ) || (atoi(arg[1]) < 0) || (atoi(arg[1]) > 255) )
        {
            printf_P(PSTR("{\"err\":\"EeData!=uint8_t\"}\r\n"));
            initCommandBuffer();
            return;
        }
        
        printf_P(PSTR("{\"EE[%s]\",:"),arg[0]);
        ee_mem = -1;
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  //  checking if we can use eeprom, or just loop.
        if ( eeprom_is_ready() ) 
        {
            eeprom_write_byte( (uint8_t *) (atoi(arg[0])), ((uint8_t) ((atoi(arg[1]) & 0xFF)) ) );
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {  //  keep checking if we can use eeprom, there is be a delay after a write
        if ( eeprom_is_ready() ) 
        {
            ee_mem =(int) (eeprom_read_byte( (uint8_t *) (atoi(arg[0])) )  );
            command_done = 13;
        }
    }
    else if ( (command_done == 13) )
    {
        printf_P(PSTR("\"%d\"}\r\n"),ee_mem);
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"AdcCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

