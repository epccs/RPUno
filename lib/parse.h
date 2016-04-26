#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

// Got your attention,  just so you know it may or not work with other versions.
#if (__GNUC__ * 100 + __GNUC_MINOR__) != 409
#error "This library was used with gcc-avr package (4.9.2+Atmel3.5.0-1) on Ubuntu 16.04"
#endif

// command line buffer (buffer size of up to 32 on AVR uses the fast ldd instruction)
#define COMMAND_BUFFER_SIZE 32
#define COMMAND_BUFFER_MASK (COMMAND_BUFFER_SIZE - 1)

// arguments that can be found after a command on the command line
#define MAX_ARGUMENT_COUNT 5
#define ARGUMNT_DELIMITER ','


extern void initCommandBuffer(void);
extern void StartEchoWhenAddressed(char);
extern void AssembleCommand(char);
extern uint8_t findArgument(uint8_t);
extern uint8_t findCommand(void);

extern uint8_t command_done;
extern uint8_t echo_on;
extern char command_buf[];
extern char *command;
extern char *arg[MAX_ARGUMENT_COUNT];
extern uint8_t arg_count;

#endif // PARSE_H 
