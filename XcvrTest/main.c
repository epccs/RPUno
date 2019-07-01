/* 
Transceiver Test
Copyright (C) 2019 Ronald Sutherland

This Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with the Arduino DigitalIO Library.  If not, see
<http://www.gnu.org/licenses/>.
*/ 

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#include "../lib/timers.h"
#include "../lib/uart.h"
#include "../lib/twi.h"
#include "../lib/adc.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Adc/references.h"

#define BLINK_DELAY 1000UL

static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t passing;

void setup(void) 
{
    // Turn Off Current Sources
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW); // Red LED
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);
    pinMode(CS2_EN,OUTPUT);
    digitalWrite(CS2_EN,LOW);
    pinMode(CS3_EN,OUTPUT);
    digitalWrite(CS3_EN,LOW);
    pinMode(CS_ICP1_EN,OUTPUT);
    digitalWrite(CS_ICP1_EN,LOW); // Green LED
    
    // Turn Off VOUT to shield (e.g. disconnect VIN from shield)
    pinMode(SHLD_VIN_EN,OUTPUT);
    digitalWrite(SHLD_VIN_EN,LOW);

    pinMode(DIO10,OUTPUT);
    digitalWrite(DIO10,LOW);
    pinMode(DIO13,OUTPUT);
    digitalWrite(DIO13,LOW);

    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);

    /* Initialize I2C, with the internal pull-up*/
    twi_init(TWI_PULLUP);

    // Enable global interrupts to start TIMER0 and UART
    sei(); 

    blink_started_at = millis();

    rpu_addr = get_Rpu_address();
    blink_delay = BLINK_DELAY;

    // blink fast if a default address from RPU manager not found
    if (rpu_addr == 0)
    {
        rpu_addr = '0';
        blink_delay = BLINK_DELAY/4;
    }
}

void test(void)
{
    // Info from some Predefined Macros
    printf_P(PSTR("RPUpi Transceiver Test date: %s\r\n"), __DATE__);
    printf_P(PSTR("avr-gcc --version: %s\r\n"),__VERSION__);
    
    // I2C is used to read serial bus manager address 
    if (rpu_addr == '1')
    {
        printf_P(PSTR("I2C provided address 0x31 from manager\r\n"));
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C failed, or address not 0x31 from manager\r\n"));
        return;
    }

    // SMBus needs connected to RPUno for testing, it works a little different than I2C
    uint8_t smbus_address = 0x2A;
    uint8_t length = 2;
    uint8_t wait = 1;
    uint8_t sendStop = 1; // see write_i2c_block_data from https://git.kernel.org/pub/scm/utils/i2c-tools/i2c-tools.git/tree/py-smbus/smbusmodule.c
    uint8_t txBuffer[2] = {0x00,0x00}; //comand 0x00 should Read the mulit-drop bus addr;
    uint8_t twi_returnCode = twi_writeTo(smbus_address, txBuffer, length, wait, sendStop); 
    if (twi_returnCode != 0)
    {
        passing = 0; 
        printf_P(PSTR(">>> SMBus write failed, twi_returnCode: %d\r\n"), twi_returnCode);
        return;
    }
    
    //  https://git.kernel.org/pub/scm/utils/i2c-tools/i2c-tools.git/tree/py-smbus/smbusmodule.c
    uint8_t cmd_length = 1; // one byte command is sent befor read with the read_i2c_block_data
    sendStop = 0; // the data is taken after the command is sent and a repeated start happens
    txBuffer[0] = 0x00; //comand 0x00 matches the above write command
    twi_returnCode = twi_writeTo(smbus_address, txBuffer, cmd_length, wait, sendStop); 
    if (twi_returnCode != 0)
    {
        passing = 0; 
        printf_P(PSTR(">>> SMBus read cmd fail, twi_returnCode: %d\r\n"), twi_returnCode);
        return;
    }
    uint8_t rxBuffer[2];
    sendStop = 1;
    uint8_t bytes_read = twi_readFrom(smbus_address, rxBuffer, length, sendStop);
    if ( bytes_read != length )
    {
        passing = 0; 
        printf_P(PSTR(">>> SMBus read missing %d bytes \r\n"), (length-bytes_read) );
        return;
    }
    if (rxBuffer[1] == '1')
    {
        printf_P(PSTR("SMBUS provided address 0x31 from manager\r\n"));
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> SMBUS gave wrong address. e.g., not 0x31\r\n"));
        return;
    }

    // The Transceiver Test is WIP

    // final test status
    if (passing)
    {
        printf_P(PSTR("[PASS]\r\n"));
    }
    else
    {
        printf_P(PSTR("[FAIL]\r\n"));
    }
    printf_P(PSTR("\r\n\r\n\r\n"));
}

void led_setup_after_test(void)
{
    // Turn Off Curr Sources
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW); // Red LED
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);
    pinMode(CS2_EN,OUTPUT);
    digitalWrite(CS2_EN,LOW);
    pinMode(CS3_EN,OUTPUT);
    digitalWrite(CS3_EN,LOW);
    pinMode(CS_ICP1_EN,OUTPUT);
    digitalWrite(CS_ICP1_EN,LOW); // Green LED

    pinMode(DIO10,OUTPUT);
    digitalWrite(DIO10,LOW);
    pinMode(DIO11,OUTPUT);
    digitalWrite(DIO11,LOW);
    pinMode(DIO12,OUTPUT);
    digitalWrite(DIO12,LOW);
    pinMode(DIO13,OUTPUT);
    digitalWrite(DIO13,LOW);
    pinMode(DIO14,INPUT);
    pinMode(DIO15,INPUT);
    pinMode(DIO16,INPUT);
    pinMode(DIO17,INPUT);
    
    if (passing)
    {
        digitalWrite(CS_ICP1_EN,HIGH); // Green LED
        pinMode(DIO13,INPUT);
    }
    else
    {
        digitalWrite(CS0_EN,HIGH); // Red LED
        pinMode(DIO11,INPUT);
    }
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        if (passing)
        {
            digitalToggle(CS_ICP1_EN); // Green LED
        }
        else
        {
            digitalToggle(CS0_EN); // Red LED
        }
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

int main(void)
{
    setup(); 
    
    if ( ! LoadAnalogRefFromEEPROM() )
    {
        printf_P(PSTR("{\"err\":\"AdcRefNotInEeprom\"}\r\n"));
        passing = 0;
    }
    else
    {
        passing = 1;
        test();
    }
    led_setup_after_test();
    
    while (1) 
    {
        blink();
    }    
}

