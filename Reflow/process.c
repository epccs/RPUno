/*
process is part of Capture, it is used to select command functions from the command line
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
#include "../lib/parse.h"
#include "process.h"
#include "../Uart/id.h"
#include "../Eeprom/ee.h"
#include "reflow.h"

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("Reflow");
    }
    if ( (strcmp_P( command, PSTR("/reflow?")) == 0) &&  (arg_count == 0 ) )
    {
        Reflow();
    }
    if ( (strcmp_P( command, PSTR("/ee?")) == 0) && (arg_count == 1 ) )
    {
        EEread();
    }
    if ( (strcmp_P( command, PSTR("/ee")) == 0) && (arg_count == 2 ) )
    {
        EEwrite();
    }
}
