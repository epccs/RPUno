/*
    This file (InputCapture.ino) is part of Capture.
    
    Timer 1 Input Capture, Arduino Uno pin 8 ICP1 facility, 
    referance
    https://gist.github.com/mpflaga/4404996
    http://forum.arduino.cc/index.php/topic,146497.0.html
    note: volatile will force compiler to load/store variable every time it is used.
    
    This capture method will skip pulses if the duty is about 300 counts (or less), 
    about 19%@10kHz or 1.9%@1kHz (assuming a 16MHz MCU).
    The counter is 32 bits 
    
    command format for rising (falling) edge to rising (falling) edge period count
    period? [rise|fall]

    pulse duty measured in clock counts. l2h (low 2 high), h2l (high 2 low).
    duty? [h2l|l2h]

    command format for channel pulse count of current channel, read befor changing channel
    count?

*/
#include "Arduino.h"
#include <avr/interrupt.h>
 
volatile uint8_t rising; //true is rising edge that will cause capture event, false is falling edge
 
// ring or circular buffer for capture events http://en.wikipedia.org/wiki/Circular_buffer
#define MAX_EVENT_BUFF 4
volatile uint8_t ring; // head of ring buffer, don't need a tail, use active

typedef struct {
      uint32_t event;
      uint32_t duty;
      uint32_t period;
      uint8_t rising;
      uint8_t active;
  }  capture;

volatile capture icp[MAX_EVENT_BUFF];

uint32_t chCount; // ch pulse count
//int32_t sumt, bogon_count;
volatile union twoword { uint32_t dword; uint16_t word[2]; } t1vc;   // timer 1 virtual counter

// Interrupt capture handler
//
ISR(TIMER1_CAPT_vect) {
  union twobyte { uint16_t word; uint8_t byte[2]; } timevalue;
  
  timevalue.byte[0] = ICR1L;	// grab captured timer1 low byte
  timevalue.byte[1] = ICR1H;	// grab captured timer1 high byte
  t1vc.word[0] = timevalue.word;

  // put next timestamp onto the ring buffer
  ring++;
  if (ring >= MAX_EVENT_BUFF) ring = 0;
  icp[ring].event = t1vc.dword;
  icp[ring].rising = rising;
  icp[ring].active = true;
  if (rising) {
    chCount++;
  }
  if (chCount > MAX_EVENT_BUFF) { // buff is full of new readings
    if (ring == 0) { // cast to int uses two's complement math giving correct result at roll over 
      icp[ring].duty = (int32_t)icp[ring].event - (int32_t)icp[MAX_EVENT_BUFF-1].event;
      icp[ring].period = (int32_t)icp[ring].event - (int32_t)icp[MAX_EVENT_BUFF-2].event;
    }  
    if (ring == 1) {
      icp[1].duty = (int32_t)icp[ring].event - (int32_t)icp[ring - 1].event;
      icp[1].period = (int32_t)icp[ring].event - (int32_t)icp[MAX_EVENT_BUFF-1].event;
    }
    if (ring > 1) {
      icp[ring].duty = (int32_t)icp[ring].event - (int32_t)icp[ring - 1].event;
      icp[ring].period = (int32_t)icp[ring].event - (int32_t)icp[ring - 2].event;
    }
  } else {
    icp[ring].duty = 0;
    icp[ring].period = 0;
  }
  // rising or falling edge tracking
  // 328 datasheet said to clear ICF1 after edge direction change 
  // switch edge setup to catch duty
  // setup to catch rising edge: TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1;
  // setup to catch falling edge: TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0;
  if (rising) { TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; }
  else {TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1;}
}

// Virtual timer counts 2^32 clocks (about 4.3E9, thus events must be less than 4.4 minutes apart) 
// Maintain the high order 16 bits here by incrementing the virtual timer
// when timer1 overflows. The low order 16 bits are captured above.
//
ISR(TIMER1_OVF_vect) {
  ++t1vc.word[1];
}

