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

extern void initIcp1(uint8_t mode, uint8_t prescaler);

// mode Input Capture Edge Select
#define TRACK_FALLING 0
#define TRACK_RISING 1
#define TRACK_BOTH 2
extern volatile uint8_t icp1_edge_mode;

// prescaler Input Capture Clock Select
#define ICP1_NOCLOCK 0
#define ICP1_MCUCLOCK 1
#define ICP1_MCUDIV8 2
#define ICP1_MCUDIV64 3
#define ICP1_MCUDIV256 4
#define ICP1_MCUDIV1024 5

// only the first 32 bytes can be accessed quickly using the AVR ldd instruction.
// this means that the total array size needs to be held bellow that
// however I want to capture up to 32 events so a struct is not helpful
extern volatile uint8_t event_Byt0[EVENT_BUFF_SIZE];
extern volatile uint8_t event_Byt1[EVENT_BUFF_SIZE];

// status bit zero tells if event is a rising/falling edge
#define RISING 0
extern volatile uint8_t event_status[EVENT_BUFF_SIZE];

// head of icp1 buffer
extern volatile uint8_t icp1_head;

//used as bool to tell if a rising edge will cause capture event, false is falling edge
extern volatile uint8_t rising;

// icp1 pulse count
extern volatile uint32_t icp1_event_count;

// a timer1 overflow records the icp1 event count
extern volatile uint32_t icp1_event_count_at_OVF;

typedef union { uint16_t word; uint8_t byte[2]; } WORD_2_BYTE; 
typedef union { uint32_t dword; uint16_t word[2]; } LONG_2_WORD;   
 
// timer 1 virtual counter
extern volatile WORD_2_BYTE t1vc;

#endif // Icp1_h
