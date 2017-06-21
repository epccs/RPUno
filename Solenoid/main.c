/*
Solenoid is a command line controled demonstration of ATmega328p ... solenoid control
Copyright (C) 2016 Ronald Sutherland

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
#include <util/atomic.h>
#include "../lib/uart.h"
#include "../lib/parse.h"
#include "../lib/timers.h"
#include "../lib/adc.h"
#include "../lib/twi.h"
#include "../lib/rpu_mgr.h"
#include "../lib/icp1.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Uart/id.h"
#include "../DayNight/day_night.h"
#include "../Adc/analog.h"
#include "../Capture/capture.h"
#include "solenoid.h"

#define ADC_DELAY_MILSEC 200UL
static unsigned long adc_started_at;

#define DAYNIGHT_STATUS_LED 4
#define DAYNIGHT_BLINK 500UL
static unsigned long day_status_blink_started_at;

#define BLINK_DELAY 1000UL
static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t solenoids_initalized;

void ProcessCmd()
{ 
    if (solenoids_initalized) 
    {
        if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
        {
            Id("Solenoid"); // ../Uart/id.c
        }
        if ( (strcmp_P( command, PSTR("/pre")) == 0) && ( (arg_count == 2 ) ) )
        {
            DelayStart(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/runtime")) == 0) && ( (arg_count == 2 ) ) )
        {
            RunTime(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/delay")) == 0) && ( (arg_count == 2 ) ) )
        {
            Delay(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/fstop")) == 0) && ( (arg_count == 2 ) ) )
        {
            FlowStop(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/run")) == 0) && ( (arg_count == 1) || (arg_count == 2) ) )
        {
            Run(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/save")) == 0) && ( (arg_count == 2 ) ) )
        {
            Save(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/load")) == 0) && ( (arg_count == 1 ) ) )
        {
            Load(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/time?")) == 0) && ( (arg_count == 1 ) ) )
        {
            Time(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/flow?")) == 0) && ( (arg_count == 1 ) ) )
        {
            Flow(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/stop")) == 0) && ( (arg_count == 1 ) ) )
        {
            Stop(); // solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/day?")) == 0) && ( (arg_count == 0 ) ) )
        {
            Day(); // ../DayNight/day_night.c
        }
        if ( (strcmp_P( command, PSTR("/analog?")) == 0) && ( (arg_count >= 1 ) && (arg_count <= 5) ) )
        {
            Analog(); // ../Adc/analog.c
        }
        if ( (strcmp_P( command, PSTR("/count?")) == 0) &&  ( (arg_count == 0) || ( (arg_count == 1) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            Count();
        }
        if ( (strcmp_P( command, PSTR("/capture?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            Capture();
        }
        if ( (strcmp_P( command, PSTR("/event?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            Event();
        }
        if ( (strcmp_P( command, PSTR("/initICP")) == 0) && ( ( (arg_count == 3) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            InitICP();
        }
    }
    else
    {
        if (! solenoids_initalized)
        {
            printf_P(PSTR("{\"err\":\"NotFinishKinit\"}\r\n"));
        }
        initCommandBuffer();
        return;
    }
}

//At start of each day load the solenoid control settings from EEPROM and operate them.
void callback_for_day_attach(void)
{
    for(uint8_t solenoid = 1; solenoid <= SOLENOID_COUNT; solenoid++)
    {
        LoadSolenoidControlFromEEPROM(solenoid);
        StartSolenoid(solenoid);
    }
}

void setup(void) 
{
	// RPUuno has no LED, but the LED_BUILTIN is defined as digital 13 (SCK) anyway.
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,HIGH);
    
    // A DayNight status LED is on digital pin 4
    pinMode(DAYNIGHT_STATUS_LED,OUTPUT);
    digitalWrite(DAYNIGHT_STATUS_LED,HIGH);
    
    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    // put ADC in Auto Trigger mode and fetch an array of channels
    enable_ADC_auto_conversion(BURST_MODE);
    adc_started_at = millis();

    /* Initialize Input Capture Unit (see ../lib/icp1.h) */
    initIcp1(TRACK_RISING, ICP1_MCUDIV64) ;

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Initialize I2C. note: I2C scan will stop without a pull-up on the bus */
    twi_init(TWI_PULLUP);

    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();

    // Enable global interrupts to start TIMER0 and UART ISR's
    sei(); 
    
    blink_started_at = millis();
    day_status_blink_started_at = millis();
    
    rpu_addr = get_Rpu_address();
    blink_delay = BLINK_DELAY;
    
    // blink fast if a default address from RPU manager not found
    if (rpu_addr == 0)
    {
        rpu_addr = '0';
        blink_delay = BLINK_DELAY/4;
    }
    
    // setup solenoid control
    init_K();
    
    // solenoids may have been previously latched so this
    // loads settings that will run a fast cycle
    Reset_All_K();
    solenoids_initalized = 0;
    
    // set callback. See Solenoid for another example, where it loads the EEPROM values used at the start of each day
    Day_AttachWork(callback_for_day_attach);
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        digitalToggle(LED_BUILTIN);
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

