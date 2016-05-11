/*
Process a command (and its arguments )
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
#include "id.h"
#include "pwm.h"
#include "count.h"
#include "duty.h"

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id();
    }
    if ( (strcmp_P( command, PSTR("/pwm")) == 0) && (arg_count == 1) )
    {
        Pwm();
    }
    if ( (strcmp_P( command, PSTR("/count?")) == 0) && (arg_count == 0) )
    {
        Count();
    }
    if ( (strcmp_P( command, PSTR("/duty?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Duty();
    }
}