void initCapture(void) {
  // Input Capture setup
  // ICNC1: Enable Input Capture Noise Canceler
  // ICES1: =1 for trigger on rising edge
  // CS10: =1 set prescaler to 1x system clock (F_CPU)
  TCCR1A = 0;
  TCCR1B = (0<<ICNC1) | (0<<ICES1) | (1<<CS10);
  TCCR1C = 0;
   
  // initialize to catch Falling Edge
  { TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0; }
   
  // Interrupt setup
  // ICIE1: Input capture
  // TOIE1: Timer1 overflow
  TIFR1 = (1<<ICF1) | (1<<TOV1);	// clear pending interrupts
  TIMSK1 = (1<<ICIE1) | (1<<TOIE1);	// enable interupts

  // Set up the Input Capture pin, ICP1, Arduino Uno pin 8
  pinMode(8, INPUT);
  digitalWrite(8, 0);	// floating may have 60 Hz noise on it.
  //digitalWrite(8, 1); // or enable the pullup
}

//period is the time it takes a signal to repeat
void Period() {
  if((argumentCount(findArgument()) == 1)) {
    String arg1;
    arg1 = command.substring(argOffset,argOffsetEnd);
    uint32_t val=0;
    boolean found =false;
    if (chCount > MAX_EVENT_BUFF) { // buff is full of new readings
      uint8_t oldsreg = SREG;     // save global interrupt flag
      cli();                      // clear global interrupts
      if(arg1.equalsIgnoreCase("rise")){
        uint8_t i = 0;
        if (!icp[i].rising) {
          i++;
        }  
        val = icp[i].period;
        found =true; 
      }
      if(arg1.equalsIgnoreCase("fall")) {
        uint8_t i = 0;
        if (icp[i].rising) {
          i++;
        }
        val = icp[i].period;
        found =true; 
      }
      if (found) chCount = 0;
      SREG = oldsreg;             // restore global interrupts
    } else {
      char c;
      for(int i = 0; c = pgm_read_byte(&(errNoCapture[i])); i++) { // + "ERR no-capture\n"
        Serial.write(c);
      }
      c = pgm_read_byte(&(comment[2])); // + "\n"
      Serial.write(c);
    }
    if (found) {
      char c;
      Serial.print (String(val,DEC));
      c = pgm_read_byte(&(comment[2])); // + "\n"
      Serial.write(c); 
    }
  }
}
  
//duty cycle is the percentage of a period a signal is active
void Duty() {
  if((argumentCount(findArgument()) == 1)) {
    String arg1;
    arg1 = command.substring(argOffset,argOffsetEnd);
    uint32_t val=0;
    boolean found =false;
    if (chCount > MAX_EVENT_BUFF) { // buff is full of new readings
      uint8_t oldsreg = SREG;     // save global interrupt flag
      cli();                      // clear global interrupts
      if(arg1.equalsIgnoreCase("h2l")){
        uint8_t i = 0;
        if (!icp[i].rising) { //falling edge is high to low
          i++;
        }  
        val = icp[i].duty;
        found =true; 
      }
      if(arg1.equalsIgnoreCase("l2h")) {
        uint8_t i = 0;
        if (icp[i].rising) {
          i++;
        }
        val = icp[i].duty;
        found =true; 
      }
      if (found) chCount = 0;
      SREG = oldsreg;             // restore global interrupts
    } else {
      char c;
      for(int i = 0; c = pgm_read_byte(&(errNoCapture[i])); i++) { // + "ERR no-capture\n"
        Serial.write(c);
      }
      c = pgm_read_byte(&(comment[2])); // + "\n"
      Serial.write(c);
    }
    if (found) {
      char c;
      Serial.print (String(val,DEC));
      c = pgm_read_byte(&(comment[2])); // + "\n"
      Serial.write(c); 
    }
  }
}

void Count(){
  if((argumentCount(findArgument()) == 0)) {
    uint8_t oldsreg = SREG;     // save global interrupt flag
    cli();                      // clear global interrupts
    uint32_t val=chCount;
    SREG = oldsreg;             // restore global interrupts
    char c;
    Serial.print (String(val,DEC));
    c = pgm_read_byte(&(comment[2])); // + "\n"
    Serial.write(c); 
  }
}


