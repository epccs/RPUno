/* 
RPUno SelfTest
Copyright (C) 2016 Ronald Sutherland

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

#define BLINK_DELAY 1000UL

// actual value of the +5V converter 
#define ADC_REF 5.00

#define R1 47.0
#define ICP1_TERM 100.0

static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t passing;

void setup(void) 
{
    // Turn Off Curr Sources
    pinMode(CURR_SOUR_EN,OUTPUT);
    digitalWrite(CURR_SOUR_EN,LOW);

    // Turn Off VOUT to shield (e.g. disconnect VIN from shield)
    pinMode(SHLD_VOUT_EN,OUTPUT);
    digitalWrite(SHLD_VOUT_EN,LOW);

    // Battery disconnect (don't let the pin glitch with a high)
    digitalWrite(BAT_DISCONNECT,LOW);
    pinMode(BAT_DISCONNECT,OUTPUT);

    // Charge control (don't let the pin glitch with a high)
    digitalWrite(CC_SHUTDOWN,LOW);
    pinMode(CC_SHUTDOWN,OUTPUT);

    // To Check CC fault state drive pin with a weak pull-up
    digitalWrite(CC_nFAULT,HIGH);
    pinMode(CC_nFAULT,INPUT);

    // Set plugable DIO to shunt current sources, each has 182 Ohm. (e.g. turn LED's off) 
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

    // +5V is used as the ADC reference (perhaps this could be given over the UART)
    printf_P(PSTR("+5V needs measured and then set as ADC_REF: %1.3f V\r\n"), ADC_REF);

    // Current sources are off
    // Charge rate OK for 150mA input?
    float chrg_i = analogRead(CHRG_I)*(ADC_REF/1024.0)/(0.068*50.0);
    printf_P(PSTR("Charging with CURR_SOUR_EN==off: %1.3f A\r\n"), chrg_i);
    if (chrg_i > 0.15) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Charge rate is to high, is PV supply CC set at 150mA?\r\n"));
        return;
    }
    if (chrg_i < 0.09) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Charge rate is to low, is PV supply CC set at 150mA?\r\n"));
        return;
    }

    // Battery voltage
    float battery_v = analogRead(PWR_V)*(ADC_REF/1024.0)*(3.0/1.0);
    printf_P(PSTR("PWR (Battery) at: %1.3f V\r\n"), battery_v);
    if (battery_v > 14.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Battery is to high, verify with DMM, turn off the PV supply if correct.\r\n"));
        return;
    }
    if (battery_v < 12.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Battery is to low, verify with DMM, and charge it if correct.\r\n"));
        return;
    }
    
    // MPPT voltage should be on the the supply (assuming battery is not at float voltage)
    float mppt_v = analogRead(PV_V)*(ADC_REF/1024.0)*(532.0/100.0);
    printf_P(PSTR("MPPT at: %1.3f V\r\n"), mppt_v);
    if (mppt_v > 17.2) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> MPPT is to high, is battery at float voltage?\r\n"));
        return;
    }
    if (mppt_v < 16.4) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> MPPT is to low.\r\n"));
        return;
    }

    // ADC0 and ADC1 with current sources off
    float adc0_v = analogRead(ADC0)*(ADC_REF/1024.0);
    printf_P(PSTR("ADC0 at: %1.3f V\r\n"), adc0_v);
    float adc1_v = analogRead(ADC1)*(ADC_REF/1024.0);
    printf_P(PSTR("ADC1 at: %1.3f V\r\n"), adc1_v);
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

    // CC nFAULT with weak pull-up
    printf_P(PSTR("CC_nFAULT measured with a weak pull-up: %d \r\n"), digitalRead(CC_nFAULT));
    if (!digitalRead(CC_nFAULT)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CC_nFAULT should be high\r\n"));
    }
    
    // enable the current sources
    digitalWrite(CURR_SOUR_EN,HIGH);
    _delay_ms(50) ; // busy-wait delay
    float chrg_del_i = chrg_i - analogRead(CHRG_I)*(ADC_REF/1024.0)/(0.068*50.0);
    printf_P(PSTR("Charging delta with CURR_SOUR_EN==on: %1.3f A\r\n"), chrg_del_i);
    
    // ADC0 hos its own 20mA source now (DIO10 and DIO11 shunt the ADC1 source, and DIO12 and DIO13 shunt the digital source)
    float adc0_20mA_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 with its own 20mA source on R1: %1.3f A\r\n"), adc0_20mA_i);
    
    // ADC1 has ICP1's 10mA source now
    float adc1_icp1_10mA_i = analogRead(ADC1)*(ADC_REF/1024.0) / ICP1_TERM;
    printf_P(PSTR("ADC1 with ICP1's 10mA on ICP1_TERM: %1.3f A\r\n"), adc1_icp1_10mA_i);
    
    // ICP1 pin is inverted from to the plug interface, which should have 10 mA on its 100 Ohm Termination now
    printf_P(PSTR("ICP1 /w 10mA on plug termination reads: %d \r\n"), digitalRead(ICP1));
    if (digitalRead(ICP1)) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 should be low with 10mA.\r\n"));
    }
    
    // charge control shutdown
    digitalWrite(CC_SHUTDOWN,HIGH);
    _delay_ms(200) ; // busy-wait delay. It needs some extra time for my supply to switch from CC mode to CV mode.
    float dischrg_i = analogRead(DISCHRG_I)*(ADC_REF/1024.0)/(0.068*50.0);
    printf_P(PSTR("Dischrging at: %1.3f A\r\n"), dischrg_i);
    if (dischrg_i < 0.05) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Discharging is to low.\r\n"));
    }

    // PV open circuit voltage (LT3652 is off)
    float pvoc_v = analogRead(PV_V)*(ADC_REF/1024.0)*(532.0/100.0);
    printf_P(PSTR("PV open circuit (LT3652 off) at: %1.3f V\r\n"), pvoc_v);
    if (pvoc_v < 19.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> PV open circuit voltage is low.\r\n"));
    }
    
    // DIO13 and DIO12 high-z to add digital curr source to R1
    pinMode(DIO13,INPUT);
    pinMode(DIO12,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc0_40mA_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 and digital curr source on R1: %1.3f A\r\n"), adc0_40mA_i);
    if (adc0_40mA_i < 0.040) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Digital and ADC0 curr is to low\r\n"));
    }
    if (adc0_40mA_i > 0.047) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Digital and ADC0 curr is to high\r\n"));
    }

    // DIO13 high-z and DIO12 shunting most of the digital curr source from going through R1
    pinMode(DIO13,INPUT);
    pinMode(DIO12,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc0_dio12_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 measure curr on R1 with DIO12 shunting: %1.3f A\r\n"), adc0_dio12_i);
    if (adc0_dio12_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO12 is not shunting.\r\n"));
    }

    // DIO12 high-z and DIO13 shunting most of the digital curr source from going through R1
    pinMode(DIO12,INPUT);
    pinMode(DIO13,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc0_dio13_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 measure curr on R1 with DIO13 shunting: %1.3f A\r\n"), adc0_dio13_i);
    if (adc0_dio13_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO13 is not shunting.\r\n"));
    }
    pinMode(DIO12,OUTPUT);

    // DIO11 and DIO10 high-z to add ADC1 curr source to R1
    pinMode(DIO11,INPUT);
    pinMode(DIO10,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc0_adc1_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 and ADC1 curr source on R1: %1.3f A\r\n"), adc0_adc1_i);
    if (adc0_adc1_i < 0.040) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ADC1 and ADC0 curr is to low\r\n"));
    }
    if (adc0_adc1_i > 0.047) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ADC1 and ADC0 curr is to high.\r\n"));
    }

    // DIO11 high-z and DIO10 shunting most of the ADC1 curr source from going through R1
    pinMode(DIO11,INPUT);
    pinMode(DIO10,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc0_dio10_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 measure curr on R1 with DIO10 shunting: %1.3f A\r\n"), adc0_dio10_i);
    if (adc0_dio10_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO10 is not shunting.\r\n"));
    }

    // DIO10 high-z and DIO11 shunting most of the ADC1 curr source from going through R1
    pinMode(DIO10,INPUT);
    pinMode(DIO11,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc0_dio11_i = analogRead(ADC0)*(ADC_REF/1024.0) / R1;
    printf_P(PSTR("ADC0 measure curr on R1 with DIO11 shunting: %1.3f A\r\n"), adc0_dio11_i);
    if (adc0_dio11_i > 0.039) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO11 is not shunting.\r\n"));
    }

    // DIO3 and DIO4 high-z to add icp1's 10mA and 16mA curr source to ICP1_TERM
    pinMode(DIO3,INPUT);
    pinMode(DIO4,INPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc1_icp1all_i = analogRead(ADC1)*(ADC_REF/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1 10mA + 16mA curr source on ICP1_TERM: %1.3f A\r\n"), adc1_icp1all_i);
    if (adc1_icp1all_i < 0.025) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 10mA and 16mA curr source is to low\r\n"));
    }
    if (adc1_icp1all_i > 0.031) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> ICP1 10mA and 16mA curr source is to high.\r\n"));
    }

    // DIO3 high-z and DIO4 shunting most of the ICP1 curr source from going through ICP1_TERM
    pinMode(DIO3,INPUT);
    pinMode(DIO4,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc1_dio4_i = analogRead(ADC0)*(ADC_REF/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1 curr on ICP1_TERM with DIO4 shunting: %1.3f A\r\n"), adc1_dio4_i);
    if (adc1_dio4_i > 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO4 is not shunting.\r\n"));
    }

    // DIO10 high-z and DIO11 shunting most of the ADC1 curr source from going through R1
    pinMode(DIO4,INPUT);
    pinMode(DIO3,OUTPUT);
    _delay_ms(50) ; // busy-wait delay
    float adc1_dio3_i = analogRead(ADC0)*(ADC_REF/1024.0) / ICP1_TERM;
    printf_P(PSTR("ICP1 curr on ICP1_TERM with DIO3 shunting: %1.3f A\r\n"), adc1_dio3_i);
    if (adc1_dio3_i > 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> DIO3 is not shunting.\r\n"));
    }

    // Disconnect message
    printf_P(PSTR("To disconnect battery turn off the PV supply and LED should stop blinking\r\n"));

    // charge control startup
    digitalWrite(CC_SHUTDOWN,LOW);
    
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
    
    // ESD packaging note
    printf_P(PSTR("Only open the ESD shield bag in an ESD safe area.\r\n"));
    printf_P(PSTR("Remove the shipping box and this paper from the area \r\n"));
    printf_P(PSTR("before opening the ESD shield bag. \r\n"));
}


void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        if (passing)
        {
            pinMode(DIO3,INPUT);
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
            if (digitalRead(CURR_SOUR_EN))
            {
                if (digitalRead(DIO10))
                {
                    pinMode(DIO10,OUTPUT);
                    digitalWrite(DIO10,LOW);
                    pinMode(DIO11,OUTPUT);
                    digitalWrite(DIO11,LOW);
                    pinMode(DIO12,INPUT);
                    pinMode(DIO13,INPUT);
                }
                else
                {
                    pinMode(DIO10,INPUT);
                    pinMode(DIO11,INPUT);
                    pinMode(DIO12,OUTPUT);
                    digitalWrite(DIO12,LOW);
                    pinMode(DIO13,OUTPUT);
                    digitalWrite(DIO13,LOW);
                }
            }
         }
        digitalToggle(CURR_SOUR_EN);
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

int main(void)
{
    setup(); 
    
    passing = 1;
    test();
    
    while (1) 
    {
        blink();
        
        // PV input voltage
        float pv_v = analogRead(PV_V)*(ADC_REF/1024.0)*(532.0/100.0);
        if (pv_v < 5.0) 
        { 
            digitalWrite(BAT_DISCONNECT,HIGH);
            while (1) {}; // loop until the power is gone
        }
    }    
}

