/*
spi-cmd is a library that enables SPI hardware as a slave that echos
Copyright (C) 2019 Ronald Sutherland

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
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/twi.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "spi-cmd.h"

volatile uint8_t spi_data;

// SPDR is used to shift data in and out, so it will echo back on SPI what was last sent.
// A copy is made for local use (i.e. SPDR will shift when the master drives SCK)
ISR(SPI_STC_vect)
{
    // The system is single buffered in the transmit direction and 
    // double buffered in the receive direction.
    spi_data = SPDR;
}

void spi_init(void)
{
    // SPI slave setup 
    pinMode(MOSI, INPUT);
    digitalWrite(MOSI,HIGH);           // weak pull up 
    pinMode(MISO, OUTPUT);
    pinMode(SCK, INPUT);             // SPI slave is cloced by the master
    digitalWrite(SCK,HIGH);           // weak pull up 
    pinMode(nSS, INPUT);             // in slave mode nSS is used for synchronization (e.g. use CE0 from Raspberry Pi)
    digitalWrite(nSS,HIGH);           // weak pull up 
}

void EnableSpi(void)
{
    if ( (command_done == 10) )
    {
        // check arg[0] is not ('UP' or 'DOWN')
        if ( !( (strcmp_P( arg[0], PSTR("UP")) == 0) || (strcmp_P( arg[0], PSTR("DOWN")) == 0) ) ) 
        {
            printf_P(PSTR("{\"err\":\"SpiNaMode\"}\r\n"));
            initCommandBuffer();
            return;
        }
        if (strcmp_P( arg[0], PSTR("UP")) == 0 ) 
        {
            // DORD bit zero transmits MSB of SPDR first
            // MSTR bit zero slave SPI mode
            // CPOL bit zero idle while SCK is low (active high)
            // CPHA bit zero sample data on active going SCK edge and setup on the non-active going edge
            SPCR = (1<<SPE)|(1<<SPIE);       // SPI Enable and SPI interrupt enable bit
            SPDR = 0;                        // SPI data register used for shifting data
            printf_P(PSTR("{\"SPI\":\"UP\"}\r\n"));
            initCommandBuffer();
        }
        else
        {
            SPCR = 0;                        // SPI Disanable SPI
            SPDR = 0;                        // SPI data register used for shifting data
            printf_P(PSTR("{\"SPI\":\"DOWN\"}\r\n"));
            initCommandBuffer();
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"SpiCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

