/*
  Initialize ATmega328p Timers  
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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <avr/interrupt.h>
#include "timers.h"

// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
ISR(TIM0_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

unsigned long millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}

unsigned long micros() {
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
#if defined(TCNT0)
	t = TCNT0;
#elif defined(TCNT0L)
	t = TCNT0L;
#else
	#error TIMER 0 not defined
#endif

#ifdef TIFR0
	if ((TIFR0 & (1<<TOV0)) && (t < 255))
		m++;
#else
	if ((TIFR & (1<<TOV0)) && (t < 255))
		m++;
#endif

	SREG = oldSREG;
	
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}

/*  Timer setup is like wiring.c (e.g. Arduino on github) but builds with avr-libc only. */
void initTimers()
{
	// on the ATmega168, timer 0 is also set for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
#if defined(TCCR0A) && defined(WGM01)
	TCCR0A |= (1<<WGM01);
	TCCR0A |= (1<<WGM00);
#endif

	// set timer 0 prescale factor to 64
#if defined(__AVR_ATmega128__)
	// CPU specific: different values for the ATmega128
	TCCR0 |= (1<<CS02);
#elif defined(TCCR0) && defined(CS01) && defined(CS00)
	// this combination is for the standard atmega8
	TCCR0 |= (1<<CS01);
	TCCR0 |= (1<<CS00);
#elif defined(TCCR0B) && defined(CS01) && defined(CS00)
	// this combination is for the standard 168/328/1280/2560
	TCCR0B |= (1<<CS01);
	TCCR0B |= (1<<CS00);
#elif defined(TCCR0A) && defined(CS01) && defined(CS00)
	// this combination is for the __AVR_ATmega645__ series
	TCCR0A |= (1<<CS01);
	TCCR0A |= (1<<CS00);
#else
	#error Timer 0 prescale factor 64 not set correctly
#endif

	// enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
	TIMSK |= (1<<TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
	TIMSK0 |= (1<<TOIE0);
#else
	#error	Timer 0 overflow interrupt not set correctly
#endif

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle

#if defined(TCCR1B) && defined(CS11) && defined(CS10)
	TCCR1B = 0;

	// set timer 1 prescale factor to 64
	TCCR1B |= (1<<CS11);
#if F_CPU >= 8000000L
	TCCR1B |= (1<<CS10);
#endif
#elif defined(TCCR1) && defined(CS11) && defined(CS10)
	TCCR1 |= (1<<CS11);
#if F_CPU >= 8000000L
	TCCR1 |= (1<<CS10);
#endif
#endif
	// put timer 1 in 8-bit phase correct pwm mode
#if defined(TCCR1A) && defined(WGM10)
	TCCR1A |= (1<<WGM10);
#endif

	// set timer 2 prescale factor to 64
#if defined(TCCR2) && defined(CS22)
	TCCR2 |= (1<<CS22);
#elif defined(TCCR2B) && defined(CS22)
	TCCR2B |= (1<<CS22);
//#else
	// Timer 2 not finished (may not be present on this CPU)
#endif

	// configure timer 2 for phase correct pwm (8-bit)
#if defined(TCCR2) && defined(WGM20)
	TCCR2 |= (1<<WGM20);
#elif defined(TCCR2A) && defined(WGM20)
	TCCR2A |= (1<<WGM20);
//#else
	// Timer 2 not finished (may not be present on this CPU)
#endif

#if defined(TCCR3B) && defined(CS31) && defined(WGM30)
	TCCR3B |= (1<<CS31);		// set timer 3 prescale factor to 64
	TCCR3B |= (1<<CS30);
	TCCR3A |= (1<<WGM30);		// put timer 3 in 8-bit phase correct pwm mode
#endif

#if defined(TCCR4A) && defined(TCCR4B) && defined(TCCR4D) /* beginning of timer4 block for 32U4 and similar */
	TCCR4B |= (1<<CS42);		// set timer4 prescale factor to 64
	TCCR4B |= (1<<CS41);
	TCCR4B |= (1<<CS40);
	TCCR4D |= (1<<WGM40);		// put timer 4 in phase- and frequency-correct PWM mode	
	TCCR4A |= (1<<PWM4A);		// enable PWM mode for comparator OCR4A
	TCCR4C |= (1<<PWM4D);		// enable PWM mode for comparator OCR4D
#else /* beginning of timer4 block for ATMEGA1280 and ATMEGA2560 */
#if defined(TCCR4B) && defined(CS41) && defined(WGM40)
	TCCR4B |= (1<<CS41);		// set timer 4 prescale factor to 64
	TCCR4B |= (1<<CS40);
	TCCR4A |= (1<<WGM40);		// put timer 4 in 8-bit phase correct pwm mode
#endif
#endif /* end timer4 block for ATMEGA1280/2560 and similar */	

#if defined(TCCR5B) && defined(CS51) && defined(WGM50)
	TCCR5B |= (1<<CS51);		// set timer 5 prescale factor to 64
	TCCR5B |= (1<<CS50);
	TCCR5A |= (1<<WGM50);		// put timer 5 in 8-bit phase correct pwm mode
#endif
}
