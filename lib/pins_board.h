/*
 * Pin definitions for use with Digital IO library
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
 */
#ifndef Pins_Board_h
#define Pins_Board_h

//#include <avr/pgmspace.h>
/* 
    32TQFPpin
        ATTMEGA328P reg-bit
              {PCINT}
                     (Digital IO#)
                            function #notes [RPUno]
    12 PB0 {0} (D 8) ICP1 []
    13 PB1 {1} (D 9) OC1A []
    14 PB2 {2} (D 10) SS OC1B [SS]
    15 PB3 {3} (D 11) MOSI OC2A [MOSI]
    16 PB4 {4} (D 12) MISO [MISO]
    17 PB5 {5} (D 13) SCK [SCK]
    23 PC0 {8} (D 14) ADC0 []
    24 PC1 {9} (D 15) ADC1 []
    25 PC2 {10} (D 16) ADC2 []
    26 PC3 {11} (D 17) ADC3 []
    27 PC4 {12} (D 18) ADC4 SDA [SDA]
    28 PC5 {13} (D 19) ADC5 SCL [SCL]
    19 (D 20) ADC6 []
    22 (D 21) ADC7 []
    9 PD0 {16} (D 0) RXD [RX]
    8 PD1 {17} (D 1) TXD [TX]
    7 PD2 {18} (D 2) INT0 []
    6 PD3 {19} (D 3) INT1 OC2B []
    5 PD4 {20} (D 4) T0 []
    4 PD5 {21} (D 5) T1 OC0B []
    3 PD6 {22} (D 6) OC0A []
    2 PD7 {23} (D 7) []
*/

#define NUM_DIGITAL_PINS            20
#define NUM_ANALOG_INPUTS        8
// analogInputToDigitalPin() takes an AVR analog channel number and returns the digital pin number otherwise -1.
// #define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 14 : -1)
#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)

static const uint8_t LED_BUILTIN = 13; // board has no led but this is the normal place it would be found

// don't use RX0 or TX0 pins they are for the serial communication.
//static const uint8_t RX0 = 0; 
//static const uint8_t TX0 = 1;

static const uint8_t SS   = 10; 
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t SDA = 18;
static const uint8_t SCL = 19;

// these are ADC channels, they do not map to a digital IO function
static const uint8_t PV_IN = 6; // measure PV input voltage at 100/532 V/V
static const uint8_t PWR = 7; // measure Battery at about 2/3 V/V

#endif // Pins_Board_h
