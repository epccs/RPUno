/*
  Analog-to-Digital Converter
  Copyright (c) 2016 Ronald S,. Sutherland

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <util/atomic.h>
#include "adc.h"

/* ADC setup is like wiring.c (e.g. Arduino on GitHub), but a reference 
    is selected during initialization, and it builds with avr-libc only.  */
void initAdcSingleConversion(uint8_t reference)
{
    /* The user has to select the reference they want to
        initialization the ADC with, it is not automagic.  
        If smoke is let out because AREF was connected to
        to another source while AVCC was selected ... 
        AREF should probably never have been broken out 
        as a user pin */
    volatile uint8_t analog_reference = reference;
    
    // select the reference so it has time to stabalize.
    ADMUX = (analog_reference << 6);
    
#if defined(ADCSRA)
	// set a2d prescaler so we are inside the desired 50-200 KHz range.
	#if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
		ADCSRA |= _BV(ADPS2);
		ADCSRA |= _BV(ADPS1);
		ADCSRA |= _BV(ADPS0);
	#elif F_CPU >= 8000000 // 8 MHz / 64 = 125 KHz
		ADCSRA |= _BV(ADPS2);
		ADCSRA |= _BV(ADPS1);
		ADCSRA &= ~_BV(ADPS0);
	#elif F_CPU >= 4000000 // 4 MHz / 32 = 125 KHz
		ADCSRA |= _BV(ADPS2);
		ADCSRA &= ~_BV(ADPS1);
		ADCSRA |= _BV(ADPS0);
	#elif F_CPU >= 2000000 // 2 MHz / 16 = 125 KHz
		ADCSRA |= _BV(ADPS2);
		ADCSRA &= ~_BV(ADPS1);
		ADCSRA &= ~_BV(ADPS0);
	#elif F_CPU >= 1000000 // 1 MHz / 8 = 125 KHz
		ADCSRA &= ~_BV(ADPS2);
		ADCSRA |= _BV(ADPS1);
		ADCSRA |= _BV(ADPS0);
	#else // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
		ADCSRA &= ~_BV(ADPS2);
		ADCSRA &= ~_BV(ADPS1);
        ADCSRA |= _BV(ADPS0);
	#endif
	// enable a2d conversions
	ADCSRA |= _BV(ADEN);
#endif
}

// select the reference so it can stabalize, if it smokes now
// it would smoke when analogRead() was done anyway.
void analogReference(uint8_t mode)
{
	analog_reference = mode;
    ADMUX = (analog_reference << 6);
}

// Use the ADC channel number
int analogRead(uint8_t channel)
{
	uint8_t low, high;

#if defined(ADCSRB) && defined(MUX5)
	// the MUX5 bit of ADCSRB selects whether we're reading from channels
	// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((channel >> 3) & 0x01) << MUX5);
#endif
  
	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
#if defined(ADMUX)
	ADMUX = (analog_reference << 6) | (channel & 0x07);
#endif

#if defined(ADCSRA) && defined(ADCL)
	// start the conversion
	ADCSRA |= _BV(ADSC);

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & _BV(ADSC));    

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read. 
	low  = ADCL;
	high = ADCH;
#else
	// we dont have that ADC channel, return -1
	return -1;
#endif

	// combine the two bytes
	return (high << 8) | low;
}
