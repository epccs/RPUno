/*
references is a library used to load and set analog conversion references in EEPROM. 
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

  I believe the LGPL is used in things like libraries and allows you to include them in 
  application code without the need to release the application source while GPL requires 
  that all modifications be provided as source when distributed.
*/
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <avr/eeprom.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pins_board.h"
#include "references.h"

//The EEPROM memory usage is as follows. 
#define EE_ANALOG_BASE_ADDR 30
// each setting is at this byte offset
#define EE_ANALOG_ID 0
#define EE_ANALOG_REF_EXTERN_AVCC 2
#define EE_ANALOG_REF_INTERN_1V1 6

#define REF_EXTERN_AVCC_MAX 5500000UL
#define REF_EXTERN_AVCC_MIN 4500000UL
#define REF_INTERN_1V1_MAX 1300000UL
#define REF_INTERN_1V1_MIN 900000UL

uint8_t ref_loaded;
uint32_t ref_extern_avcc_uV;
uint32_t ref_intern_1v1_uV;

uint8_t IsValidValForAvccRef(uint32_t *value) 
{
    if ( ((*value > REF_EXTERN_AVCC_MIN) && (*value < REF_EXTERN_AVCC_MAX)) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t IsValidValFor1V1Ref(uint32_t *value) 
{
    if ( ((*value > REF_INTERN_1V1_MIN) && (*value < REF_INTERN_1V1_MAX)) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t WriteEeReferenceId() 
{
    uint16_t ee_id = eeprom_read_word((uint16_t*)(EE_ANALOG_BASE_ADDR+EE_ANALOG_ID));
    if ( eeprom_is_ready() )
    {
        uint16_t value = 0x4144; // 'A' is 0x41 and 'D' is 0x44;
        if (ee_id != value)
        {
            eeprom_write_word( (uint16_t *)(EE_ANALOG_BASE_ADDR+EE_ANALOG_ID), value);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t WriteEeReferenceAvcc() 
{
    uint32_t ee_ref_extern_avcc_uV = eeprom_read_dword((uint32_t*)(EE_ANALOG_BASE_ADDR+EE_ANALOG_REF_EXTERN_AVCC)); 
    if ( eeprom_is_ready() )
    {
        if (ee_ref_extern_avcc_uV != ref_extern_avcc_uV)
        {
            eeprom_write_dword( (uint32_t *)(EE_ANALOG_BASE_ADDR+EE_ANALOG_REF_EXTERN_AVCC), ref_extern_avcc_uV);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t WriteEeReference1V1() 
{
    uint32_t ee_ref_intern_1v1_uV = eeprom_read_dword((uint32_t*)(EE_ANALOG_BASE_ADDR+EE_ANALOG_REF_INTERN_1V1)); 
    if ( eeprom_is_ready() )
    {
        if (ee_ref_intern_1v1_uV != ref_intern_1v1_uV)
        {
            eeprom_write_dword( (uint32_t *)(EE_ANALOG_BASE_ADDR+EE_ANALOG_REF_INTERN_1V1), ref_intern_1v1_uV);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t LoadAnalogRefFromEEPROM() 
{
    uint16_t id = eeprom_read_word((uint16_t*)(EE_ANALOG_BASE_ADDR+EE_ANALOG_ID));
    if (id == 0x4144) // 'A' is 0x41 and 'D' is 0x44
    {
        ref_extern_avcc_uV = eeprom_read_dword((uint32_t*)(EE_ANALOG_BASE_ADDR+EE_ANALOG_REF_EXTERN_AVCC)); 
        ref_intern_1v1_uV = eeprom_read_dword((uint32_t*)(EE_ANALOG_BASE_ADDR+EE_ANALOG_REF_INTERN_1V1));
        ref_loaded = 1;
        return 1;
    }
    else
    {
        ref_loaded = 0;
        return 0;
    }
}
