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
#include <ctype.h>
#include "../lib/parse.h"
#include "../lib/twi.h"
#include "i2c-cmd.h"


static uint8_t returnCode;

static uint8_t I2cAddress = 0;

static uint8_t txBuffer[TWI_BUFFER_LENGTH];
static uint8_t txBufferIndex = 0;

static uint8_t rxBuffer[TWI_BUFFER_LENGTH];
static uint8_t rxBufferLength = 0;

static uint8_t JsonIndex;


/* set I2C bus addresses and clear the buffer*/
void I2c_address(void)
{
    if (command_done == 10)
    {
        // check that argument[0] is 7 bits e.g. the range 1..127
        uint8_t arg0 = is_arg_in_uint8_range(0,1,127);
        if ( !arg0 )
        {
            initCommandBuffer();
            return;
        }

        txBufferIndex = 0;
        I2cAddress = arg0; 
        printf_P(PSTR("{\"address\":\"0x%X\"}\r\n"),I2cAddress);
        initCommandBuffer();
    }

    else
    {
        initCommandBuffer();
    }
}


/* buffer bytes into an I2C txBuffer */
void I2c_txBuffer(void)
{
    
    if (command_done == 10)
    {
        // check that arguments are digit and in the range 0..255
        for (uint8_t arg_index=0; arg_index < arg_count; arg_index++) 
        {
            if ( ( !( isdigit(arg[arg_index][0]) ) ) || (atoi(arg[arg_index]) < 0) || (atoi(arg[arg_index]) > 255) )
            {
                printf_P(PSTR("{\"err\":\"I2CDatOutOfRng\"}\r\n"));
                initCommandBuffer();
                return;
            }
            if ( txBufferIndex >= TWI_BUFFER_LENGTH)
            {
                printf_P(PSTR("{\"err\":\"I2CBufOVF\"}\r\n"));
                txBufferIndex = 0;
                initCommandBuffer();
                return;
            }
            txBuffer[txBufferIndex] = (uint8_t) atoi(arg[arg_index]);
            txBufferIndex += 1; 
        }

        printf_P(PSTR("{\"txBuffer[%d]\":["),txBufferIndex);
        JsonIndex = 0;
        command_done = 11;
    }
    
    else if (command_done == 11)
    {
        // txBufferIndex is pointing to the next position data would be placed
        if ( JsonIndex >= txBufferIndex ) 
        {
            command_done = 12;
        }
        else
        {
            if (JsonIndex > 0)
            {
                printf_P(PSTR(","));
            }
            printf_P(PSTR("{\"data\":\"0x%X\"}"),txBuffer[JsonIndex]);
            JsonIndex += 1;
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

/* write the I2C txBuffer */
void I2c_write(void)
{
    if (command_done == 10)
    {
        uint8_t wait = 1;
        uint8_t sendStop = 1; 
        uint8_t txBufferLength = txBufferIndex;
        returnCode = twi_writeTo(I2cAddress, txBuffer, txBufferLength, wait, sendStop);
        if (returnCode == 0)
        {
            printf_P(PSTR("{\"returnCode\":\"success\""));
            txBufferIndex = 0; 
        }
        if (returnCode == 1)
            printf_P(PSTR("{\"rtnCode\":\"bufOVF\",\"i2c_addr\":\"%d\""),I2cAddress);
        if ( (returnCode == 2) || (returnCode == 3) )
            printf_P(PSTR("{\"rtnCode\":\"nack\",\"i2c_addr\":\"%d\""),I2cAddress);
        if (returnCode == 4)
            printf_P(PSTR("{\"rtnCode\":\"other\",\"i2c_addr\":\"%d\""),I2cAddress);
        command_done = 11;
    }
    
    else if (command_done == 11)
    {
        printf_P(PSTR("}\r\n"));
        initCommandBuffer();
    }
    
    else
    {
        initCommandBuffer();
    }
}


/* write the byte(s) in buffer (e.g. command byte) to I2C address 
    without a stop condition. Then send a repeated Start condition 
    and address with read bit to obtain readings.*/
void I2c_read(void)
{
    if (command_done == 10)
    {
        // check that argument[0] range 1..32
        if ( ( !( isdigit(arg[0][0]) ) ) || (atoi(arg[0]) < 1) || (atoi(arg[0]) > TWI_BUFFER_LENGTH) )
        {
            printf_P(PSTR("{\"err\":\"I2cReadUpTo%d\"}\r\n"),TWI_BUFFER_LENGTH);
            initCommandBuffer();
            return;
        }
        
        // send command byte(s) without a Stop (causing a repeated Start durring read)
        if (txBufferIndex) 
        {
            uint8_t wait = 1;
            uint8_t sendStop = 0; 
            uint8_t txBufferLength = txBufferIndex;
            returnCode = twi_writeTo(I2cAddress, txBuffer, txBufferLength, wait, sendStop);
            if (returnCode == 0)
            {
                printf_P(PSTR("{\"rxBuffer\":["));
                txBufferIndex = 0; 
                command_done = 11;
            }
            else
            {
                if (returnCode == 1)
                    printf_P(PSTR("{\"rtnCode\":\"bufOVF\",\"i2c_addr\":\"%d\""),I2cAddress);
                if ( (returnCode == 2) || (returnCode == 3) )
                    printf_P(PSTR("{\"rtnCode\":\"nack\",\"i2c_addr\":\"%d\""),I2cAddress);
                if (returnCode == 4)
                    printf_P(PSTR("{\"rtnCode\":\"other\",\"i2c_addr\":\"%d\""),I2cAddress);
                printf_P(PSTR("}\r\n"));
                initCommandBuffer();
            }
        }
        else
        {
            printf_P(PSTR("{\"rxBuffer\":["));
            command_done = 11;
        }
    }

    else if (command_done == 11)
    {
        // read I2C, it will cause a repeated start if a command has been sent.
        uint8_t sendStop = 1; 
        uint8_t quantity = (uint8_t) atoi(arg[0]); // arg[0] has been checked to be in range 1..32
        uint8_t read = twi_readFrom(I2cAddress, rxBuffer, quantity, sendStop);
        rxBufferLength = read;
        JsonIndex = 0;
        command_done = 12;
    }

    else if (command_done == 12)
    {
        if (JsonIndex > 0)
        {
            printf_P(PSTR(","));
        }
        printf_P(PSTR("{\"data\":\"0x%X\"}"),rxBuffer[JsonIndex]);

        if ( JsonIndex >= (rxBufferLength-1) ) 
        {
            command_done = 13;
        }
        else
        {
            JsonIndex += 1;
        }
    }
    
    else if (command_done == 13)
    {
        printf_P(PSTR("]}\r\n"));
        initCommandBuffer();
    }

    else
    {
        initCommandBuffer();
    }
}