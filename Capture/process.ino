/*
    This file (process.ino) is part of Capture.
    id?            // Returns the device identify string
*/

void processCommand(){ 
  if (findCommand()) {
    String cmd;
    cmd = command.substring(cmdOffset,cmdOffsetEnd);
    if (cmd.equalsIgnoreCase("count?")){ // Pulse or cycle count on input capture. 
      Count();
    } else if (cmd.equalsIgnoreCase("duty?")) { // pulse active time, from rising edge to falling edge, needs one arg.
      Duty();
    } else if (cmd.equalsIgnoreCase("id?")) { // Identify self
      char c;
      for(int i = 0; c = pgm_read_byte(&(identify[i])); i++) {
        Serial.write(c);
      }
    } else if (cmd.equalsIgnoreCase("period?")) { // Input capture period, needs one arg.
      Period();
    } else if (cmd.equalsIgnoreCase("pwm")) { // pwm duty, needs one arg.
      Pwm();
    } 
  } 
}
