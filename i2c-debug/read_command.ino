#include "i2c-debug.h"

// Handle incoming Serial data, try to find a command in there
void read_command_bytes()
{
    for (uint8_t d = Serial.available(); d > 0; d--)
    {
        incoming_command[incoming_position] = Serial.read();
        // Check for line end and in such case do special things
        if (   incoming_command[incoming_position] == 0xA // LF
            || incoming_command[incoming_position] == 0xD) // CR
        {
            incoming_command[incoming_position] = 0x0;
            if (   incoming_position > 0
                && (   incoming_command[incoming_position-1] == 0xD // CR
                    || incoming_command[incoming_position-1] == 0xA) // LF
               )
            {
                incoming_command[incoming_position-1] = 0x0;
            }
            have_command = 1;
            return;
        }
        incoming_position++;

        // Sanity check buffer sizes
        if (incoming_position > MAX_COMMAND_LENGTH+2)
        {
            Serial.println(0x15); // NACK
            Serial.print(F("PANIC: No end-of-line seen and incoming_position="));
            Serial.print(incoming_position, DEC);
            Serial.println(F(" clearing buffers"));
            
            memset(incoming_command, 0, MAX_COMMAND_LENGTH+2);
            incoming_position = 0;
        }
    }
}
