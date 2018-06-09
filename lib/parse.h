#ifndef PARSE_H
#define PARSE_H

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
extern unsigned long is_arg_in_ul_range (uint8_t, unsigned long, unsigned long);
extern uint8_t is_arg_in_uint8_range (uint8_t, uint8_t, uint8_t);

extern uint8_t command_done;
extern uint8_t echo_on;
extern char command_buf[];
extern char *command;
extern char *arg[MAX_ARGUMENT_COUNT];
extern uint8_t arg_count;

#endif // PARSE_H 
