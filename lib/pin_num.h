/* ATmega328 DigitalIO Library
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
 * 
 * Started with stuff from William Greiman at https://github.com/greiman/SdFat/blob/master/SdFat/utility/DigitalPin.h
 * removed noise so I could see what was going on.
 * "fast", is it really? Who cares. This sort of software is for fun, in actuality it is crap method, Should I call it crapDigitalRead(),
 * removed C++ stuff for use in C programs. 
 *
 */
#ifndef PinNum_h
#define PinNum_h

// avr-libc
#include <avr/io.h>
#include <util/atomic.h>

// avr-gcc
#include <stdbool.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

typedef struct {
  volatile uint8_t* ddr; 
  volatile uint8_t* pin; 
  volatile uint8_t* port;
  uint8_t bit;  
} Pin_Map;

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)

#define NUM_DIGITAL_PINS 20

/* Each of the AVR Digital I/O ports is associated with three I/O registers. 
8 bit Data Direction Register (DDRx) each bit sets a pin as input (=0) or output (=1).
8 bit Port Input Register (PINx) each  bit is the input from a pin that was latched durring last low edge of the system clock.
8 bit Port Data Register (PORTx) each bit drives a pin if set as output (or sets pullup if input)
Where x is the port A, B, C, etc.

Wiring (e.g. Arduino) uses pin numbers to control their functions. 
This is wasteful, but teaching " DDRB |= _BV(PB5)" to set an LED pin as an output is not going to do.
Make no mistake this is for the kids, but is also is for fun :-) */
static const Pin_Map pinMap[NUM_DIGITAL_PINS] = {
    [0] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD0 },
    [1] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD1 },
    [2] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD2 },
    [3] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD3 },
    [4] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD4 },
    [5] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD5 },
    [6] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD6 },
    [7] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD7 }, 
    [8] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB0 }, 
    [9] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB1 }, 
    [10] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB2 }, 
    [11] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB3 }, 
    [12] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB4 }, 
    [13] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB5 }, 
    [14] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC0 }, 
    [15] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC1 }, 
    [16] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC2 }, 
    [17] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC3 }, 
    [18] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC4 }, 
    [19] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC5 } 
};
#endif  // defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__)  || defined(__AVR_ATmega328P__)

void badPinNumber(void) __attribute__((error("Pin number is too large or not a constant")));

static inline __attribute__((always_inline)) void badPinCheck(uint8_t pin) 
{
    if (pin >= NUM_DIGITAL_PINS) badPinNumber();
}

static inline __attribute__((always_inline))
void BitWriteSafe(volatile uint8_t* register_addr, uint8_t bit_offset, bool value_for_bit) 
{
    uint8_t oldSREG;
    if (register_addr > ((uint8_t*)0x5F)) {
        oldSREG = SREG;
        cli();
    }
    if (value_for_bit) {
        *register_addr |= 1 << bit_offset;
    } else {
        *register_addr &= ~(1 << bit_offset);
    }
    if (register_addr > ((uint8_t*)0x5F)) {
        SREG = oldSREG;
    }
}

/* read value from pin number */
static inline __attribute__((always_inline))
bool DigitalRead(uint8_t pin_num) 
{
    badPinCheck(pin_num);
    return (*pinMap[pin_num].pin >> pinMap[pin_num].bit) & 1;
}

/* toggle pin number  */
static inline __attribute__((always_inline))
void DigitalToggle(uint8_t pin) {
  badPinCheck(pin);
  if ( pinMap[pin].pin > ((uint8_t*)0X5F) ) {
    // must write bit to high address port
    *pinMap[pin].pin = 1 << pinMap[pin].bit;
  } else {
    // will compile to sbi and PIN register will not be read.
    *pinMap[pin].pin |= 1 << pinMap[pin].bit;
  }
}

/* set pin value */
static inline __attribute__((always_inline))
void DigitalWrite(uint8_t pin_num, bool value_for_bit) {
  badPinCheck(pin_num);
  BitWriteSafe(pinMap[pin_num].port, pinMap[pin_num].bit, value_for_bit);
}

/* set pin mode INPUT and OUTPUT */
static inline __attribute__((always_inline))
void PinMode(uint8_t pin_num, bool output_mode) {
  badPinCheck(pin_num);
  BitWriteSafe(pinMap[pin_num].ddr, pinMap[pin_num].bit, output_mode);
}

/* set both mode and value, note INPUT mode and high/low value will enable/disable the pin's pull-up */
static inline __attribute__((always_inline))
void PinConfig(uint8_t pin_num, bool output_mode, bool value_for_bit) {
  PinMode(pin_num, output_mode);
  DigitalWrite(pin_num, value_for_bit);
}
#endif  // DigitalPin_h

