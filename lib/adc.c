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

volatile int adc[ADC_CHANNELS];
volatile uint8_t adc_channel;
volatile uint8_t ADC_auto_conversion;
volatile uint8_t analog_reference;

// Interrupt service routine for enable_ADC_auto_conversion
ISR(ADC_vect){
    // ADCL contain lower 8 bits, ADCH upper (two bits)
    // Must read ADCL first
    adc[adc_channel] = ADCL | (ADCH << 8);

    ++adc_channel;
    if (adc_channel >= ADC_CHANNELS) 
    {
        adc_channel = 0;
    }
    // select next channel to do conversion on
    ADMUX &= ~(1<<MUX3) & ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);
    ADMUX = (((ADMUX & ~(1<<REFS1)) | (1<<REFS0)) & ~(1<<ADLAR)) + adc_channel;

    // set ADSC in ADCSRA, ADC Start Conversion
    ADCSRA |= (1<<ADSC);
}


/* This ADC setup is almost like wiring.c (e.g. Arduino on GitHub), except
    a reference is selected at initialization (and it only needs avr-libc).  */
void init_ADC_single_conversion(uint8_t reference)
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
    ADC_auto_conversion = 0;
}


/* This changes the ADC to Auto Trigger mode, so it will continusly take readings 
    Why? AnalogRead() starts a single conversion and then blocks the program for 
    over 1600 clocks as it waits for the reading, which will break icp1.c and may 
    cause uart.c some issues. 

    Once AutoConversion is running the Analog value is accessed by reading the adc[] 
    array which does not block, although the reading could be 14,000 clock counts old.*/
void enable_ADC_auto_conversion()
{
    adc_channel = 0;
    for(uint8_t i=0; i<ADC_CHANNELS; i++) 
    {
        adc[i] = 0;
    }

    // ADLAR is right adjust (zero) by default so it does not need clear e.g. ~(1<<ADLAR)
    // MUX[3:0] default to zero which sellects ADC0
    // analog_reference:
    //      EXTERNAL_AREF 0
    //      EXTERNAL_AVCC (1<<REFS0)
    //      INTERNAL_1V1 (1<<REFS1) | (1<<REFS0)
#if defined(ADMUX)
    ADMUX = ( ( (ADMUX & ~(ADREFSMASK)) & ~(0x07)) | analog_reference );
#else
#   error missing ADMUX register which is used to sellect the reference and channel
#endif

    // in ADCSRA:
    // set ADEN, Enable ADC
    // clear ADATE, not Auto Trigger Enable
    // set ADIE, Interrupt Enable
    // set ADPS[2:0], Prescaler division factor (128, thus 16M/128 = 125kHz ADC clock)
    // Note, the first instruction takes 25 ADC clocks to execute, next takes 13 clocks
#if defined(ADCSRA) && defined(ADCL)
    ADCSRA = ( (ADCSRA | (1<<ADEN)) & ~(1<<ADATE) ) | (1 << ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    // set ADSC in ADCSRA, ADC Start Conversion
    ADCSRA |= (1<<ADSC);
#else
#   error missing ADCSRA register which has ADSC bit that is used to start a conversion
#endif
    ADC_auto_conversion =1;
}


// select the reference so it can stabalize, if it smokes now
// it would smoke when analogRead() was done anyway.
void analogReference(uint8_t mode)
{
	analog_reference = mode;
#if defined(ADMUX)
        ADMUX = ( (ADMUX & ~(ADREFSMASK)) | analog_reference );
#else
#   error missing ADMUX register which is used to sellect the reference and channel
#endif
}


// Use the ADC channel number only (not the pin number)
int analogRead(uint8_t channel)
{
    if (ADC_auto_conversion)
    {
        ATOMIC_BLOCK ( ATOMIC_RESTORESTATE )
        {
            // this moves two byes one at a time, so the ISR could change it durring the move
            return adc[channel];
        }
    }
    else
    {
        uint8_t low, high;

#if defined(ADCSRB) && defined(MUX5)
        // the MUX5 bit of ADCSRB selects whether we're reading from channels
        // 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
        ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((channel >> 3) & 0x01) << MUX5);
#endif
      
        // mask reference select (REFS) and channel MUX off,  then set referenc and select the
        // channel (low 4 bits).  ADLAR is not changed (0 is the default).
#if defined(ADMUX)
        ADMUX = ( ( (ADMUX & ~(ADREFSMASK)) & ~(0x07)) | analog_reference ) | (channel & 0x07);
#else
#   error missing ADMUX register which is used to sellect the reference and channel
#endif

#if defined(ADCSRA) && defined(ADCL)
        // start the conversion
        ADCSRA |= (1 <<ADSC);

        // ADSC is cleared when the conversion finishes
        while (ADCSRA & (1 <<ADSC));    

        // we have to read ADCL first; doing so locks both ADCL
        // and ADCH until ADCH is read. 
        low  = ADCL;
        high = ADCH;
#else
#   error missing ADCSRA register which has ADSC bit that is used to start a conversion
#endif

        // combine the two bytes
        return (high << 8) | low;
    }
    // this should never run.
    return -1;
}
