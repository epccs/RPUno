/* RPUno DigitalIO Library
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
 * along with the DigitalIO Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Hacked from William Greiman to work in C with my board
 * Functions are inspired by Wiring from Hernando Barragan
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

Wiring uses pin numbers to control their functions.  {PCINT} function #notes [RPUno] */
static const Pin_Map pinMap[NUM_DIGITAL_PINS] = {
    [0] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD0 }, // {16} RXD [DTR_RXD]
    [1] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD1 }, // {17} TXD [DTR_TXD]
    [2] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD2 }, // {18} INT0 []
    [3] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD3 }, // {19} INT1 OC2B []
    [4] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD4 }, // {20} T0 []
    [5] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD5 }, // {21} T1 OC0B []
    [6] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD6 }, // {22} OC0A []
    [7] = { .ddr=&DDRD, .pin=&PIND, .port=&PORTD, .bit= PD7 }, // {23} []
    [8] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB0 }, // {0} ICP1 []
    [9] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB1 }, // {1} OC1A []
    [10] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB2 }, // {2} nSS OC1B [nSS]
    [11] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB3 }, // {3} MOSI OC2A [MOSI]
    [12] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB4 }, // {4} MISO [MISO]
    [13] = { .ddr=&DDRB, .pin=&PINB, .port=&PORTB, .bit= PB5 }, // {5} SCK [SCK]
    [14] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC0 }, // {8} ADC0 []
    [15] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC1 }, // {9} ADC1 []
    [16] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC2 }, // {10} ADC2 []
    [17] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC3 }, // {11} ADC3 []
    [18] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC4 }, // {12} ADC4 SDA [SDA]
    [19] = { .ddr=&DDRC, .pin=&PINC, .port=&PORTC, .bit= PC5 } // {13} ADC5 SCL [SCL]
};
#endif  // defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__)  || defined(__AVR_ATmega328P__)

// note: the use of dead code elimination tricks is not standard C. 
static inline __attribute__((always_inline)) uint8_t badPin(uint8_t pin) 
{
    if (pin >= NUM_DIGITAL_PINS) 
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static inline __attribute__((always_inline))
void bitWrite(volatile uint8_t* register_addr, uint8_t bit_offset, bool value_for_bit) 
{
    // Although I/O Registers 0x20 and 0x5F, e.g. PORTn and DDRn should not need 
    // atomic change control it does not harm.
    ATOMIC_BLOCK ( ATOMIC_RESTORESTATE )
    {
        if (value_for_bit) 
        {
            *register_addr |= 1 << bit_offset;
        } 
        else 
        {
            *register_addr &= ~(1 << bit_offset);
        }
    }
}

/* read value from pin number */
static inline __attribute__((always_inline))
bool digitalRead(uint8_t pin_num) 
{
    if (!badPin(pin_num)) 
    {
        return (*pinMap[pin_num].pin >> pinMap[pin_num].bit) & 1;
    }
    else
    {
        return 0;
    }
}

/* set pin value HIGH and LOW */
static inline __attribute__((always_inline))
void digitalWrite(uint8_t pin_num, bool value_for_bit) {
    if (!badPin(pin_num)) bitWrite(pinMap[pin_num].port, pinMap[pin_num].bit, value_for_bit);
}

/* toggle pin number  */
static inline __attribute__((always_inline))
void digitalToggle(uint8_t pin_num) {
    if (!badPin(pin_num)) 
    {
        // Ckeck if pin is in OUTPUT mode befor changing it
        if( ( ( (*pinMap[pin_num].ddr) >> pinMap[pin_num].bit ) & 1) == OUTPUT )  
        {
            digitalWrite(pin_num, !digitalRead(pin_num));
        }
    }
}

/* set pin mode INPUT and OUTPUT */
static inline __attribute__((always_inline))
void pinMode(uint8_t pin_num, bool output_mode) {
    if (!badPin(pin_num)) bitWrite(pinMap[pin_num].ddr, pinMap[pin_num].bit, output_mode);
}

#endif  // DigitalPin_h

