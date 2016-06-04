/*
    Input Capture Unit 1 used to timestamp an event for Timing Pulse Interpolation
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

    Capture can skip pulses if the next event is less then about 300 counts, 
    e.g. about 19% duty at 10kHz or 1.9% duty at 1kHz (assuming a 16MHz MCU).
    
    http://forum.arduino.cc/index.php?topic=260172.0
*/

#include <util/atomic.h>
#include "icp1.h"

volatile uint8_t event_Byt0[EVENT_BUFF_SIZE];
volatile uint8_t event_Byt1[EVENT_BUFF_SIZE];
volatile uint8_t event_status[EVENT_BUFF_SIZE];
volatile uint8_t icp1_head; 
volatile uint32_t icp1_event_count; 
volatile uint32_t icp1_event_count_at_OVF; 
volatile uint8_t rising; 

// timer 1 virtual counter
volatile WORD_2_BYTE t1vc;

/* 
Input Capture Unit 1 interrupt handler (Timing Pulse Interpolation)
Interrupts are not instant and are often blocked by being disabled, in fact while doing this
interrupt the Uart, Timer0, and other interrupts are blocked, just like they will block this one.
The capture event causes Timer1 to be copied into the ICR1 register and is instant for our purposes.
Precision time stamps are used to measure many things like a servo control signal, flow-meter, coulomb 
counters, or any sufficiently stable oscillator,  An example of a standards-based application is 
Double-Timing Pulse Interpolation (API 4.6) which uses a clock to time flow meter events and 
another clock to time volume events from a calibrated Prover (a volume measurement device) 
during a calibration of a custody transfer flow meter (it can then be certified to that standard). */
ISR(TIMER1_CAPT_vect) {

    // put the next timestamp onto the buffer
    icp1_head = (icp1_head+1) & EVENT_BUFF_MASK;
    
    // Capture Register bytes
    event_Byt0[icp1_head] = ICR1L; 
    event_Byt1[icp1_head] = ICR1H; 
    event_status[icp1_head] = (rising & (1<<RISING));
    
    // count all edges
    icp1_event_count++;

    // edge tracking: rising or falling
    // ATmega328p datasheet said to clear ICF1 after edge direction change 
    // setup to catch rising edge: TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1;
    // setup to catch falling edge: TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; 
    // swap edge setup to catch the next edge
    if (rising) 
    { 
        TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; 
    }
    else 
    {
        TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1;
    }
}

/* Virtual timer counts each Timer1 overflow.
    e.g. 2^32 clocks (about 4.3E9, thus will overflow after 4.3 minutes) */
ISR(TIMER1_OVF_vect) 
{
    t1vc.word++;
    icp1_event_count_at_OVF = icp1_event_count;
}

void initIcp1(void) 
{
    icp1_event_count = 0;
    rising = 0;
    icp1_head = 0;
    t1vc.word = 0;
    
    // Input Capture setup
    // ICNC1: Enable Input Capture Noise Canceler
    // ICES1: = 1 for trigger on rising edge
    // CS12 CS11 CS10 : set prescaler from system clock (F_CPU)
    //   0    0    1  :   /1    No prescaler, CPU clock
    //   0    1    0  :   /8    prescaler
    //   0    1    1  :   /64   prescaler
    //   1    0    0  :   /256  prescaler
    //   1    0    1  :   /1024 prescaler */
    TCCR1A = 0;
    TCCR1B = (0<<ICNC1) | (1<<CS10);
    TCCR1C = 0;

    // initialize to catch Falling Edge (this line is also used in ISR to swap from rising to falling edge)
    { TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; }

    // Interrupt setup
    // ICIE1: Input capture
    // TOIE1: Timer1 overflow
    TIFR1 = (1<<ICF1) | (1<<TOV1);	// clear pending interrupts
    TIMSK1 = (1<<ICIE1) | (1<<TOIE1);	// enable interupts

    // Set up the Input Capture pin, ICP1, Arduino Uno pinMode(8, INPUT)
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
    if ( (DDRB & _BV(PB0)) ) // if bit PB0 is set then it is an OUTPUT
    {
        DDRB &= ~_BV(PB0);
    }
    
    // floating may have 60 Hz noise on it (I'll assume it is connected to a pulse source). 
    PORTB &= ~_BV(PB0); //Arduino Uno digitalWrite(8, 0) 
    
    // or enable the pullup by setting bit PB0 high in PORTB register. 
    // PORTB |= _BV(PB0); // Arduino Uno digitalWrite(8, 1) 
    
#else
#   error mega328[p] has ICP1 on PB0, check Datasheet for your mcu and then fix this file
#endif
}

