/*
id is part of RPUno, it adds the identify command to an RPU_BUS command line interface, 
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
#include "id.h"

// The RPU_BUS master should have the local address
#include "../lib/twi.h"

#define RPU_BUS_MSTR_CMD_SZ 2

char get_Rpu_address(void)
{ 
    uint8_t twi_returnCode;

    uint8_t RPU_mgr_i2c_address = 0x29;

    // ping I2C for an RPU bus manager
    uint8_t address = RPU_mgr_i2c_address;
    uint8_t data = 0;
    uint8_t length = 0;
    uint8_t wait = 1;
    uint8_t sendStop = 1;
    twi_returnCode = twi_writeTo(address, &data, length, wait, sendStop); 
    
    if (twi_returnCode != 0)
    { 
        return 0;
    }
    // An RPU bus manager was found now try to read the bus address from it
    // note the first byte is command, second is for that data (it will size the reply from what was sent)
    uint8_t txBuffer[RPU_BUS_MSTR_CMD_SZ] = {0x00,0x00}; //comand 0x00 should Read Shield RPU addr;
    length = RPU_BUS_MSTR_CMD_SZ;
    sendStop = 0;  //this will cause a I2C repeated Start durring read
    twi_returnCode = twi_writeTo(address, txBuffer, length, wait, sendStop); 
    if (twi_returnCode != 0)
    {
        return 0;
    }
    
    uint8_t rxBuffer[RPU_BUS_MSTR_CMD_SZ];
    sendStop = 1;
    uint8_t quantity = RPU_BUS_MSTR_CMD_SZ;
    uint8_t bytes_read = twi_readFrom(address, rxBuffer, quantity, sendStop);
    if ( bytes_read != quantity )
    {
        return 0;
    }
    else
    {
        return (char) (rxBuffer[1]);
    }
}


void Id(char name[])
{ 
    // /id? 
    if ( (command_done == 10) && (arg_count == 0) )
    {
        printf_P(PSTR("{\"id\":{"));
        command_done = 11;
    }
    // /id? name 
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("name")) == 0) ) 
    {
        printf_P(PSTR("{\"id\":{"));
        command_done = 11;
    }
    // /id? desc
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("desc")) == 0) )
    {
        printf_P(PSTR("{\"id\":{" ));
        command_done = 12;
    }
    // /id? avr-gcc
    else if ( (command_done == 10) && (arg_count == 1) && (strcmp_P( arg[0], PSTR("avr-gcc")) == 0) )
    {
        printf_P(PSTR("{\"id\":{"));
        command_done = 14;
    }
    else if ( command_done == 11 )
    {
        printf_P(PSTR("\"name\":\"%s\"" ),name);
        if (arg_count == 1) 
        { 
            command_done = 15;  
        }
        else 
        { 
            printf_P(PSTR("," ));
            command_done = 12; 
        }
    }
    else if ( command_done == 12 )
    {
        printf_P(PSTR("\"desc\":\"RPUno Board /w " ));
        command_done = 13;
    }
    else if ( command_done == 13 )
    {
        printf_P(PSTR("atmega328p and LT3652\""));
        if (arg_count == 1) 
        { 
            command_done = 15; 
        }
        else 
        { 
            printf_P(PSTR("," ));
            command_done = 14; 
        }
    }
    else if ( command_done == 14 )
    {
        printf_P(PSTR("\"avr-gcc\":\"%d.%d\""),__GNUC__,__GNUC_MINOR__);
        command_done = 15; 
    }
    else if ( command_done == 15 )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"idBadArg_%s\"}\r\n"),arg[0]);
        initCommandBuffer();
    }
}

