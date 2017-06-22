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
#include "icp_buf.h"
#include "icp1.h"

volatile uint8_t icp1_edge_mode;
ICP_ISR icp1;

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

    // index to next buffer location 
    uint8_t loacal_head = (icp1.head+1) & ICP_EVENT_BUFF_MASK;
    
    // put the Capture Register bytes of timestamp onto the buffer
#if defined(ICR1L) && defined(ICR1H)
    icp1.event.Byt0[loacal_head] = ICR1L; 
    icp1.event.Byt1[loacal_head] = ICR1H; 
#else
#   error missing ICR1 register which is used to hold a 16 bit capture from timer 1
#endif
    icp1.event.status[loacal_head] = (rising & (1<<RISING));
    icp1.head = loacal_head;
    
    // count each timestamp
    icp1.count++;

    // edge tracking: rising or falling
    // ATmega328p datasheet said to clear ICF1 after edge direction change 
    // setup to catch rising edge: TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1;
    // setup to catch falling edge: TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; 
    // swap edge setup to catch the next edge
#if defined(TCCR1B) && defined(ICES1) && defined(TIFR1) && defined(ICF1)
    if (icp1_edge_mode == TRACK_BOTH)
    {
        if (rising) 
        { TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; }
        else 
        { TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1; }
    }
#else
#   error TCCR1B register needs bit ICES1, and TIFR1 register needs bit ICF1 to set capture edge
#endif
}

/* Virtual timer counts each Timer1 overflow.
    e.g. 2^32 clocks (about 4.3E9, thus will overflow after 4.3 minutes) */
ISR(TIMER1_OVF_vect) 
{
    t1vc.word++;
    icp1_event_count_at_OVF = icp1.count;
}

// mode { TRACK_FALLING: 0, TRACK_RISING: 1, TRACK_BOTH: 2)
// prescaler { masked with 0x7 so only lower 3 bits are used see CS12, CS11, and CS10}
//          1 sets Timer1 to CPU clock, 2 sets Timer1 prescaler /8 ... 
void initIcp1(uint8_t mode, uint8_t prescaler) 
{
    icp1.count = 0;
    rising = 0;
    icp1.head = 0;
    t1vc.word = 0;
    icp1_edge_mode = mode;
    
    // Input Capture setup
#if defined(TCCR1A)
    TCCR1A = 0;
#else
#   error missing TCCR1A register which is used to control Timer 1
#endif

    
    // ICNC1: Enable Input Capture Noise Canceler
    // ICES1: = 1 for trigger on rising edge
    // CS12 CS11 CS10 : set prescaler from system clock (F_CPU)
    //   0    0    1  :   /1    No prescaler, CPU clock
    //   0    1    0  :   /8    prescaler
    //   0    1    1  :   /64   prescaler
    //   1    0    0  :   /256  prescaler
    //   1    0    1  :   /1024 prescaler */
#if defined(TCCR1B)
    TCCR1B = (prescaler & 0x7);
#else
#   error missing TCCR1B register which is used to control Timer 1 prescaler
#endif

#if defined(TCCR1C)
    TCCR1C = 0;
#else
#   error missing TCCR1C register which is used to control Timer 1
#endif

#if defined(TCCR1B) && defined(ICES1) && defined(TIFR1) && defined(ICF1)
    if (icp1_edge_mode) // initialize to TRACK_BOTH orTRACK_RISING
    { TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1; }
    else // initialize to TRACK_FALLING
    { TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; }
#else
#   error TCCR1B register needs bit ICES1, and TIFR1 register needs bit ICF1 to set capture edge
#endif

    // Interrupt setup
    // ICIE1: Input capture
    // TOIE1: Timer1 overflow
#if defined(TIFR1) && defined(ICF1) && defined(TOV1)
    TIFR1 = (1<<ICF1) | (1<<TOV1);	// clear pending interrupts
#else
#   error TIFR1 register needs bit ICF1 set to run the ISR on ICP1 pin events and bit TOV1 to run the ISR on Timer 1 overflow events
#endif
#if defined(TIMSK1) && defined(ICIE1) && defined(TOIE1)
    TIMSK1 = (1<<ICIE1) | (1<<TOIE1);	// enable interupts
#else
#   error TIMSK1 register needs bit ICIE1 set to enable interrupts from ICP1 pin events and bit TOIE1 to enable interupts from Timer 1 overflow events
#endif

    // Set up the Input Capture pin, ICP1, i.e. on an Arduino Uno this is what pinMode(8, INPUT) does
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
    if ( (DDRB & (1<<PB0)) ) // if bit PB0 is set then it is an OUTPUT
    {
        DDRB &= ~(1<<PB0);
    }
    
    // floating may have 60 Hz noise on it (I'll assume it is connected to a pulse source). 
    PORTB &= ~(1<<PB0); //Arduino Uno digitalWrite(8, 0) 
    
    // or enable the pullup by setting bit PB0 high in PORTB register. 
    // PORTB |= (1<<PB0); // Arduino Uno digitalWrite(8, 1) 

#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
    if ( (DDRD & (1<<PD6)) ) // if bit PD6 is set then it is an OUTPUT
    {
        DDRD &= ~(1<<PD6);
    }

    // floating may have 60 Hz noise on it (I'll assume it is connected to a pulse source). 
    PORTD &= ~(1<<PD6); //i.e. like Wiring's digitalWrite(8, LOW) function found in Arduino 
    
    // or enable the pullup by setting the bit high in PORT register. 
    // PORTD |= (1<<PD6);
#else
#   error I do not know where ICP1 is on your MCU, check the Datasheet and then fix this file
#endif
}

