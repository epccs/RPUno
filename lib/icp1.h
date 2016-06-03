/*
  Input Capture Unit 1 used to timestamp an event (rising/falling edge) time of occurrence
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

#ifndef Icp1_h
#define Icp1_h

#define EVENT_BUFF_SIZE 32
#define EVENT_BUFF_MASK (EVENT_BUFF_SIZE - 1)

#if ( EVENT_BUFF_SIZE & EVENT_BUFF_MASK )
    #error event buffer size is not a power of 2
#endif

extern void initIcp1(void) ;

// only the first 32 bytes can be accessed quickly using the AVR ldd instruction.
// this means that the total array size needs to be held bellow that
// however I want to capture up to 32 events so a struct is not helpful
extern volatile uint8_t event_Byt0[EVENT_BUFF_SIZE];
extern volatile uint8_t event_Byt1[EVENT_BUFF_SIZE];
extern volatile uint8_t event_Byt2[EVENT_BUFF_SIZE];
extern volatile uint8_t event_Byt3[EVENT_BUFF_SIZE];
extern volatile uint8_t event_BytChk[EVENT_BUFF_SIZE];

// status bit zero tells if event is a rising/falling edge or how the capture value is likly damaged 
#define RISING 0
#define TOV1_WHILE_IN_CAPT_ISR 1
#define ICF1_WHILE_IN_OVF_ISR 2
#define BYTCHK_ERROR_AT_USR_CPY 3
extern volatile uint8_t event_status[EVENT_BUFF_SIZE];

// head of icp1 buffer
extern volatile uint8_t icp1_head;

//used as bool to tell if a rising edge will cause capture event, false is falling edge
extern volatile uint8_t rising;

// icp1 pulse count
extern volatile uint32_t icp1_event_count;

typedef union { uint16_t word; uint8_t byte[2]; } WORD_2_BYTE; 
typedef union { uint32_t dword; uint16_t word[2]; } LONG_2_WORD;   
 
#endif // Icp1_h
