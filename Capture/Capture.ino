/*
    Copyright (C) 2014  Ronald Sutherland

    Capture is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Capture is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Capture.  If not, see <http://www.gnu.org/licenses/>.

    Capture 
    Arduino Uno (ATmega328) Input Capture Unit Example
    PWM from pin 11 may be used as a signal to capture with pin 8, plug a 10k Ohm resistor
    between them to be safe.
    updates may be found at http://epccs.org/hg/epccs/Software/file/tip/Embeded/Capture

    INPUT COMMAND STRUCTURE:
    position            usage 
    first alpha         command, a string of alpha and '?' only 
    [white space]       optional used to separate command from arguments
    [arguments]         one or more comma delimited e.g. "13,high"  
    \n                  end of cmd
            

    id?                 sends back identify[]
    count?              pulse or cycle count on Uno pin 8 (ICP1).
    duty? [l2h|h2l]     active (or inactive) pulse duty measured in clock counts on ICP1 
                        "l2h" is inactive time, falling edge to rising edge (low 2 high).
                        "h2l" is active time, rising edge to falling edge (high 2 low).
                        The duty cycle is typically a percentage of the period signal is active 
                        rather than clock counts.
    period? [rise|fall] pulse period measured in clock counts on ICP1 (Uno pin 8).
                        rising (falling) edge to rising (falling) edge period
                        period is the time it takes a signal to repeat.
    pwm [3 thru 252]    pulse width modulation pin 11 with the duty provided in argument.
                        Since timer1 is used for signal capture, timer2 is free to use
                        for pwm on pin 11. The duty has a minimum active (inactive) time
                        or the ISR may skip pulses (at about 300 counts).
*/


#include <avr/pgmspace.h>
// confusing, so prog_char typedef is deprecated, an attribute may be used on a variable declaration.
// http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html
// debug msg, that do not use SRAM
PROGMEM const char comment[]= {"#:\n\x06"};
PROGMEM const char identify[] = {"Capture Rev 0.0, Input Capture Unit Example\n"};
PROGMEM const char notImplemented[] = {": cmd not implemented "};
PROGMEM const char crcPass[] = {": CRC PASS\n"};
PROGMEM const char errNoCapture[] ={"ERR no-capture\n"};

// offests to parts found within a command, 
#define MAX_CMD_LENGTH 16
uint8_t cmdOffset = 0; // may be zero or non-zero, and must be found
uint8_t cmdOffsetEnd = 0;
uint8_t argOffset = 0; // may be found after command may be alpha-numeric and have ',' within
uint8_t argOffsetEnd = 0;
uint8_t delimter[3] = {0, 0, 0};
uint8_t crcMarkLoc = 0;

uint8_t ownID = 0;            // my ID e.g. my address
boolean debugmsg = false;      //retrun a debug answer

// Arduino has Tx and Rx buffers with ISRs that push/pull from its respective buffer.
// So all I need to do is check if a serial input is available, and extract it.
String command;
union twoByteCrc { uint16_t word; uint8_t byte[2]; } crc;
boolean commandComplete = false;
boolean crcComplete = false;
uint8_t recID;

void setup() {
  command.reserve(MAX_CMD_LENGTH);
  Serial.begin(9600);
  command = "";
  crc.word =0;
  cmdOffset =0;
  initCapture();
}

void loop () {
  if(Serial.available() > 0) {
    getIncomingChars();
  }
  if (commandComplete == true) {
    processCommand();
    command = "";
    commandComplete = false;
    cmdOffset =0;
    cmdOffsetEnd =0;
    argOffset =0;
    argOffsetEnd = 0;
    delimter[0] = 0;
    delimter[1] = 0;
    delimter[2] = 0;
    char c = pgm_read_byte(&(comment[3])); // + <ACK>
    Serial.write(c);
    c = pgm_read_byte(&(comment[2])); // + "\n"
    Serial.write(c);
  }
}


