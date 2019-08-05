/* 
RPUno SelfTest
Copyright (C) 2019 Ronald Sutherland

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

For a copy of the GNU General Public License use
http://www.gnu.org/licenses/gpl-2.0.html
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
#define REF_EXTERN_AVCC 4996300UL


#define R1 33.333
#define ICP1_TERM 100.0

static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t passing;

void setup(void) 
{
    // Turn Off Current Sources
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW);
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);
    pinMode(CS2_EN,OUTPUT);
    digitalWrite(CS2_EN,LOW);
    pinMode(CS3_EN,OUTPUT);
    digitalWrite(CS3_EN,LOW);
    pinMode(CS_ICP1_EN,OUTPUT);
    digitalWrite(CS_ICP1_EN,LOW);
    
    // Turn Off VOUT to shield (e.g. disconnect VIN from shield)
    pinMode(SHLD_VIN_EN,OUTPUT);
    digitalWrite(SHLD_VIN_EN,LOW);

    // ADC2 and ADC3 have LED that may show a voltage if light is on them
    // sink ADC3 with DIO10
    pinMode(DIO10,OUTPUT);
    digitalWrite(DIO10,LOW);
    // sink ADC2 with DIO13
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
    // Info from some Predefined Macros
    printf_P(PSTR("RPUno Self Test date: %s\r\n"), __DATE__);
    printf_P(PSTR("avr-gcc --version: %s\r\n"),__VERSION__);
    
    // I2C is used to read serial bus manager address 
    if (rpu_addr == '1')
    {
        printf_P(PSTR("I2C provided address 0x31 from RPUadpt serial bus manager\r\n"));
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C failed, or address not 0x31 from serial bus manager\r\n"));
        return;
    }

    // With current sources off measure input current
    _delay_ms(1000) ; // busy-wait to let the 1uF settle
    
    // Input voltage
    int adc_pwr_v = analogRead(PWR_V);
    printf_P(PSTR("adc reading for PWR_V: %d\r\n"), adc_pwr_v);
    float input_v = adc_pwr_v*((ref_extern_avcc_uV/1.0E6)/1024.0)*(115.8/15.8);
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

    //current sources are off,  ADC2 is pull low with DIO10, and ADC3 is low with  DIO13 

    // measure ADC0..ADC3 
    float adc0_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC0 R1 /W all CS off: %1.3f V\r\n"), adc0_v);
    float adc1_v = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC1 at ICP1 TERM /w all CS off: %1.3f V\r\n"), adc1_v);
    float adc2_v = analogRead(ADC2)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC2 at GN LED /w DIO13 sinking and all CS off: %1.3f V\r\n"), adc2_v);
    float adc3_v = analogRead(ADC3)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC3 at YE LED /w DIO10 sinking and all CS off: %1.3f V\r\n"), adc3_v);
    if ( (adc0_v > 0.01)  || (adc1_v > 0.01) || (adc2_v > 0.01) || (adc3_v > 0.01))
    { 
        passing = 0; 
        printf_P(PSTR(">>> ADC is to high, is the self-test wiring right?\r\n"));
        return;
    }

    // ICP1 pin is inverted from the plug interface, which ADC2 showed to have zero mA on its 100 Ohm Termination
    printf_P(PSTR("ICP1 input should be HIGH with 0mA loop current: %d \r\n"), digitalRead(ICP1));
    if (!digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be high.\r\n"));
    }
    
    // enable CS0 which DIO11 can shunt
    digitalWrite(CS0_EN,HIGH);
    pinMode(DIO11,INPUT);
    _delay_ms(100); // busy-wait delay

    // R1 should have CS0 in it, and D3 red LED should be on
    float adc0_cs0_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc0_cs0_i = adc0_cs0_v / R1;
    printf_P(PSTR("CS0 on R1: %1.3f A\r\n"), adc0_cs0_i);
    if (adc0_cs0_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS0 curr is to low.\r\n"));
        return;
    }
    if (adc0_cs0_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS0 curr is to high.\r\n"));
        return;
    }

    // DIO11 can shunt the current from CS0
    pinMode(DIO11,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio11_shunt_i = adc0_cs0_i - analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO11 shunting CS0: %1.3f A\r\n"), dio11_shunt_i);
    if (dio11_shunt_i < 0.010) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO11 is not shunting enough curr.\r\n"));
    }
    if (dio11_shunt_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO11 shunting error.\r\n"));
    }
    digitalWrite(CS0_EN,LOW);

    // enable CS1
    digitalWrite(CS1_EN,HIGH);
    _delay_ms(100); // busy-wait delay
    
    // CS1 drives R1 direct (e.g. goes through no LED)
    float adc0_cs1_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc0_cs1_i = adc0_cs1_v / R1;
    printf_P(PSTR("CS1 source on R1: %1.3f A\r\n"), adc0_cs1_i);
    if (adc0_cs1_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS1 curr is to low.\r\n"));
    }
    if (adc0_cs1_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS1 curr is to high.\r\n"));
    }

    //This is a good place to swap ADC referances and find the band-gap voltage
    init_ADC_single_conversion(INTERNAL_1V1); 
    _delay_ms(100); // busy-wait delay
    int adc0_used_for_ref_intern_1v1_uV = analogRead(ADC0);
    printf_P(PSTR("   ADC0 reading used to calculate ref_intern_1v1_uV: %d A\r\n"), adc0_used_for_ref_intern_1v1_uV);
    float _ref_intern_1v1_uV = 1.0E6*1024.0 * ((adc0_cs1_i * R1) / adc0_used_for_ref_intern_1v1_uV);
    uint32_t temp_ref_intern_1v1_uV = (uint32_t)_ref_intern_1v1_uV;
    printf_P(PSTR("   calculated ref_intern_1v1_uV: %lu uV\r\n"), temp_ref_intern_1v1_uV);
    uint32_t temp_ref_extern_avcc_uV = ref_extern_avcc_uV;
    
    // check for old referance values
    ref_extern_avcc_uV = 0;
    ref_intern_1v1_uV = 0;
    LoadAnalogRefFromEEPROM();
    printf_P(PSTR("REF_EXTERN_AVCC old value found in eeprom: %lu uV\r\n"), ref_extern_avcc_uV);
    printf_P(PSTR("REF_INTERN_1V1 old value found in eeprom: %lu uV\r\n"), ref_intern_1v1_uV);
    ref_intern_1v1_uV = temp_ref_intern_1v1_uV;
    if (ref_extern_avcc_uV == temp_ref_extern_avcc_uV)
    {
        printf_P(PSTR("REF_EXTERN_AVCC from eeprom is same\r\n"));
    }
    else
    {
        ref_extern_avcc_uV = temp_ref_extern_avcc_uV;
        if ((ref_intern_1v1_uV > 1050000UL)  || (ref_intern_1v1_uV < 1150000UL) )
        {
            while ( !WriteEeReferenceId() ) {};
            while ( !WriteEeReferenceAvcc() ) {};
            while ( !WriteEeReference1V1() ) {};
            printf_P(PSTR("REF_EXTERN_AVCC saved into eeprom: %lu uV\r\n"), ref_extern_avcc_uV);
            printf_P(PSTR("REF_INTERN_1V1 saved into eeprom: %lu uV\r\n"), ref_intern_1v1_uV);
        }
        else
        { 
            passing = 0; 
            printf_P(PSTR(">>> REF_* for ADC not saved in eeprom.\r\n"));
        }
    }
    digitalWrite(CS1_EN,LOW);
    _delay_ms(1000); 

    // input current at no load 
    float input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I at no load use INTERNAL_1V1: %1.3f A\r\n"), input_i);
    if (input_i > 0.025) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to high.\r\n"));
        return;
    }
    if (input_i < 0.005) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to low.\r\n"));
        return;
    }

    //swap back to the AVCC referance 
    init_ADC_single_conversion(EXTERNAL_AVCC); 
    _delay_ms(100); // busy-wait delay

    // enable CS2, which DIO10 can shunt
    digitalWrite(CS2_EN,HIGH);
    pinMode(DIO10,INPUT);
    _delay_ms(100); // busy-wait delay
    
    // R1 hos CS2 on it
    float adc0_cs2_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc0_cs2_i = adc0_cs2_v / R1;
    printf_P(PSTR("CS2 source on R1: %1.3f A\r\n"), adc0_cs2_i);
    if (adc0_cs2_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS2 curr is to low.\r\n"));
    }
    if (adc0_cs2_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS2 curr is to high.\r\n"));
    }

    // CS2 drives R1 through  a yellow LED which ADC3 can read
    float adc3_cs2_v = analogRead(ADC3)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float d4ye_led_v = adc3_cs2_v - adc0_cs2_v;
    printf_P(PSTR("Yellow LED D4 fwd /w CS2 V: %1.3f V\r\n"), d4ye_led_v);

    // DIO10 can shunt the current from CS2
    pinMode(DIO10,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio10_shunt_i = adc0_cs2_i - analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO10 shunting CS2: %1.3f A\r\n"), dio10_shunt_i);
    if (dio10_shunt_i < 0.010) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO10 is not shunting enough curr.\r\n"));
    }
    if (dio10_shunt_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO10 shunting error.\r\n"));
    }

    digitalWrite(CS2_EN,LOW);

    // enable CS3, which DIO12 can shunt
    digitalWrite(CS3_EN,HIGH);
    pinMode(DIO12,INPUT);
    _delay_ms(100); // busy-wait delay
    
    // R1 hos CS3 on it
    float adc0_cs3_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc0_cs3_i = adc0_cs3_v / R1;
    printf_P(PSTR("CS3 source on R1: %1.3f A\r\n"), adc0_cs3_i);
    if (adc0_cs3_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS3 curr is to low.\r\n"));
    }
    if (adc0_cs3_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS3 curr is to high.\r\n"));
    }

    // DIO12 can shunt the current from CS3
    pinMode(DIO12,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio12_shunt_i = adc0_cs3_i - analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0) / R1;
    printf_P(PSTR("DIO12 shunting CS3: %1.3f A\r\n"), dio12_shunt_i);
    if (dio12_shunt_i < 0.010) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO12 is not shunting enough curr.\r\n"));
    }
    if (dio12_shunt_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO12 shunting error.\r\n"));
    }
    digitalWrite(CS3_EN,LOW);

    // enable CS_ICP1, which a green LED that ADC2 can measure and DIO13 can shunt
    digitalWrite(CS_ICP1_EN,HIGH);
    pinMode(DIO13,INPUT);
    _delay_ms(100); // busy-wait delay

    // ICP1_TERM has CS_ICP1 on it
    float adc1_cs_icp1_v = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc1_cs_icp1_i = adc1_cs_icp1_v / ICP1_TERM;
    printf_P(PSTR("CS_ICP1 in UUT PL input: %1.3f A\r\n"), adc1_cs_icp1_i);
    if (adc1_cs_icp1_i < 0.012) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS_ICP1 curr is to low.\r\n"));
    }
    if (adc1_cs_icp1_i > 0.020) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS_ICP1 curr is to high.\r\n"));
    }

    // CS_ICP1 drives ICP1 through  a green LED which ADC2 can read
    float adc2_cs_icp1_v = analogRead(ADC2)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float d1gn_led_v = adc2_cs_icp1_v - adc1_cs_icp1_v;
    printf_P(PSTR("Green LED D1 fwd /w CS_ICP1 V: %1.3f V\r\n"), d1gn_led_v);

    // ICP1 pin is inverted from to the plug interface, which should have 17 mA on its 100 Ohm Termination now
    printf_P(PSTR("ICP1 /w 17mA on termination reads: %d \r\n"), digitalRead(ICP1));
    if (digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be low with 17mA.\r\n"));
    }

    // DIO13 can shunt the current from CS_ICP1
    pinMode(DIO13,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float dio13_shunt_i = adc1_cs_icp1_i - analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0) / ICP1_TERM;
    printf_P(PSTR("DIO13 shunting CS_ICP1: %1.3f A\r\n"), dio13_shunt_i);
    if (dio13_shunt_i < 0.007) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO13 is not shunting enough curr.\r\n"));
    }
    if (dio13_shunt_i > 0.020) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO13 shunting error.\r\n"));
    }
    pinMode(DIO13,INPUT);
    digitalWrite(CS_ICP1_EN,LOW);

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
    digitalWrite(CS0_EN,LOW);
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);
    pinMode(CS2_EN,OUTPUT);
    digitalWrite(CS2_EN,LOW);
    pinMode(CS3_EN,OUTPUT);
    digitalWrite(CS3_EN,LOW);
    pinMode(CS_ICP1_EN,OUTPUT);
    digitalWrite(CS_ICP1_EN,LOW);

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
        digitalWrite(CS_ICP1_EN,HIGH);
        pinMode(DIO13,INPUT);
    }
    else
    {
        digitalWrite(CS0_EN,HIGH);
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
            digitalToggle(CS_ICP1_EN);
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

