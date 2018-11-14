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

volatile int adc[ADC_CHANNELS+1];
volatile uint8_t adc_channel;
volatile uint8_t ADC_auto_conversion;
volatile uint8_t analog_reference;

static uint8_t free_running;

// Interrupt service routine for enable_ADC_auto_conversion
ISR(ADC_vect){
    // ADCL contain lower 8 bits, ADCH upper (two bits)
    // Must read ADCL first (news ADC is now defined for this)
    adc[adc_channel] = ADC;
    //adc[adc_channel] = ADCL | (ADCH << 8);
    
    ++adc_channel;
    if (adc_channel >= ADC_CHANNELS) 
    {
        adc_channel = 0;
        adc[ADC_CHANNELS] = 0x7FFF; // mark to notify burst is done
        if (!free_running)
        {
            return;
        }

    }

#if defined(ADMUX)
    // clear the mux to select the next channel to do conversion without changing the reference
    ADMUX &= ~(1<<MUX3) & ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);
        
    // use a stack register to reset the referance, most likly it is not changed and fliping the hardware bit would mess up the reading.
    ADMUX = ( (ADMUX & ~(ADREFSMASK) & ~(1<<ADLAR) ) | analog_reference ) + adc_channel;
#else
#   error missing ADMUX register which is used to sellect the reference and channel
#endif

    // set ADSC in ADCSRA, ADC Start Conversion
    ADCSRA |= (1<<ADSC);
}


/* This ADC setup is almost like wiring.c (e.g. Arduino on GitHub), except
    a reference is selected at initialization (and it only needs avr-libc).  */
void init_ADC_single_conversion(uint8_t reference)
{
    // The user must select the reference they want to initialization the ADC with, 
    // it should not be automagic. Smoke will get let out if AREF is connected to
    // another source while AVCC is selected. AREF should not be run to a pin.
    analog_reference = reference;
    free_running = 0;

#if defined(ADMUX)
    // clear the channel select MUX
    uint8_t local_ADMUX = ADMUX & ~(1<<MUX3) & ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);

    // clear the reference bits REFS0, REFS1[,REFS2]
    local_ADMUX = (local_ADMUX & ~(ADREFSMASK));
    
    // select the reference so it has time to stabalize.
    ADMUX = local_ADMUX | reference ;
#else
#   error missing ADMUX register which is used to sellect the reference and channel
#endif
    
    
#if defined(ADCSRA)
	// set a2d prescaler so we are inside the desired 50-200 KHz range.
	#if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA |= (1<<ADPS1);
		ADCSRA |= (1<<ADPS0);
	#elif F_CPU >= 8000000 // 8 MHz / 64 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA |= (1<<ADPS1);
		ADCSRA &= ~(1<<ADPS0);
	#elif F_CPU >= 4000000 // 4 MHz / 32 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA &= ~(1<<ADPS1);
		ADCSRA |= (1<<ADPS0);
	#elif F_CPU >= 2000000 // 2 MHz / 16 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA &= ~(1<<ADPS1);
		ADCSRA &= ~(1<<ADPS0);
	#elif F_CPU >= 1000000 // 1 MHz / 8 = 125 KHz
		ADCSRA &= ~(1<<ADPS2);
		ADCSRA |= (1<<ADPS1);
		ADCSRA |= (1<<ADPS0);
	#else // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
		ADCSRA &= ~(1<<ADPS2);
		ADCSRA &= ~(1<<ADPS1);
        ADCSRA |= (1<<ADPS0);
	#endif
	// enable a2d conversions
	ADCSRA |= (1<<ADEN);
#else
#   error missing ADCSRA register which is used to set the prescaler range
#endif
    ADC_auto_conversion = 0;
}


/* This changes the ADC to Auto Trigger mode. It will take readings on each 
    channel and hold them in an array. The array value is accessed by reading from adc[]  */
