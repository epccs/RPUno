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

#ifdef  __cplusplus
extern "C" {
#endif

#include "icp_buf.h"

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

extern ICP_ISR icp1;

//used as bool to tell if a rising edge will cause capture event, false is falling edge
extern volatile uint8_t rising;

// a timer1 overflow records the icp1 event count
extern volatile uint32_t icp1_event_count_at_OVF;

typedef union { uint16_t word; uint8_t byte[2]; } WORD_2_BYTE; 
typedef union { uint32_t dword; uint16_t word[2]; } LONG_2_WORD;   
 
// timer 1 virtual counter
extern volatile WORD_2_BYTE t1vc;

#ifdef  __cplusplus
}
#endif

#endif // Icp1_h
