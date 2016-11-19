/* RPUno Blink
 * Copyright (C) 2016 Ronald Sutherland
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the Arduino DigitalIO Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */ 

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"

#define BLINK_DELAY 1000UL

static unsigned long blink_started_at;

void setup(void) 
{
	// RPUuno has no LED, but LED_BUILTIN is defined as pin 13 anyway.
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    
    //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    initTimers(); 

    sei(); // Enable global interrupts to start TIMER0
    
    blink_started_at = millis();
}

// don't block (e.g. _delay_ms(1000) ), just ckeck if it is time to toggle 
void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > BLINK_DELAY)
    {
        digitalToggle(LED_BUILTIN);
        
        // next toggle 
        blink_started_at += BLINK_DELAY; 
    }
}

int main(void)
{
    setup(); 
    
    while (1) 
    {
        blink();
        // I could do other things, but blinking is a good test 
    }    
}

