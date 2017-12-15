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
#define REF_EXTERN_AVCC 5008600UL
// ref_intern_1v1_uV is calculated based on the above value and the ICP1 PL resistor


#define R1 50.0
#define ICP1_TERM 100.0

static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t passing;

void setup(void) 
{
    // Turn Off 22MA_DIO3, 22MA_DIO11, 17MA_ICP1 Curr Sources
    pinMode(CS_EN,OUTPUT);
    digitalWrite(CS_EN,LOW);
    
    // Turn Off 22MA_A0 Curr Source
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW);
    
    // Turn Off 22MA_A1 Curr Source
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);

    // Turn Off 10MA PL Curr Source
    pinMode(CS_ICP1_10MA_EN,OUTPUT);
    digitalWrite(CS_ICP1_10MA_EN,LOW);
    
    // Turn Off VOUT to shield (e.g. disconnect VIN from shield)
    pinMode(SHLD_VIN_EN,OUTPUT);
    digitalWrite(SHLD_VIN_EN,LOW);

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
}

void test(void)
{
    // Info
    printf_P(PSTR("Self Test date: %s\r\n"), __DATE__);
    
    // I2C is used to read serial bus manager address 
    if (rpu_addr == '1')
    {
        printf_P(PSTR("I2C provided address 0x31 from serial bus manager\r\n"));
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C failed, or address not 0x31 from serial bus manager\r\n"));
        return;
    }

    // With current sources off measure input current
    _delay_ms(1000) ; // busy-wait to let the 1uF settle
    float input_i = analogRead(PWR_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CS_EN==off: %1.3f A\r\n"), input_i);
    if (input_i > 0.025) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to high.\r\n"));
        return;
    }
    if (input_i < 0.01) 
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
    printf_P(PSTR("ADC0 without curr in R1: %1.3f V\r\n"), adc0_v);
    float adc1_v = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC1 without curr in PL for ICP1: %1.3f V\r\n"), adc1_v);
    if ( (adc0_v > 0.01)  || (adc1_v > 0.01) )
    { 
        passing = 0; 
        printf_P(PSTR(">>> ADC is to high, is the self-test wiring right?\r\n"));
        return;
    }

    // ICP1 pin is inverted from to the plug interface, which should have zero mA on its 100 Ohm Termination now
    printf_P(PSTR("ICP1's PL input has 0mA input and reads: %d \r\n"), digitalRead(ICP1));
    if (!digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be high.\r\n"));
    }
    
    // enable 22MA_A0 which DIO11 can shunt with selftest wiring
    digitalWrite(CS0_EN,HIGH);
    pinMode(DIO11,INPUT);
    _delay_ms(100); // busy-wait delay
    
    // R1 hos 22MA_A0 on it
    float adc0_22mA_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("22MA_A0 source on R1: %1.3f A\r\n"), adc0_22mA_i);
    if (adc0_22mA_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_A0 curr is to low.\r\n"));
        return;
    }
    if (adc0_22mA_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_A0 curr is to high.\r\n"));
        return;
    }
    digitalWrite(CS0_EN,LOW);
    pinMode(DIO11,OUTPUT);

    // enable 22MA_A1
    digitalWrite(CS1_EN,HIGH);
    _delay_ms(100); // busy-wait delay
    
    // R1 hos 22MA_A1 on it
    float adc1_22mA_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("22MA_A1 source on R1: %1.3f A\r\n"), adc1_22mA_i);
    if (adc1_22mA_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_A1 curr is to low.\r\n"));
    }
    if (adc1_22mA_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_A1 curr is to high.\r\n"));
    }

    // enable 10mA into PL input for ICP1
    digitalWrite(CS1_EN,LOW);
    digitalWrite(CS_ICP1_10MA_EN,HIGH);
    _delay_ms(100); // busy-wait delay

    // ADC1 reads 100 Ohm on ICP1's PL input with 10mA
    float icp1_10mA_i = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1's PL input with 10mA: %1.3f A\r\n"), icp1_10mA_i);
    
    //This is a good place to swap ADC referances and find the band-gap voltage
    init_ADC_single_conversion(INTERNAL_1V1); 
    _delay_ms(100); // busy-wait delay
    int adc1_used_for_ref_intern_1v1_uV = analogRead(ADC1);
    printf_P(PSTR("   ADC1 reading used to calculate ref_intern_1v1_uV: %d A\r\n"), adc1_used_for_ref_intern_1v1_uV);
    float _ref_intern_1v1_uV = 1.0E6*1024.0 * ((icp1_10mA_i * ICP1_TERM) / adc1_used_for_ref_intern_1v1_uV);
    uint32_t temp_ref_intern_1v1_uV = (uint32_t)_ref_intern_1v1_uV;
    printf_P(PSTR("   calculated ref_intern_1v1_uV: %lu uV\r\n"), temp_ref_intern_1v1_uV);
    uint32_t temp_ref_extern_avcc_uV = ref_extern_avcc_uV;
    
    // check for old referance values
    if (LoadAnalogRefFromEEPROM())
    {
        printf_P(PSTR("REF_EXTERN_AVCC old value was in eeprom: %lu uV\r\n"), ref_extern_avcc_uV);
        printf_P(PSTR("REF_INTERN_1V1 old value was in eeprom: %lu uV\r\n"), ref_intern_1v1_uV);
    }
    ref_extern_avcc_uV = temp_ref_extern_avcc_uV;
    ref_intern_1v1_uV = temp_ref_intern_1v1_uV;
    if ((ref_intern_1v1_uV > 1050000UL)  || (ref_intern_1v1_uV < 1150000UL) )
    {
        while ( !WriteEeReferenceId() ) {};
        while ( !WriteEeReferenceAvcc() ) {};
        while ( !WriteEeReference1V1() ) {};
        printf_P(PSTR("REF_EXTERN_AVCC saved in eeprom: %lu uV\r\n"), ref_extern_avcc_uV);
        printf_P(PSTR("REF_INTERN_1V1 saved in eeprom: %lu uV\r\n"), ref_intern_1v1_uV);
    }
    else
    { 
        passing = 0; 
        printf_P(PSTR(">>> REF_* for ADC not saved in eeprom.\r\n"));
    }
    
    //swap back to the AVCC referance 
    init_ADC_single_conversion(EXTERNAL_AVCC); 
    _delay_ms(100); // busy-wait delay
    
    // ICP1 pin is inverted from to the plug interface, which should have 10 mA on its 100 Ohm Termination now
    printf_P(PSTR("ICP1 /w 10mA on plug termination reads: %d \r\n"), digitalRead(ICP1));
    if (digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be low with 10mA.\r\n"));
    }
    digitalWrite(CS_ICP1_10MA_EN,LOW);

    // enable the  22MA_DIO3, 22MA_DIO11, 17MA_ICP1 current sources
    digitalWrite(CS_EN,HIGH);
    _delay_ms(1000); // busy-wait delay
    float input_withcs_i = analogRead(PWR_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CS_EN==on: %1.3f A\r\n"), input_withcs_i);

    // DIO13 and DIO12 high-z to place 22MA_DIO11 on R1
    pinMode(DIO13,INPUT);
    pinMode(DIO12,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio11_22mA_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("22MA_DIO11 curr source on R1: %1.3f A\r\n"), dio11_22mA_i);
    if (dio11_22mA_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_DIO11 curr is to low.\r\n"));
    }
    if (dio11_22mA_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_DIO11 curr is to high.\r\n"));
    }

    // DIO12 shunting 
    pinMode(DIO13,INPUT);
    pinMode(DIO12,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio12_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO12 shunting 22MA_DIO11: %1.3f A\r\n"), dio12_shunt_i);
    if (dio12_shunt_i > 0.015) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO12 is not shunting.\r\n"));
    }

    // DIO13 shunting
    pinMode(DIO12,INPUT);
    pinMode(DIO13,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio13_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO13 shunting 22MA_DIO11: %1.3f A\r\n"), dio13_shunt_i);
    if (dio13_shunt_i > 0.015) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO13 is not shunting.\r\n"));
    }
    pinMode(DIO12,OUTPUT);

    // DIO3 and DIO10 high-z to place 22MA_DIO3 on R1
    pinMode(DIO3,INPUT);
    pinMode(DIO10,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio3_22mA_i  = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("22MA_DIO3 curr source on R1: %1.3f A\r\n"), dio3_22mA_i);
    if (dio3_22mA_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_DIO3 curr is to low.\r\n"));
    }
    if (dio3_22mA_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> 22MA_DIO3 curr is to high.\r\n"));
    }

    // DIO10 shunting 
    pinMode(DIO3,INPUT);
    pinMode(DIO10,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio10_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO10 shunting 22MA_DIO3: %1.3f A\r\n"), dio10_shunt_i);
    if (dio10_shunt_i > 0.015) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO10 is not shunting.\r\n"));
    }

    // DIO3 shunting
    pinMode(DIO10,INPUT);
    pinMode(DIO3,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio3_shunt_i = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO3 shunting 22MA_DIO3: %1.3f A\r\n"), dio3_shunt_i);
    if (dio3_shunt_i > 0.015) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO3 is not shunting.\r\n"));
    }
    pinMode(DIO10,OUTPUT);

    // DIO4 high-z to allow17mA curr source to ICP1's PL input.
    pinMode(DIO4,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float icp1_17mA_i = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1 17mA curr source on ICP1's PL plug: %1.3f A\r\n"), icp1_17mA_i);
    if (icp1_17mA_i < 0.012) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 17mA curr source is to low\r\n"));
    }
    if (icp1_17mA_i > 0.022) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 17mA curr source is to high.\r\n"));
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
    // Turn Off 22MA_DIO3, 22MA_DIO11, 17MA_ICP1 Curr Sources
    pinMode(CS_EN,OUTPUT);
    digitalWrite(CS_EN,LOW);
    
    // Turn Off 22MA_A0 Curr Source
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW);
    
    // Turn Off 22MA_A1 Curr Source
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);

    // Turn Off 10MA PL Curr Source
    pinMode(CS_ICP1_10MA_EN,OUTPUT);
    digitalWrite(CS_ICP1_10MA_EN,LOW);

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
        digitalWrite(CS_EN,HIGH);
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
        digitalWrite(CS0_EN,HIGH);
    }
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        if (passing)
        {
            digitalToggle(CS_EN);
        }
        else
        {
            digitalToggle(CS0_EN);
        }
        
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

