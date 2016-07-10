/*
process is used to select command functions from the command line
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
#include "../Capture/capture.h"
#include "ht.h"

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("Ht");
    }
    if ( (strcmp_P( command, PSTR("/count?")) == 0) &&  ( (arg_count == 0) || ( (arg_count == 1) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        Count();
    }
    if ( (strcmp_P( command, PSTR("/capture?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        Capture();
    }
    if ( (strcmp_P( command, PSTR("/event?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        Event();
    }
    if ( (strcmp_P( command, PSTR("/initICP")) == 0) && ( ( (arg_count == 3) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
    {
        InitICP();
    }
    if ( (strcmp_P( command, PSTR("/ht?")) == 0) && ( (arg_count == 0 ) ) )
    {
        Ht();
    }
}
