#ifndef I2C_DEVICE_DEBUG
#define I2C_DEVICE_DEBUG

#include <Wire.h>

#define MAX_COMMAND_LENGTH 100 // null-terminated
#define I2C_DATA_LENGTH 10
extern char incoming_command[MAX_COMMAND_LENGTH+2]; //Reserve space for CRLF too.
extern uint8_t incoming_position; // Arduino.h defins byte as unit8_t, so that means byte is not a standard type. 
extern uint8_t have_command;
extern unsigned long jiffies_last_blink;

extern void read_command_bytes();
extern void process_command();

extern uint8_t start_address;
extern uint8_t end_address;

extern void scanFunc( byte addr, byte result );

#endif

