/* MCU  = atmega328p
 *
 */ 

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "pin_macros.h"


int main(void)
{
	// RPUuno has no LED, but B5 is Arduino pin 13.
    b5_output;
    
    while (1) 
    {
        b5_high;
        _delay_ms(5000);
        b5_low;
        _delay_ms(5000);
    }    
}

