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

    // SMBus needs connected to RPUno I2C master for testing, it is for write_i2c_block_data and read_i2c_block_data from
    //  https://git.kernel.org/pub/scm/utils/i2c-tools/i2c-tools.git/tree/py-smbus/smbusmodule.c
    // write_i2c_block_data sends a command byte and data to the slave 
    uint8_t smbus_address = 0x2A;
    uint8_t length = 2;
    uint8_t wait = 1;
    uint8_t sendStop = 1;
    uint8_t txBuffer[2] = {0x00,0x00}; //comand 0x00 should Read the mulit-drop bus addr;
    uint8_t twi_returnCode = twi_writeTo(smbus_address, txBuffer, length, wait, sendStop); 
    if (twi_returnCode != 0)
    {
        passing = 0; 
        printf_P(PSTR(">>> SMBus write failed, twi_returnCode: %d\r\n"), twi_returnCode);
        return;
    }
    
    // read_i2c_block_data sends a command byte and then a repeated start followed by reading the data 
    uint8_t cmd_length = 1; // one byte command is sent befor read with the read_i2c_block_data
    sendStop = 0; // a repeated start happens after the command byte is sent
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
    if ( (rxBuffer[0] == 0x0) && (rxBuffer[1] == '1') )
    {
        printf_P(PSTR("SMBUS cmd %d provided address %d from manager\r\n"), rxBuffer[0], rxBuffer[1]);
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> SMBUS wrong addr %d for cmd %d\r\n"), rxBuffer[1], rxBuffer[0]);
    }

    // SPI loopback at R-Pi header. e.g., drive MISO/DIO12 to test MOSI/DIO11.
    // with selft test wirring MISO/DIO12 can shunt the current from CS3 to bypass yellow LED D2
    pinMode(MISO,OUTPUT);
    digitalWrite(MISO,HIGH);
    digitalWrite(CS3_EN,LOW);
    // and MOSI/DIO11 can shunt the current from CS0 to bypass red LED D3
    pinMode(MOSI,INPUT);
    digitalWrite(MOSI,LOW); // turn off the weak pullup, the RPUpi board has a 3k pullup resistor 
    digitalWrite(CS0_EN,LOW); // a HITH would drive the MOSI pin with Self-Test wiring
    digitalWrite(CS1_EN,HIGH); // add bias to R1 so MOSI loopback can be seen
    digitalWrite(CS2_EN,HIGH); // add more bias to R1 so MOSI loopback can be seen
    _delay_ms(50) ; // busy-wait delay
    uint8_t miso_rd = digitalRead(MISO);
    uint8_t mosi_rd = digitalRead(MOSI);
    if (miso_rd && mosi_rd) 
    { 
        printf_P(PSTR("MISO loopback to MOSI == HIGH\r\n"));
    }
    else 
    { 
        passing = 0; 
        printf_P(PSTR(">>> MISO %d did not loopback to MOSI %d\r\n"), miso_rd, mosi_rd);
    }

    digitalWrite(MISO,LOW);
    _delay_ms(50) ; // busy-wait delay
    miso_rd = digitalRead(MISO);
    mosi_rd = digitalRead(MOSI);
    if ( (!miso_rd) && (!mosi_rd) ) 
    { 
        printf_P(PSTR("MISO loopback to MOSI == LOW\r\n"));
    }
    else 
    { 
        passing = 0; 
        printf_P(PSTR(">>> MISO %d did not loopback to MOSI %d\r\n"), miso_rd, mosi_rd);
    }
    digitalWrite(CS1_EN,LOW);
    digitalWrite(CS2_EN,LOW);

    // My R-Pi Shutdown is on BCM6 (pin31) and that loops back into SCK on the test header
    pinMode(SCK,INPUT);
    digitalWrite(SCK,LOW); // turn off the weak pullup, the RPUpi board has a 3k pullup resistor 
    pinMode(DIO15,OUTPUT);
    digitalWrite(DIO15,HIGH); // this will bias icp1 input over the self-test wiring so green D1 can allow digital voltage on SCK
    digitalWrite(CS_ICP1_EN,LOW); // Green LED
    _delay_ms(50) ; // busy-wait delay
    uint8_t sck_rd = digitalRead(SCK);
    if (sck_rd ) 
    { 
        printf_P(PSTR("SCK with Shutdown loopbakc == HIGH\r\n"));
    }
    else 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Shutdown HIGH did not loopback to SCK %d\r\n"), sck_rd);
    }

    // To cause a shutdown I can send the I2C command 5 with data 1
    // note the RPUno can monitor its input power to verify the R-Pi is in shutdown and turn off power to the shield VIN    
    uint8_t i2c_address = 0x29;
    length = 2;
    wait = 1;
    sendStop = 0; // use a repeated start after write
    txBuffer[0] = 0x05;
    txBuffer[1] = 0x01; //comand 0x05 should pull Shutdown low
    twi_returnCode = twi_writeTo(i2c_address, txBuffer, length, wait, sendStop); 
    if (twi_returnCode != 0)
    {
        passing = 0; 
        printf_P(PSTR(">>> I2C cmd 5 write fail, twi_returnCode: %d\r\n"), twi_returnCode);
        return;
    }
    rxBuffer[0]=0;
    rxBuffer[1]=0;
    sendStop = 1;
    bytes_read = twi_readFrom(i2c_address, rxBuffer, length, sendStop);
    if ( bytes_read != length )
    {
        passing = 0; 
        printf_P(PSTR(">>> I2C read missing %d bytes \r\n"), (length-bytes_read) );
    }
    if ( (txBuffer[0] == rxBuffer[0]) && (txBuffer[1] == rxBuffer[1]) )
    {
        printf_P(PSTR("I2C Shutdown cmd is clean {%d, %d}\r\n"), rxBuffer[0], rxBuffer[1]);
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C Shutdown cmd echo bad {%d, %d}\r\n"), rxBuffer[0], rxBuffer[1]);
    }

    _delay_ms(50) ; // busy-wait delay
    sck_rd = digitalRead(SCK);
    if (!sck_rd) 
    { 
        printf_P(PSTR("SCK with Shutdown loopbakc == LOW\r\n"));
    }
    else 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Shutdown LOW did not loopback to SCK %d\r\n"), sck_rd);
    }
    pinMode(DIO15,INPUT);
    digitalWrite(DIO15,LOW); 

    // Manager should save the shutdown detected after SHUTDOWN_TIME timer runs (see rpubus_manager_state.h in Remote fw for value)
    _delay_ms(1100) ; // busy-wait delay
    sendStop = 0; // use a repeated start after write
    txBuffer[0] = 0x04;
    txBuffer[1] = 0xFF; //comand 0x04 will return the value 0x01 (0xff is a byte for the ISR to replace)
    twi_returnCode = twi_writeTo(i2c_address, txBuffer, length, wait, sendStop);
    if (twi_returnCode != 0)
    {
        passing = 0; 
        printf_P(PSTR(">>> I2C cmd 4 write fail, twi_returnCode: %d\r\n"), twi_returnCode);
        return;
    }
    rxBuffer[0]=0;
    rxBuffer[1]=0;
    sendStop = 1;
    bytes_read = twi_readFrom(i2c_address, rxBuffer, length, sendStop);
    if ( bytes_read != length )
    {
        passing = 0; 
        printf_P(PSTR(">>> I2C read missing %d bytes \r\n"), (length-bytes_read) );
    }
    if ( (txBuffer[0] == rxBuffer[0]) && (0x1 == rxBuffer[1]) )
    {
        printf_P(PSTR("I2C Shutdown Detect cmd is clean {%d, %d}\r\n"), rxBuffer[0], rxBuffer[1]);
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C Shutdown Detect cmd echo bad {%d, %d}\r\n"), rxBuffer[0], rxBuffer[1]);
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

    pinMode(DIO10,INPUT); 
    digitalWrite(DIO10,LOW);
    pinMode(DIO11,INPUT);
    digitalWrite(DIO11,LOW); // is MOSI in hi-z
    pinMode(DIO12,INPUT);
    digitalWrite(DIO12,LOW); // is MISO in hi-z
    pinMode(DIO13,INPUT);
    digitalWrite(DIO13,LOW); // is SCK in hi-z
    pinMode(DIO14,INPUT);
    pinMode(DIO15,INPUT);
    pinMode(DIO16,INPUT);
    pinMode(DIO17,INPUT);
    
    if (passing)
    {
        digitalWrite(CS_ICP1_EN,HIGH); // Green LED
    }
    else
    {
        digitalWrite(CS0_EN,HIGH); // Red LED
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

