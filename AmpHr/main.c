/*
AmpHr is a command line controled demonstration of how to estimate the charge used from a power source.
Copyright (C) 2017 Ronald Sutherland

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
#include "../lib/timers.h"
#include "../lib/uart.h"
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/twi.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Uart/id.h"
#include "../Adc/analog.h"
#include "../i2c-debug/i2c-scan.h"
#include "../i2c-debug/i2c-cmd.h"
#include "../Digital/digital.h"
#include "../CurrSour/cs.h"
#include "../CurrSour/csp.h"
#include "../CurrSour/status.h"
#include "chrg_accum.h"

// how fast does the discharge reading change? (it can be fast)
// at 16MHz the adc clock runs at 125kHz (see the core file ../lib/adc.c)
// the first reading takes 25 ADC clocks and the next takes 13 ADC clocks.
// since the channel changes with each new reading it always takes 25 ADC clocks for each reading.
// which means the max reading rate is about 5000 per sec.
// Fill the adc array every 10mSec with readings on all channels (e.g. 800 reading per sec, ) 
// The ADC is off 84% of the time so most of the power saving is relized.
#define ADC_DELAY_MILSEC 10UL
static unsigned long adc_started_at;

// 22mA current sources enabled with CS0_EN and CS1_EN which are defined in ../lib/pins_board.h
#define STATUS_LED CS0_EN

#define BLINK_DELAY 1000UL
static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("AmpHr"); // ../Uart/id.c
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
    if ( (strcmp_P( command, PSTR("/pMod")) == 0) && ( (arg_count == 2 ) ) )
    {
        Mode(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/dWrt")) == 0) && ( (arg_count == 2 ) ) )
    {
        Write(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/dTog")) == 0) && ( (arg_count == 1 ) ) )
    {
        Toggle(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/dRe?")) == 0) && ( (arg_count == 1 ) ) )
    {
        Read(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/cs")) == 0) && ( (arg_count == 2 ) ) )
    {
        CurrSour(); // ../CurrSour/cs.c
    }
    if ( (strcmp_P( command, PSTR("/csp")) == 0) && ( (arg_count == 2 ) ) )
    {
        CurrSourICP(); // ../CurrSour/csp.c
    }
    if ( (strcmp_P( command, PSTR("/showstat")) == 0) && ( (arg_count == 1 ) ) )
    {
        ShowStatus(); // ../CurrSour/status.c
    }
    if ( (strcmp_P( command, PSTR("/charge?")) == 0) && ( (arg_count == 0 ) ) )
    {
        Charge(60000UL); // ./chrg_accum.c: show every 60 sec until terminated
    }
    if ( (strcmp_P( command, PSTR("/reset")) == 0) && ( (arg_count == 0 ) ) )
    {
        if (!ResetChargeAccum()) // ./chrg_accum.c
        {
            blink_delay = BLINK_DELAY/4;
        }
    }
}

void setup(void) 
{
    pinMode(STATUS_LED,OUTPUT);
    digitalWrite(STATUS_LED,LOW);

    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    // put ADC in Auto Trigger mode and fetch an array of channels
    enable_ADC_auto_conversion(BURST_MODE);
    adc_started_at = millis();

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Initialize I2C, with the internal pull-up 
        note: I2C scan will stop without a pull-up on the bus */
    twi_init(TWI_PULLUP);

    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();

    // Enable global interrupts to start TIMER0 and UART ISR's
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

    // do not use current source to show status unless told with the CLI
    show_status = 0;
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        if (show_status) digitalToggle(STATUS_LED);
        
        // next toggle 
        blink_started_at += blink_delay; 
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
        // use STATUS_LED to show if I2C has a bus manager
        blink();

        // delay between ADC burst
        adc_burst();

        // check how much current went through high side sensor
        CheckChrgAccumulation(PWR_I);
    
        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from ../lib/parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar()); // ../lib/parse.c

            // address is an ascii value, warning: a null address would terminate the command string. 
            StartEchoWhenAddressed(rpu_addr); // ../lib/parse.c
        }
        
        // check if a character is available, and if so flush transmit buffer and nuke the command in process.
        // A multi-drop bus can have another device start transmitting after getting an address byte so
        // the first byte is used as a warning, it is the onlly chance to detect a possible collision.
        if ( command_done && uart0_available() )
        {
            // dump the transmit buffer to limit a collision 
            uart0_flush(); // ../lib/uart.c
            initCommandBuffer(); // ../lib/parse.c
        }

        // finish echo of the command line befor starting a reply (or the next part of a reply)
        if ( command_done && (uart0_availableForWrite() == UART_TX0_BUFFER_SIZE) )
        {
            if ( !echo_on  )
            { // this happons when the address did not match 
                initCommandBuffer(); // ../lib/parse.c
            }
            else
            {
                if (command_done == 1)  
                {
                    findCommand(); // ../lib/parse.c
                    // steps 2..9 are skipped. Reserved for more complex parse
                    command_done = 10;
                }
                
                // do not overfill the serial buffer since that blocks looping, e.g. process a command in 32 byte chunks
                if ( (command_done >= 10) && (command_done < 250) )
                {
                    // setps 10..249 are moved through by the procedure selected
                     ProcessCmd();
                }
                else 
                {
                    initCommandBuffer(); // ../lib/parse.c
                }
            }
         }
    }        
    return 0;
}
