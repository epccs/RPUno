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

// LT3652 has some pins that may be connected to mega328p
#define SHUTDOWN 5
#define DISCONNECT 6
#define FAULT 7

// RPUno board has no led but this is the normal place it would be found
#define LED_BUILTIN 13 

// UART on RPUno is for serial communication (you should never use these pins)
#define RX0 0 
#define TX0 1

// SPI on RPUno
#define SS 10 
#define MOSI 11
#define MISO 12
#define SCK 13

// I2C on RPUno
#define SDA 18
#define SCL 19

// ADC channels
// There are values from 0 to 1023 for 1024 slots where each reperesents 1/1024 of the reference. Last slot has issues
// https://forum.arduino.cc/index.php?topic=303189.0      

// PV_I_ADC1 is a high side current sense on CCtest board
// PV_I_ADC1 voltage is analogRead(PV_I)*(5.0/1024.0)/(0.068*50.0)
#define PV_I 1

// CHRG_ADC2 voltage is analogRead(CHRG_I)*(5.0/1024.0)/(0.068*50.0)
#define CHRG_I 2

// DISCHRG_ADC3 voltage is analogRead(DISCHRG_I)*(5.0/1024.0)/(0.068*50.0)
#define DISCHRG_I 3

// ADC4 and ADC5 are used for I2C with the RPUadpt/RPUftdi/RPUpi shields

// PV_IN_ADC6 voltage is analogRead(PV_V)*(5.0/1024.0)*(532.0/100.0)
#define PV_V 6 

// PWR_ADC7 or Battery voltage is analogRead(PWR_V)*(5.0/1024.0)*(3.0/2.0)
#define PWR_V 7

#endif // Pins_Board_h
