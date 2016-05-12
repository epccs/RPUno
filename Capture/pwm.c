/*
pwm is part of Capture, it uses a command line arguemnt to set pwm duty (OCR2A), 
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
#include "pwm.h"

/* use ../lib/timers.c to initTimers().
   Timer2 is then in Phase-correct PWM mode, the timer goes up from 0 to 255 and down to 0. 
   The duty can be controled from 0 to 255 by setting the OCR2A or OCR2B register.
   And the output frequency is 16 MHz / 64 / 255 / 2 = 490.196Hz   */
void Pwm(void)
{
    int duty;
    
    if (arg_count == 2)
    {
        duty = atoi(arg[1]);
        if ( (command_done == 10) && (duty >=0) && (duty <= 255) )
        {
            if (strcmp_P( arg[0], PSTR("oc2a")) == 0)
            {
                // set Data Direction Register (its what pinMode(11, OUTPUT) does more or less does)
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
        
                if ( !(DDRB & _BV(PB3)) ) // if bit PB3 is set then it is already an output 
                {
                    DDRB |= _BV(PB3);
                }

                // connect PB3 pin to OC2A output (pwm) from timer 2, channel A set in Clear on Compare Match mode.
                if ( !(TCCR2A & _BV(COM2A1)) ) 
                {
                    TCCR2A |= _BV(COM2A1);
                }
#else
#   error mega328[p] has OC2A on PB3, check Datasheet for your mcu and then fix this file
#endif
                
                OCR2A = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"OCR2A\":\"%d\"}}\r\n"),OCR2A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("oc2b")) == 0)
            {
                // set Data Direction Register (its what pinMode(3, OUTPUT) does more or less does)
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
        
                if ( !(DDRD & _BV(PD3)) ) // if bit PD3 is set then it is already an output 
                {
                    DDRD |= _BV(PD3);
                }

                // connect PD3 pin to OC2B output (pwm) from timer 2, channel A set in Clear on Compare Match mode.
                if ( !(TCCR2A & _BV(COM2B1)) ) 
                {
                    TCCR2A |= _BV(COM2B1);
                }
#else
#   error mega328[p] has OC2B on PD3, check Datasheet for your mcu and then fix this file
#endif
                
                OCR2B = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"OCR2B\":\"%d\"}}\r\n"),OCR2B);
                initCommandBuffer();
            }
        }
        else
        {
            printf_P(PSTR("{\"err\":\"pwmRange_%d\"}\r\n"),duty);
            initCommandBuffer();
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"pwmNeed2Arg\"}\r\n"));
        initCommandBuffer();
    }
}

