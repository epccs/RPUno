/*
I2c-scan 
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
#include <stdio.h>
#include <stdlib.h> 
#include "../lib/parse.h"
#include "../lib/twi.h"
#include "i2c-scan.h"

static uint8_t address;
static uint8_t start_address;
static uint8_t end_address;
static uint8_t found_addresses;
static uint8_t returnCode;

/* Scan the I2C bus between addresses from_addr and to_addr.
   On each address, call the callback function with the address and result.
   If result==0, address was found, otherwise, address wasn't found
   can use result to potentially get other status off the I2C bus, see twi.c   */
void I2c_scan(void)
{
    if (command_done == 10)
    {
        start_address = 0x8; // 0-0x7 is reserved (e.g. general call, CBUS, Hs-mode...)
        end_address = 0x77; // 0x78-0x7F is reserved (e.g. 10-bit...)
        address = start_address;
        found_addresses = 0;
        printf_P(PSTR("{\"scan\":[")); // start of JSON
        command_done = 11;
    }
    
    else if (command_done == 11)
    {
        uint8_t data = 0;
        uint8_t length = 0;
        uint8_t wait = 1;
        uint8_t sendStop = 1;
        returnCode = twi_writeTo(address, &data, length, wait, sendStop); 
        
        if ( (returnCode == 0) && (found_addresses > 0) )
        {
            printf_P(PSTR(","));
        }
        
        if (returnCode == 0)
        {
            printf_P(PSTR("{\"addr\":\"0x%X\"}"),address);
            found_addresses += 1;
        }
        
        if (address >= end_address) 
        {
            command_done = 12;
        }
        else
        {
            address += 1;
        }
    }
    
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("]}\r\n"));
        initCommandBuffer();
    }

    else
    {
        initCommandBuffer();
    }
}