void blink_day_status(void)
{
    unsigned long kRuntime = millis() - day_status_blink_started_at;
    uint8_t state = DayState();
    if ( ( (state == DAYNIGHT_EVENING_DEBOUNCE_STATE) || \
           (state == DAYNIGHT_MORNING_DEBOUNCE_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK/2) ) )
    {
        digitalToggle(DAYNIGHT_STATUS_LED);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK/2; 
    }
    if ( ( (state == DAYNIGHT_DAY_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK) ) )
    {
        digitalWrite(DAYNIGHT_STATUS_LED,HIGH);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK; 
    }
    if ( ( (state == DAYNIGHT_NIGHT_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK) ) )
    {
        digitalWrite(DAYNIGHT_STATUS_LED,LOW);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK; 
    }
    if ( ( (state == DAYNIGHT_FAIL_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK/8) ) )
    {
        digitalToggle(DAYNIGHT_STATUS_LED);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK/8; 
    }
}

void adc_burst(void)
{
    unsigned long kRuntime= millis() - adc_started_at;
    if ((kRuntime) > ((unsigned long)ADC_DELAY_MILSEC))
    {
        enable_ADC_auto_conversion(BURST_MODE); // ../lib/adc.c
        adc_started_at += ADC_DELAY_MILSEC; 
    } 
}

int main(void) 
{
    setup();

    while(1) 
    { 
        // use LED_BUILTIN to show if I2C has a bus manager
        blink();
        
        // use DAYNIGHT_STATUS_LED to show day_state
        blink_day_status();

        // Check Day Light is a function that operates a day-night state machine.
        CheckDayLight(); // ../DayNight/day_night.c

        // delay between ADC burst
        adc_burst();

        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar());

            // address is an ascii value, warning: a null address would terminate the command string. 
            StartEchoWhenAddressed(rpu_addr);
        }
        
        // check if a character is available, and if so flush transmit buffer and nuke the command in process.
        // A multi-drop bus can have another device start transmitting after getting an address byte so
        // the first byte is used as a warning, it is the onlly chance to detect a possible collision.
        if ( command_done && uart0_available() )
        {
            // dump the transmit buffer to limit a collision 
            uart0_flush(); 
            initCommandBuffer();
        }
        
        // finish echo of the command line befor starting a reply (or the next part of a reply)
        if ( command_done && (uart0_availableForWrite() == UART_TX0_BUFFER_SIZE) )
        {
            if ( !echo_on  )
            { // this happons when the address did not match 
                initCommandBuffer();
            }
            else
            {
                if (command_done == 1)  
                {
                    findCommand();
                    command_done = 10;
                }
                
                // do not overfill the serial buffer since that blocks looping, e.g. process a command in 32 byte chunks
                if ( (command_done >= 10) && (command_done < 250) )
                {
                     ProcessCmd();
                }
                else 
                {
                    initCommandBuffer();
                }
            }
        }
        
        // Solenoid Control is a function that moves them through different states that are timed with millis() or icp1 flow count.
        SolenoidControl();
        
        if (!solenoids_initalized)
        {
            // lets test if they are in use.
            uint8_t solenoids_in_use = 0;
            for(uint8_t solenoid = 1; solenoid <= SOLENOID_COUNT; solenoid++)
            {
                if (Live(solenoid))
                {
                    solenoids_in_use =1;
                    break;
                }
            }
            if (! solenoids_in_use) 
            {
                solenoids_initalized = 1;
            }
        }
    }        
    return 0;
}
