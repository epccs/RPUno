/* MCU  = atmega328p
 *
 */ 

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
//#include "../lib/pin_macros.h"
#include "../lib/pin_num.h"

int main(void)
{
	// RPUuno has no LED, but B5 is Arduino pin 13.
    //b5_output;
    pinMode(13,OUTPUT);
    
    while (1) 
    {
        digitalWrite(13,HIGH);
        //b5_high;
        _delay_ms(1000);
        digitalWrite(13,LOW);
        //b5_low;
        _delay_ms(1000);
    }    
}

