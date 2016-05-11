/*
Parse serial commands into tokens e.g. "/0/pwm 252" into "/0/pwm\0" and "252\0" 
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
#include <ctype.h>
#include <avr/pgmspace.h>
#include "parse.h"
#include "uart.h"

// used to assemble command line
char command_buf[COMMAND_BUFFER_SIZE];
uint8_t command_head;
uint8_t command_done;

// used to convert command line into its parts
char *command;
char *arg[MAX_ARGUMENT_COUNT];
uint8_t arg_count;

// command loopback happons from the addressed device
uint8_t echo_on;


void initCommandBuffer(void) 
{
    command_buf[1] = '\0';  // best to set the address as a null value
    for (uint8_t i=0; i < MAX_ARGUMENT_COUNT; i++)
    {
        arg[i] = NULL;
    }
    command = NULL;
    command_done = 0;
    command_head =0;
    arg_count = 0;
    echo_on = 0;
}


void StartEchoWhenAddressed(char address)
{
    if ( (!echo_on) && (command_buf[0] == '/') && (command_buf[1] == address) )
    {
        echo_on = 1;
        printf_P(PSTR("%c%c"),command_buf[0], command_buf[1]);
    }
}

// assemble command line from incoming char's 
void AssembleCommand(char input) 
{
    // a return or new-line finishes the line (or starts a new command line)
    if ( (input == '\r') || (input == '\n') ) // pressing enter in picocom sends a \r
    {
        //echo both carrage return and newline.
        if (echo_on) printf("\r\n");
        
        // finish command line as a null terminated string
        command_buf[command_head] = '\0';
        
        // do not go past the buffer
        if (command_head < (COMMAND_BUFFER_SIZE - 1) )
        {
            ++command_head;
        }
        else // command is to big 
        {
            if (echo_on) printf_P(PSTR("Ignore_Input\r\n"));
            initCommandBuffer();
        }
        command_done = 1;                     
    }
    else
    {
        //echo the input  
        if (echo_on) printf("%c", input);

        // assemble the command
        command_buf[command_head] = input;
        
        // do not go past the buffer
        if (command_head < (COMMAND_BUFFER_SIZE - 1) )
        {
            ++command_head;
        }
        else // command is to big
        {
            command_buf[1] = '\0'; 
            if (echo_on) printf_P(PSTR("Ignore_Input\r\n"));
            echo_on = 0;
        }
    }
}

// find argument(s) starting from a given offset
uint8_t findArgument(uint8_t at_command_buf_offset) 
{
    if (at_command_buf_offset < COMMAND_BUFFER_SIZE) 
    {
        uint8_t lastAlphaNum = at_command_buf_offset;
        
        //get past any white space, but not end of line (EOL was replaced with a null)
        while (isspace(command_buf[lastAlphaNum]) && !(command_buf[lastAlphaNum] == '\0')) 
        { 
            lastAlphaNum++;
        }

        // after command+space but the char is null
        if( (command_buf[lastAlphaNum] == '\0') ) 
        {
            if (echo_on) printf_P(PSTR("{\"err\": \"NullArgAftrCmd+Sp\"}\r\n"));
            initCommandBuffer();
            return 0;
        }
        
        //for each valid argument add it to the arg array of strings
        for (arg_count = 0; command_buf[lastAlphaNum] != '\0' ; arg_count++) 
        {
            // to many arguments
            if( !(arg_count < MAX_ARGUMENT_COUNT) ) 
            {
                if (echo_on) printf_P(PSTR("{\"err\": \"ArgCnt%dAt%d\"}\r\n"), arg_count, lastAlphaNum);
                initCommandBuffer();
                return 0;
            }   
            
            arg[arg_count] = command_buf + lastAlphaNum;
            
            //  skip through the argument
            while( (isalnum(command_buf[lastAlphaNum]) || (command_buf[lastAlphaNum] == '-')) && (lastAlphaNum < (COMMAND_BUFFER_SIZE-1)) ) 
            { 
                lastAlphaNum++;
            }
            if ( (command_buf[lastAlphaNum] == ARGUMNT_DELIMITER) )
            {
                if ( lastAlphaNum < (COMMAND_BUFFER_SIZE-2) ) 
                {
                    // check if char after delimiter is valid for an arg 
                    if( !(isalnum(command_buf[lastAlphaNum+1]) || (command_buf[lastAlphaNum+1] == '-')) ) 
                    {
                        if (echo_on) printf_P(PSTR("{\"err\": \"ArgAftr'%c@%d!Valid\"}\r\n"),command_buf[lastAlphaNum],lastAlphaNum);
                        initCommandBuffer();
                        return 0;
                    }  
                    
                    // null terminate the argument, e.g. replace the delimiter
                    command_buf[lastAlphaNum] = '\0';
                    lastAlphaNum++;
                }
                else
                {
                    // a delimiter was found but there is not enough room for an argument and null termination
                    if (echo_on) printf_P(PSTR("{\"err\": \"DropArgCmdLn2Lng\"}\r\n"));
                    initCommandBuffer();
                    return 0;
                }
            }
            
            // only EOL or delimiter is valid way to terminate an argument (e.g. a space befor end of line is not valid)
            else if (command_buf[lastAlphaNum] != '\0')
            {
                // do not index past command buffer
                if (echo_on) printf_P(PSTR("{\"err\": \"!DelimAftrArg'%c@%d\"}\r\n"), command_buf[lastAlphaNum],lastAlphaNum);
                initCommandBuffer();
                return 0;
            }
        }
        return arg_count;
    }
    else
    {
        // do not index past command buffer
        if (echo_on) printf_P(PSTR("{\"err\": \"ArgIndxPastCmdBuf\"}\r\n"));
        initCommandBuffer();
        return 0;
    }
}


// in an previous version white space was allowed befor the command, but not now.
// a command string always starts at postion 2 and ends at the first white space
// the command looks like an MQTT topic or the directory structure of a file system 
// e.g. /0/pwm 127
// find end of command and place a null termination so it can be used as a string
uint8_t findCommand(void) 
{
    uint8_t lastAlpha =2;
    
    // the command always starts after the addrss at position 2
    command = command_buf + lastAlpha;
    
    // Only an isspace or null may terminate a valid command.
    // The command is made of chars of isalpa, '/', or '?'.
    while( !( isspace(command_buf[lastAlpha]) || (command_buf[lastAlpha] == '\0') ) && lastAlpha < (COMMAND_BUFFER_SIZE-1) ) 
    {
        if ( isalpha(command_buf[lastAlpha]) || (command_buf[lastAlpha] == '/') || (command_buf[lastAlpha] == '?') ) 
        {
            lastAlpha++;
        }
        else
        {
            if (echo_on) printf_P(PSTR("{\"err\": \"BadCharInCmd '%c'\"}\r\n"),command_buf[lastAlpha]);
            initCommandBuffer();
            return 0;
        }
    }
    
    // command does  not fit in buffer
    if ( lastAlpha >= (COMMAND_BUFFER_SIZE-1) ) 
    {
        if (echo_on) printf_P(PSTR("{\"err\": \"HugeCmd\"}\r\n"));
        initCommandBuffer();
        return 0;
    }

    if ( isspace(command_buf[lastAlpha]) )
    {
        // the next poistion may be an argument.
        if ( findArgument(lastAlpha+1) )
        {
            // replace the space with a null so command works as a null terminated string.
            command_buf[lastAlpha] = '\0';
        }
        else
        {
            // isspace() found after command but argument was not valid 
            if (echo_on) printf_P(PSTR("{\"err\": \"CharAftrCmdBad '%c'\"}\r\n"),command_buf[lastAlpha+1]);
            initCommandBuffer();
            return 0;
        }
    }
    else
    {
        if (command_buf[lastAlpha] != '\0')
        {
            // null must end command. 
            if (echo_on) printf_P(PSTR("{\"err\": \"MissNullAftrCmd '%c'\"}\r\n"),command_buf[lastAlpha]);
            initCommandBuffer();
            return 0;
        }
    }
    // zero indexing is also the count and should match with strlen()
    return lastAlpha;
}