void enable_ADC_auto_conversion(uint8_t free_run)
{
    adc_channel = 0;
    adc[ADC_CHANNELS] = 0x00;
    free_running = free_run;

    // ADLAR is right adjust (zero) by default so it does not need clear e.g. ~(1<<ADLAR)
    // MUX[3:0] default to zero which sellects ADC0
    // analog_reference:
    //      EXTERNAL_AREF 0
    //      EXTERNAL_AVCC (1<<REFS0)
    //      INTERNAL_1V1 (1<<REFS1) | (1<<REFS0)
#if defined(ADMUX)
    // clear the channel select MUX
    uint8_t local_ADMUX = ADMUX & ~(1<<MUX3) & ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);

    // clear the reference bits REFS0, REFS1[,REFS2]
    local_ADMUX = (local_ADMUX & ~(ADREFSMASK));
    
    // select the reference so it has time to stabalize.
    ADMUX = local_ADMUX | analog_reference ;
#else
#   error missing ADMUX register which is used to sellect the reference and channel
#endif

    // in ADCSRA:
    // set ADEN, Enable ADC
    // clear ADATE, not Auto Trigger Enable
    // set ADIE, Interrupt Enable
    // set ADPS[2:0], Prescaler division factor (128, thus 16M/128 = 125kHz ADC clock)
    // Note, the first reading takes 25 ADC clocks, the next takes 13 clocks
#if defined(ADCSRA)
	// set a2d prescaler so we are inside the desired 50-200 KHz range.
	#if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA |= (1<<ADPS1);
		ADCSRA |= (1<<ADPS0);
	#elif F_CPU >= 8000000 // 8 MHz / 64 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA |= (1<<ADPS1);
		ADCSRA &= ~(1<<ADPS0);
	#elif F_CPU >= 4000000 // 4 MHz / 32 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA &= ~(1<<ADPS1);
		ADCSRA |= (1<<ADPS0);
	#elif F_CPU >= 2000000 // 2 MHz / 16 = 125 KHz
		ADCSRA |= (1<<ADPS2);
		ADCSRA &= ~(1<<ADPS1);
		ADCSRA &= ~(1<<ADPS0);
	#elif F_CPU >= 1000000 // 1 MHz / 8 = 125 KHz
		ADCSRA &= ~(1<<ADPS2);
		ADCSRA |= (1<<ADPS1);
		ADCSRA |= (1<<ADPS0);
	#else // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
		ADCSRA &= ~(1<<ADPS2);
		ADCSRA &= ~(1<<ADPS1);
        ADCSRA |= (1<<ADPS0);
	#endif
    
	// Power up the ADC and set it for a single conversion with interrupts enabled
    ADCSRA = ( (ADCSRA | (1<<ADEN) ) & ~(1<<ADATE) ) | (1 << ADIE);

    // Start an ADC Conversion 
    ADCSRA |= (1<<ADSC);
#else
#   error missing ADCSRA register which has ADSC bit that is used to start a conversion
#endif
    ADC_auto_conversion =1;
}


// select the reference so it can stabalize, if it smokes now
// it would smoke when analogRead() was done anyway.
void analogReference(uint8_t reference)
{
	analog_reference = reference;
#if defined(ADMUX)
    // clear the reference bits REFS0, REFS1[,REFS2]
    uint8_t local_ADMUX = (ADMUX & ~(ADREFSMASK));
    
    // select the reference so it has time to stabalize.
    ADMUX = local_ADMUX | reference ;
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
      
#if defined(ADMUX)
        // clear the channel select MUX, ADLAR is not changed (0 is the default).
        uint8_t local_ADMUX = ADMUX & ~(1<<MUX3) & ~(1<<MUX2) & ~(1<<MUX1) & ~(1<<MUX0);

        // clear the reference bits REFS0, REFS1[,REFS2]
        local_ADMUX = (local_ADMUX & ~(ADREFSMASK));
        
        // select the reference
        local_ADMUX = local_ADMUX | analog_reference ;
    
        // select the channel (note MUX4 has some things for advanced users).
        ADMUX = local_ADMUX | (channel & 0x07) ;
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
