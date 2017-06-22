/*
  Input Capture Unit Buffer
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

#ifndef Icp_buf_h
#define Icp_buf_h

// only the first 32 bytes can be accessed quickly using the AVR ldd instruction.
// this means that the total array size needs to be held bellow that
// however I want to capture up to 32 events so a struct is not helpful
#define ICP_EVENT_BUFF_SIZE 32
#define ICP_EVENT_BUFF_MASK (ICP_EVENT_BUFF_SIZE - 1)

#if ( ICP_EVENT_BUFF_SIZE & ICP_EVENT_BUFF_MASK )
    #error event buffer size is not a power of 2
#endif

// bit zero of status tells if event is a rising/falling edge
#define RISING 0

struct icp_event_struct_for_isr
{
    volatile uint8_t Byt0[ICP_EVENT_BUFF_SIZE];
    volatile uint8_t Byt1[ICP_EVENT_BUFF_SIZE];
    volatile uint8_t status[ICP_EVENT_BUFF_SIZE];
};

typedef  struct icp_buf_struct_for_isr
{
    struct icp_event_struct_for_isr event;     // nested structure
    volatile uint8_t head;
    volatile uint32_t count;
} ICP_ISR;

struct icp_event_struct_local
{
    uint8_t Byt0[ICP_EVENT_BUFF_SIZE];
    uint8_t Byt1[ICP_EVENT_BUFF_SIZE];
    uint8_t status[ICP_EVENT_BUFF_SIZE];
};

typedef  struct icp_buf_struct_local
{
    struct icp_event_struct_local event;     // nested structure
    uint8_t head;
    uint32_t count;
} ICP;

extern void double_buffer_copy( ICP_ISR* from, ICP* to, uint8_t num_of_events_needed);

#endif // Icp_buf_h
