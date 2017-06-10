/*
analog is part of Adc, it returns Analog Conversions for channels which are provided in parse arguments, 
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
#include "../lib/pins_board.h"
#include "references.h"
#include "calibrate.h"

// arg[0] is external voltage on AVCC pin in microVolts (10E-6)
void CalibrateAVCC(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] starts with a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"%s0NaN\"}\r\n"),command[1]);
            return;
        }
        unsigned long ul_from_arg0 = strtoul(arg[0], (char **)NULL, 10);
        if ( !IsValidValForAvccRef(&ul_from_arg0) )
        {
            printf_P(PSTR("{\"err\":\"%s0OtOfRng\"}\r\n"),command[1]);
            initCommandBuffer();
            return;
        }
        
        ref_extern_avcc_uV = ul_from_arg0;
        printf_P(PSTR("{\"REF\":{"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        printf_P(PSTR("\"extern_avcc\":\"%1.4f\","),(ref_extern_avcc_uV/1.0E6));
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

// arg[0] is internal bandgap 1V1 in microVolts
void Calibrate1V1(void)
{
    if ( (command_done == 10) )
    {
        // check that arg[0] starts with a digit 
        if ( ( !( isdigit(arg[0][0]) ) ) )
        {
            printf_P(PSTR("{\"err\":\"%s0NaN\"}\r\n"),command[1]);
            return;
        }
        unsigned long ul_from_arg0 = strtoul(arg[0], (char **)NULL, 10);
        if ( !IsValidValFor1V1Ref(&ul_from_arg0) )
        {
            printf_P(PSTR("{\"err\":\"%s0OtOfRng\"}\r\n"),command[1]);
            initCommandBuffer();
            return;
        }

        ref_intern_1v1_uV = ul_from_arg0;
        printf_P(PSTR("{\"REF\":{"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {
        printf_P(PSTR("\"intern_1v1\":\"%1.4f\","),(ref_intern_1v1_uV/1.0E6));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"Cal1V1CmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}


// Use with command line to load referances values from EEPROM
void ReFmEe(void)
{
    if ( (command_done == 10) )
    {
        if ( eeprom_is_ready() )
        {
            if (LoadAnalogRefFromEEPROM())
            {
                printf_P(PSTR("{\"REF\":{"));
                command_done = 11;
            }
            else
            {
                printf_P(PSTR("{\"err\":\"LdAdcEeFailNoRef\"}\r\n"));
                initCommandBuffer();
                return;
            }
        }
    }
    else if ( (command_done == 11) )
    {  
        printf_P(PSTR("\"extern_avcc\":\"%1.4f\","),(ref_extern_avcc_uV/1.0E6));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("\"intern_1v1\":\"%1.4f\","),(ref_intern_1v1_uV/1.0E6));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"LdAdcEeCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// Use with command line to save referances values to EEPROM
void Ref2Ee(void)
{
    if ( (command_done == 10) )
    {
         if ( IsValidValForAvccRef(&ref_extern_avcc_uV) && IsValidValFor1V1Ref(&ref_intern_1v1_uV) )
         {
            if ( WriteEeReferenceId() ) // may be false when eeprom is not ready
            {
                printf_P(PSTR("{\"REF\":{"));
                command_done = 11;
            }
         }
         else
        {
            printf_P(PSTR("{\"err\":\"SvFailBadRef\"}\r\n"));
            initCommandBuffer();
            return;
        }
    }
    else if ( (command_done == 11) )
    {
        if (WriteEeReferenceAvcc())
        {
            printf_P(PSTR("\"extern_avcc\":\"%1.4f\","),(ref_extern_avcc_uV/1.0E6));
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        if (WriteEeReference1V1())
        {
            printf_P(PSTR("\"intern_1v1\":\"%1.4f\","),(ref_intern_1v1_uV/1.0E6));
            command_done = 13;
        }
    }
    else if ( (command_done == 13) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"SvAdcEeCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}


