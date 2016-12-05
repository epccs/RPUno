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
#include <string.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "ee.h"

static uint32_t ee_mem;

uint8_t ee_read_type(const char * addr, const char * type)
{
    if ( (strlen(type) == 0) || (strcmp_P(type, PSTR("UINT8")) ==0) )
    {
        ee_mem =(uint32_t) read_eeprom_byte(atoi(addr) );
        return 1;
    }
    if ( strcmp_P(type, PSTR("UINT16")) == 0 )
    {
        ee_mem =(uint32_t) read_eeprom_word(atoi(addr) );
        return 1;
    }
    if ( strcmp_P(type, PSTR("UINT32")) == 0 )
    {
        ee_mem =(uint32_t) read_eeprom_dword(atoi(addr) );
        return 1;
    }
    return 0;
}

/* /0/ee? 0..1023, [UINT8|UINT16|UINT32] */
void EEread(void)
{
    if (arg_count > 2)
    {
        printf_P(PSTR("{\"err\":\"EeRdArgCount\"}\r\n"));
        initCommandBuffer();
        return;
    }
    
    if ( (command_done == 10) )
    {
        // check that argument[0] is in the range 0..1023
        if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 0) || (atoi(arg[0]) >= EEPROM_SIZE) )
        {
            printf_P(PSTR("{\"err\":\"EeRdMaxAddr %d\"}\r\n"), EEPROM_SIZE);
            initCommandBuffer();
            return;
        }

        if ( arg_count == 1)
        {
            if (strlen(arg[1]) != 0)
            {
                printf_P(PSTR("{\"err\":\"ParserBroken\"}\r\n"));
                initCommandBuffer();
                return;
            }
        }

       // check that argument[1] is UINT8|UINT16|UINT32
        if ( ( arg_count == 2 ) && ( !( (strcmp_P(arg[1], PSTR("UINT8")) == 0) || \
                                                    (strcmp_P(arg[1], PSTR("UINT16")) == 0) || \
                                                    (strcmp_P(arg[1], PSTR("UINT32")) == 0) ) ) )
        {
            printf_P(PSTR("{\"err\":\"EeRdTypUINT8|16|32\"}\r\n"));
            initCommandBuffer();
            return;
        }
        
        printf_P(PSTR("{\"EE[%s]\":"),arg[0]);
        ee_mem = -1;
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  //  checking if we can use eeprom, or just loop. There is a delay after a write.
        if ( eeprom_is_ready() ) 
        {
            if (!ee_read_type(arg[0], arg[1]))
            {
                printf_P(PSTR("{\"err\":\"EeRdCmdDn11WTF\"}\r\n"));
                initCommandBuffer();
                return;
            }
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("\"%lu\"}\r\n"),ee_mem);
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"AdcCmdDoneWTF\"}\r\n"));
        initCommandBuffer();
    }
}

/* /0/ee address,value[,type] */
void EEwrite(void)
{
    if ( (command_done == 10) )
    {
        // check that argument[0] is in the range 0..EEPROM_SIZE
        if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 0) || (atoi(arg[0]) >= EEPROM_SIZE) )
        {
            printf_P(PSTR("{\"err\":\"EeAddrSize %d\"}\r\n"), EEPROM_SIZE);
            initCommandBuffer();
            return;
        }
        
        // check that argument[1] is a number (it will not overflow a uint32_t)
        if ( !( isdigit(arg[1][0]) ) )
        {
            printf_P(PSTR("{\"err\":\"EeData!=uint8_t\"}\r\n"));
            initCommandBuffer();
            return;
        }
        ee_mem = strtoul(arg[1], (char **)NULL, 10);

        if ( arg_count == 2)
        {
            if (strlen(arg[3]) != 0)
            {
                printf_P(PSTR("{\"err\":\"ParserBroken\"}\r\n"));
                initCommandBuffer();
                return;
            }
        }
        
       // check that argument[2] is UINT8|UINT16|UINT32
        if ( ( arg_count == 3 ) && ( !( (strcmp_P(arg[2], PSTR("UINT8")) == 0) || \
                                                    (strcmp_P(arg[2], PSTR("UINT16")) == 0) || \
                                                    (strcmp_P(arg[2], PSTR("UINT32")) == 0) ) ) )
        {
            printf_P(PSTR("{\"err\":\"EeWrTypUINT8|16|32\"}\r\n"));
            initCommandBuffer();
            return;
        }
        
        printf_P(PSTR("{\"EE[%s]\":"),arg[0]);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  //  check if we can use eeprom, or just loop.
        if ( eeprom_is_ready() ) 
        {
            if ( (strlen(arg[2]) == 0) || (strcmp_P(arg[2], PSTR("UINT8")) ==0) )
            {
                write_eeprom_byte(atoi(arg[1]), (uint8_t) (ee_mem & 0xFF) ) ;
            }
            if ( strcmp_P(arg[2], PSTR("UINT16")) == 0 )
            {
                write_eeprom_word(atoi(arg[1]), (uint16_t) (ee_mem & 0xFFFF) );
            }
            if ( strcmp_P(arg[2], PSTR("UINT32")) == 0 )
            {
                write_eeprom_dword(atoi(arg[1]), ee_mem);
            }
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {  //  keep checking if we can use eeprom, there is a delay after a write
        if ( eeprom_is_ready() ) 
        {
            if (!ee_read_type(arg[0], arg[2]))
            {
                printf_P(PSTR("{\"err\":\"EeWrCmdDn12WTF\"}\r\n"));
                initCommandBuffer();
                return;
            }
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

