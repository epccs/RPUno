/*
    Input Capture Unit Buffer
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

*/

#include <util/atomic.h>
#include "icp_buf.h"

// Used to make a second buffer for reports, since the ISR will change the buffer it is using at any time.
// from is a live ICP buffer filled by an ISR 
// warning the from buffer is not checked to see if it has enough valid events 
void double_buffer_copy( ICP_ISR* from, ICP* to, uint8_t num_of_events_needed)
{
    // copy at least the number of events needed
    // cast to integers allows use of two's-complement math
    // ->  follow pointer to structure (it has been a few years ... decades .. since doing this with C)
    to->head =(uint8_t)( (int8_t)(from->head) - (int8_t)(num_of_events_needed+1) ) & (ICP_EVENT_BUFF_MASK);
    
    // now copy the event buffer, from->head may advance but when we catch up to it the copy is done
    uint8_t i=0;
    while ((to->head != from->head) | (i < num_of_events_needed) ) 
    {
        ATOMIC_BLOCK ( ATOMIC_RESTORESTATE )
        {
            to->head = (to->head +1 ) & (ICP_EVENT_BUFF_MASK);
            to->event.Byt0[to->head] = from->event.Byt0[to->head];
            to->event.Byt1[to->head] = from->event.Byt1[to->head];
            to->event.status[to->head] = from->event.status[to->head];
            to->count = from->count;
        }
        if (i < num_of_events_needed) i++;
    }
}



