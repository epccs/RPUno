/* 
RPUno SelfTest
Copyright (C) 2017 Ronald Sutherland

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

// Save the Value of the References for ADC converter 
// measure AVCC and put it hear in uV 
#define REF_EXTERN_AVCC 4943800UL
// I am not sure how to measure the 1V1 bandgap, this is just a holding place
#define REF_INTERN_1V1 1100000UL

#define R1 50.0
#define ICP1_TERM 100.0

static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t passing;

void setup(void) 
{
    // Turn Off Curr Sources
    pinMode(CS_EN,OUTPUT);
    digitalWrite(CS_EN,LOW);

    // Turn Off VOUT to shield (e.g. disconnect VIN from shield)
    pinMode(SHLD_VIN_EN,OUTPUT);
    digitalWrite(SHLD_VIN_EN,LOW);

    // Battery disconnect (not used on RPUno^7)
    digitalWrite(DIO7,LOW);
    pinMode(DIO7,OUTPUT);

    // Charge control (not used on RPUno^7)
    digitalWrite(DIO5,LOW);
    pinMode(DIO5,OUTPUT);

    // To Check CC fault state drive pin with a weak pull-up
    digitalWrite(DIO6,HIGH);
    pinMode(DIO6,INPUT);

    // Set plugable DIO to shunt current sources, each has 127 Ohm. (e.g. turn LED's off) 
    pinMode(DIO3,OUTPUT);
    digitalWrite(DIO3,LOW);
    pinMode(DIO4,OUTPUT);
    digitalWrite(DIO4,LOW);
    pinMode(DIO10,OUTPUT);
    digitalWrite(DIO10,LOW);
    pinMode(DIO11,OUTPUT);
    digitalWrite(DIO11,LOW);
    pinMode(DIO12,OUTPUT);
    digitalWrite(DIO12,LOW);
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
    
    // set the referances and save them in EEPROM
    ref_extern_avcc_uV = REF_EXTERN_AVCC;
    ref_intern_1v1_uV = REF_INTERN_1V1;
    while ( !WriteEeReferenceId() ) {};
    while ( !WriteEeReferenceAvcc() ) {};
    while ( !WriteEeReference1V1() ) {};
}

void test(void)
{
    // Info
    printf_P(PSTR("Self Test date: %s\r\n"), __DATE__);
    
    // I2C is used to read RPU bus manager address
    if (rpu_addr == '1')
    {
        printf_P(PSTR("I2C provided address 0x31 from RPU bus manager\r\n"));
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C failed, or address not 0x31 from RPU bus manager\r\n"));
        return;
    }

    // +5V is used as the ADC reference
    printf_P(PSTR("REF_EXTERN_AVCC saved in eeprom: %1.3f V\r\n"), (ref_extern_avcc_uV/1.0E6));

    // Current sources are off but 22MA_DIO3 and 22MA_DIO11 are on
    // Input current
    _delay_ms(1000) ; // busy-wait delay 
    float input_i = analogRead(PWR_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CS_EN==off: %1.3f A\r\n"), input_i);
    if (input_i > 0.08) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to high.\r\n"));
        return;
    }
    if (input_i < 0.04) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to low.\r\n"));
        return;
    }

    // Input voltage
    float input_v = analogRead(PWR_V)*((ref_extern_avcc_uV/1.0E6)/1024.0)*(115.8/15.8);
    printf_P(PSTR("PWR at: %1.3f V\r\n"), input_v);
    if (input_v > 14.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input voltage is to high.\r\n"));
        return;
    }
    if (input_v < 12.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input voltage is to low.\r\n"));
        return;
    }
    
    // ADC0 and ADC1 with current sources off
    float adc0_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC0 /w shunts on & CS_EN==off: %1.3f V\r\n"), adc0_v);
    float adc1_v = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC1 /w shunts on & CS_EN==off: %1.3f V\r\n"), adc1_v);
    if ( (adc0_v > 0.01)  || (adc1_v > 0.01) )
    { 
        passing = 0; 
        printf_P(PSTR(">>> ADC is to high, is the self-test wiring right?\r\n"));
        return;
    }

    // ICP1 pin is inverted from to the plug interface, which should have zero mA on its 100 Ohm Termination now
    printf_P(PSTR("ICP1 /w 0mA on plug termination reads: %d \r\n"), digitalRead(ICP1));
    if (!digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be high.\r\n"));
    }
    
    // enable the current sources
    digitalWrite(CS_EN,HIGH);
    _delay_ms(1000) ; // busy-wait delay
    float input_withcs_i = analogRead(PWR_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CS_EN==on: %1.3f A\r\n"), input_withcs_i);
    
    // ADC0 hos ADC1's curr source, but DIO11 shunts ADC0's curr source, 
    // DIO3 and DIO10 shunt DIO3's curr source, 
    // DIO12 and DIO13 shunt DIO11's curr source.
    float adc1_22mA_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("ADC1_22mA source on R1: %1.3f A\r\n"), adc1_22mA_i);
    
    // ADC1 has ICP1's 10mA source
    float icp1_10mA_i = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1's 10mA on ICP1_TERM: %1.3f A\r\n"), icp1_10mA_i);
    
    // ICP1 pin is inverted from to the plug interface, which should have 10 mA on its 100 Ohm Termination now
    printf_P(PSTR("ICP1 /w 10mA on plug termination reads: %d \r\n"), digitalRead(ICP1));
    if (digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be low with 10mA.\r\n"));
    }
    
    // DIO13 and DIO12 high-z to add DIO11's curr source to R1
    pinMode(DIO13,INPUT);
    pinMode(DIO12,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio11_22mA_adc1_22mA_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("Add DIO11_22MA curr source to R1: %1.3f A\r\n"), dio11_22mA_adc1_22mA_i);
    if (dio11_22mA_adc1_22mA_i < 0.040) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO11_22MA curr is to low.\r\n"));
    }
    if (dio11_22mA_adc1_22mA_i > 0.047) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO11_22MA curr is to high.\r\n"));
    }

    // DIO13 high-z and DIO12 shunting curr 
    pinMode(DIO13,INPUT);
    pinMode(DIO12,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio12_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO12 shunting DIO11_22mA: %1.3f A\r\n"), dio12_shunt_i);
    if (dio12_shunt_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO12 is not shunting.\r\n"));
    }

    // DIO12 high-z and DIO13 shunting most of the digital curr source from going through R1
    pinMode(DIO12,INPUT);
    pinMode(DIO13,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio13_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO13 shunting DIO11_22mA: %1.3f A\r\n"), dio13_shunt_i);
    if (dio13_shunt_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO13 is not shunting.\r\n"));
    }
    pinMode(DIO12,OUTPUT);

    // DIO3 and DIO10 high-z to add DIO3's curr source to R1
    pinMode(DIO3,INPUT);
    pinMode(DIO10,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio3_22mA_adc1_22mA_i  = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("Add DIO3_22MA curr source to R1: %1.3f A\r\n"), dio3_22mA_adc1_22mA_i);
    if (dio3_22mA_adc1_22mA_i < 0.040) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO3_22MA curr is to low.\r\n"));
    }
    if (dio3_22mA_adc1_22mA_i > 0.047) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO3_22MA curr is to high.\r\n"));
    }

    // DIO3 high-z and DIO10 shunting curr 
    pinMode(DIO3,INPUT);
    pinMode(DIO10,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio10_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO10 shunting DIO3_22mA: %1.3f A\r\n"), dio10_shunt_i);
    if (dio10_shunt_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO10 is not shunting.\r\n"));
    }

    // DIO10 high-z and DIO3 shunting curr
    pinMode(DIO10,INPUT);
    pinMode(DIO3,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio3_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO3 shunting DIO3_22mA: %1.3f A\r\n"), dio3_shunt_i);
    if (dio3_shunt_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO3 is not shunting.\r\n"));
    }

    // DIO4 high-z to add icp1's 10mA and 17mA curr source to ICP1's PL plug.
    pinMode(DIO4,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float icp1_17mA_icp1_10mA_i = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1 10mA + 17mA curr source on ICP1's PL plug: %1.3f A\r\n"), icp1_17mA_icp1_10mA_i);
    if (icp1_17mA_icp1_10mA_i < 0.025) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 10mA & 17mA curr source is to low\r\n"));
    }
    if (icp1_17mA_icp1_10mA_i > 0.031) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 10mA & 16mA curr source is to high.\r\n"));
    }
    pinMode(DIO4,OUTPUT);

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
    if (passing)
    {
        pinMode(DIO3,OUTPUT);
        digitalWrite(DIO3,LOW);
        pinMode(DIO4,INPUT);
        pinMode(DIO10,OUTPUT);
        digitalWrite(DIO10,LOW);
        pinMode(DIO11,OUTPUT);
        digitalWrite(DIO11,LOW);
        pinMode(DIO12,OUTPUT);
        digitalWrite(DIO12,LOW);
        pinMode(DIO13,OUTPUT);
        digitalWrite(DIO13,LOW);
    }
    else
    {
        pinMode(DIO3,OUTPUT);
        digitalWrite(DIO3,LOW);
        pinMode(DIO4,OUTPUT);
        digitalWrite(DIO4,LOW);
        pinMode(DIO10,OUTPUT);
        digitalWrite(DIO10,LOW);
        pinMode(DIO11,INPUT);
        pinMode(DIO12,OUTPUT);
        digitalWrite(DIO12,LOW);
        pinMode(DIO13,OUTPUT);
        digitalWrite(DIO13,LOW);
    }
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        digitalToggle(CS_EN);
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

int main(void)
{
    setup(); 
    
    passing = 1;
    test();
    led_setup_after_test();
    
    while (1) 
    {
        blink();
    }    
}

