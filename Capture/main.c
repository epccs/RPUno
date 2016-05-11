/*
Capture is a command line controled demonstration of Input Capture Unit
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

    Capture is an interactive command line program that demonstrates
    control of an ATmega328p (e.g. Arduino Uno) Input Capture Unit (ICP1)
    PWM from pin 11 may be used as a signal to capture with pin 8, plug 
    a 10k Ohm resistor between them to be safe.
    
    Updates may be found at http://epccs.org/hg/open/RPUno/file/tip/Capture
    Forum: http://forum.arduino.cc/index.php?topic=260172.0

    COMMAND LINE STRUCTURE: e.g. /0/id?
    position            usage 
    '/'                 first char will flush the transmit buffer and nuke any command in process
    '0'                 second char will address a (multi-drop) device and start echo
    command             is a string of isalpha() or '/' or '?' only 
    [space]             use an isspace() char between command and arguments
    [arg[,arg[...]]]    one or more comma delimited arguments e.g. "13,high"  
    [\r]\n              end of comand line
            

    id? [name|desc|avr-gcc]       sends back device info
    count?              sends back the ICP1 event count.
    duty?               duty returns Timer1 counts for high and low signal states
                        "low" count is Timer1 counts from a falling edge to a rising edge.
                        "high" count is Timer1 counts from a rising edge to a falling edge.
                        The duty and period can both be found from these timer counts.
    pwm [0 thru 255]    pulse width modulation of I/O pin 11 with the duty provided in argument.
                        Timer1 is used with ICP1. Timer2 is free to use for pwm on pin 11. 
                        pulses may be skiped if the on (or off) time is less than about 300 counts.


On Linux picocom can be used as a minimal serial terminal
https://github.com/npat-efault/picocom

picocom -b 115200 /dev/ttyUSB0
exit is C-a, C-x
*/
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include "process.h"
#include "../lib/uart.h"
#include "../lib/parse.h"
#include "../lib/timers.h"
#include "../lib/adc.h"
#include "../lib/icp1.h"

int main(void) 
{    
    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c 
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    initAdcSingleConversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup
    
    // setup()

    /* Initialize Input Capture Unit 1 */
    initIcp1() ;
    
    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();

    sei(); // Enable global interrupts starts TIMER0, UART0, ICP1 and other ISR's
    
    // loop() /* I am tyring to use non-blocking code */
    while(1) 
    {
        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar());

            // address is the ascii value for '0' note: a null address will terminate the command string. 
            StartEchoWhenAddressed('0');
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
    }        
    return 0;
}
