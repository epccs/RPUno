/*
  Analog to Digital Converter  
  Copyright (c) 2016 Ronald S. Sutherland

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

#ifndef Adc_h
#define Adc_h

// define adc referance modes (note DEFAULT is gone, use EXTERNAL_AVCC in its place)
// EXTERNAL_AVCC: connects the analog reference to AVCC power supply. 
// INTERNAL: built-in reference 1.1V reference on ATmega328p and 2.56V on ATmega8
// INTERNAL1V1: a built-in 1.1V reference on ATmega1284p, ATmega1280
// INTERNAL2V56: a built-in 2.56V reference ATmega1284p ATmega1280
// EXTERNAL: the voltage applied to the AREF pin
#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define EXTERNAL_AVCC 0
#define EXTERNAL 1
#define INTERNAL 2
#else  
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define INTERNAL1V1 2
#define INTERNAL2V56 3
#else
#define INTERNAL 3
#endif
#define EXTERNAL_AVCC 1
#define EXTERNAL 0
#endif

extern void initAdcSingleConversion(uint8_t reference);
extern void analogReference(uint8_t mode);
extern int analogRead(uint8_t channel);

extern volatile uint8_t analog_reference;

#endif // Adc_h
