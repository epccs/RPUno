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
#include "../Adc/analog.h"
#include "../i2c-debug/i2c-scan.h"
#include "../i2c-debug/i2c-cmd.h"
#include "../DayNight/day_night.h"
#include "../AmpHr/chrg_accum.h"
#include "../Alternat/alternat.h"
#include "../Capture/capture.h"
#include "../Solenoid/solenoid.h"
#include "../NightLight/nightlight.h"

#define ADC_DELAY_MILSEC 100UL
static unsigned long adc_started_at;

#define STATUS_LED DIO13

#define DAYNIGHT_STATUS_LED DIO17
#define DAYNIGHT_BLINK 500UL
static unsigned long day_status_blink_started_at;

#define BLINK_DELAY 1000UL
static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t solenoids_initalized;
static uint8_t leds_initalized;

void ProcessCmd()
{ 
    if (solenoids_initalized) 
    {
        if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
        {
            Id("KNL"); // ../Uart/id.c
        }
        if ( (strcmp_P( command, PSTR("/kpre")) == 0) && ( (arg_count == 2 ) ) )
        {
            KDelayStart(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/krunt")) == 0) && ( (arg_count == 2 ) ) )
        {
            KRunTime(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/kdelay")) == 0) && ( (arg_count == 2 ) ) )
        {
            KDelay(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/kfstop")) == 0) && ( (arg_count == 2 ) ) )
        {
            KFlowStop(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/krun")) == 0) && ( (arg_count == 1) || (arg_count == 2) ) )
        {
            KRun(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/ksave")) == 0) && ( (arg_count == 2 ) ) )
        {
            KSave(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/kload")) == 0) && ( (arg_count == 1 ) ) )
        {
            KLoad(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/ktime?")) == 0) && ( (arg_count == 1 ) ) )
        {
            KTime(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/kflow?")) == 0) && ( (arg_count == 1 ) ) )
        {
            KFlow(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/kstop")) == 0) && ( (arg_count == 1 ) ) )
        {
            KStop(); // ../Solenoid/solenoid.c
        }
        if ( (strcmp_P( command, PSTR("/preled")) == 0) && ( (arg_count == 2 ) ) )
        {
            NLDelayStart(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/runtimeled")) == 0) && ( (arg_count == 2 ) ) )
        {
            NLRunTime(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/delayled")) == 0) && ( (arg_count == 2 ) ) )
        {
            NLDelay(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/mahrled")) == 0) && ( (arg_count == 2 ) ) )
        {
            NLAHrStop(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/runled")) == 0) && ( (arg_count == 1) || (arg_count == 2) ) )
        {
            NLRun(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/saveled")) == 0) && ( (arg_count == 2 ) ) )
        {
            NLSave(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/loadled")) == 0) && ( (arg_count == 1 ) ) )
        {
            NLLoad(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/led?")) == 0) && ( (arg_count == 1 ) ) )
        {
            NLTime(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/stopled")) == 0) && ( (arg_count == 1 ) ) )
        {
            NLStop(); // ../NightLight/nightlight.c
        }
        if ( (strcmp_P( command, PSTR("/analog?")) == 0) && ( (arg_count >= 1 ) && (arg_count <= 5) ) )
        {
            Analog(20000UL); // ../Adc/analog.c: show every 20 sec until terminated
        }
        if ( (strcmp_P( command, PSTR("/iscan?")) == 0) && (arg_count == 0) )
        {
            I2c_scan(); // ../i2c-debug/i2c-scan.c
        }
        if ( (strcmp_P( command, PSTR("/iaddr")) == 0) && (arg_count == 1) )
        {
            I2c_address(); // ../i2c-debug/i2c-cmd.c
        }
        if ( (strcmp_P( command, PSTR("/ibuff")) == 0) )
        {
            I2c_txBuffer(); // ../i2c-debug/i2c-cmd.c
        }
        if ( (strcmp_P( command, PSTR("/ibuff?")) == 0) && (arg_count == 0) )
        {
            I2c_txBuffer(); // ../i2c-debug/i2c-cmd.c
        }
        if ( (strcmp_P( command, PSTR("/iwrite")) == 0) && (arg_count == 0) )
        {
            I2c_write(); // ../i2c-debug/i2c-cmd.c
        }
        if ( (strcmp_P( command, PSTR("/iread?")) == 0) && (arg_count == 1) )
        {
            I2c_read(); // ../i2c-debug/i2c-cmd.c
        }
        if ( (strcmp_P( command, PSTR("/day?")) == 0) && ( (arg_count == 0 ) ) )
        {
            Day(60000UL); // ../DayNight/day_night.c: show every 60 sec until terminated
        }
        if ( (strcmp_P( command, PSTR("/charge?")) == 0) && ( (arg_count == 0 ) ) )
        {
            Charge(60000UL); // ../AmpHr/chrg_accum.c: show every 60 sec until terminated
        }
        if ( (strcmp_P( command, PSTR("/alt")) == 0) && ( (arg_count == 0 ) ) )
        {
            EnableAlt(); // ../Alternat/alternat.c
        }
        if ( (strcmp_P( command, PSTR("/altcnt?")) == 0) && ( (arg_count == 0 ) ) )
        {
            AltCount(); // ../Alternat/alternat.c
        }
        /*
        if ( (strcmp_P( command, PSTR("/count?")) == 0) &&  ( (arg_count == 0) || ( (arg_count == 1) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            Count(); // ../Capture/capture.c
        }
        if ( (strcmp_P( command, PSTR("/capture?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            Capture(); // ../Capture/capture.c
        }
        if ( (strcmp_P( command, PSTR("/event?")) == 0) && ( (arg_count == 0 ) || ( (arg_count == 2) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            Event(); // ../Capture/capture.c
        }
        if ( (strcmp_P( command, PSTR("/initICP")) == 0) && ( ( (arg_count == 3) && (strcmp_P( arg[0], PSTR("icp1")) == 0) ) ) )
        {
            InitICP(); // ../Capture/capture.c
        }
        */
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

//At start of each night 
void callback_for_night_attach(void)
{
    //turn off alternat power input
    alt_enable = 0;
    
    // turn off the Solenoid's
    //for (uint8_t solenoid = 1; solenoid <= SOLENOID_COUNT; solenoid++)
    //{
    //    StopK(solenoid);
    //}
    
    // If the battery got charged.
    if (alt_count > 5) 
    {
        for(uint8_t led = 1; led <= LEDSTRING_COUNT; led++)
        {
            // load the LED control settings from EEPROM
            LoadLedControlFromEEPROM(led); // nightlight.c
            // operate LED
            StartLed(led); // nightlight.c
        }
    }
}

//At start of each day.
void callback_for_day_attach(void)
{
    alt_enable = 1; //turn on alternat power input
    alt_count = 0; // this value is used to tell if the battery got a full charge

    // setup AmpHr accumulators and load Adc calibration reference
    if (!init_ChargAccumulation(PWR_I)) // ../AmpHr/chrg_accum.c
    {
        blink_delay = BLINK_DELAY/4;
    }

    // turn off the LED's
    for (uint8_t led = 1; led <= LEDSTRING_COUNT; led++)
    {
        StopLED(led);
    }

    // water the garden
    for(uint8_t solenoid = 1; solenoid <= SOLENOID_COUNT; solenoid++)
    {
        // load the solenoid control settings from EEPROM
        LoadKControlFromEEPROM(solenoid);
        // operate them
        StartK(solenoid);
    }
}

void setup(void) 
{
    pinMode(STATUS_LED,OUTPUT);
    digitalWrite(STATUS_LED,LOW);

    pinMode(DAYNIGHT_STATUS_LED,OUTPUT);
    digitalWrite(DAYNIGHT_STATUS_LED,HIGH);

    pinMode(ALT_EN,OUTPUT);
    digitalWrite(ALT_EN,LOW);

    pinMode(CS_ICP1_EN,OUTPUT);
    digitalWrite(CS_ICP1_EN,HIGH);

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
    Reset_All_K();
    solenoids_initalized = 0;

    // setup Led string control
    init_Led();
    Reset_All_LED();
    leds_initalized = 0;

    // register callbacks for DayNight events
    Night_AttachWork(callback_for_night_attach);
    Day_AttachWork(callback_for_day_attach);

    // setup AmpHr accumulators
    if (!init_ChargAccumulation(PWR_I)) // ../AmpHr/chrg_accum.c
    {
        blink_delay = BLINK_DELAY/4;
    }

    // default debounce is 15 min (e.g. 900,000 millis)
    // evening_debouce = 18000UL; // 18 sec
    // morning_debouce = 18000UL;

    alt_count = 0;
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        digitalToggle(STATUS_LED);
        
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

uint8_t adc_burst(void)
{
    unsigned long kRuntime= millis() - adc_started_at;
    if ((kRuntime) > ((unsigned long)ADC_DELAY_MILSEC))
    {
        enable_ADC_auto_conversion(BURST_MODE);
        adc_started_at += ADC_DELAY_MILSEC; 
        return 1;
    } 
    else
    {
        return 0;
    }
}

int main(void) 
{
    setup();

    while(1) 
    { 
        // use STATUS_LED to show if I2C has a bus manager
        blink();
        
        // use DAYNIGHT_STATUS_LED to show day_state
        blink_day_status();

        // Check Day Light is a function that operates a day-night state machine.
        CheckDayLight(ADC2); // ../DayNight/day_night.c

        // delay between ADC burst
        if ( adc_burst() )
        {
            check_if_alt_should_be_on(PWR_V, 115.8/15.8, 13.6);
        }

        // check how much charge went into battery
        CheckChrgAccumulation(PWR_I);

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
        KControl();
        
        if (!solenoids_initalized)
        {
            // lets test if they are in use.
            uint8_t solenoids_in_use = 0;
            for(uint8_t solenoid = 1; solenoid <= SOLENOID_COUNT; solenoid++)
            {
                if (KLive(solenoid))
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
        
        // Led Control is a non-blocking function that moves the LED's through different states that are timed with millis().
        LedControl();
        
        if (!leds_initalized)
        {
            // lets test if they are in use.
            uint8_t leds_in_use = 0;
            for(uint8_t led = 1; led <= LEDSTRING_COUNT; led++)
            {
                if (NLLive(led))
                {
                    leds_in_use =1;
                    break;
                }
            }
            if (! leds_in_use) 
            {
                leds_initalized = 1;
            }
        }
    }        
    return 0;
}
