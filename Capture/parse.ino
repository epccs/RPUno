/*
    This file (parse.ino) is part of Captrue.

    ISO C standard and IEEE Std 1003.1-2001 defines 
    isdigit(), isalnum(), isalpha(), iscntrl(), isgraph(), islower(), 
    isprint(), ispunct(), isspace(), isupper(), isxdigit()
*/

void getIncomingChars() {
  // char in gcc is signed, but that is not always true 
  char inChar = Serial.read();
  // "\n" == 10 line feed, "\r" == 13 carriage return,  ";" == 59 
  command.concat(inChar);
  if(inChar == '\n'){
    commandComplete = true;
  }
}

uint8_t findCommand() {
  if (cmdOffset < MAX_CMD_LENGTH) {
    uint8_t lastAlpha = cmdOffset;
    while(isalpha(command.charAt(lastAlpha))) {
      lastAlpha++;
    }
    if(lastAlpha > cmdOffset) {
      if (command.charAt(lastAlpha) == '?') { //commands ending with ? should retrun somthing.
        lastAlpha++;
      }
      cmdOffsetEnd=lastAlpha;
      return cmdOffsetEnd - cmdOffset;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

uint8_t findArgument() {
  if (cmdOffset < MAX_CMD_LENGTH) {
    uint8_t lastAlphaNum = cmdOffsetEnd; // start looking at end of command
    while (isspace(command.charAt(lastAlphaNum)) && !(command.charAt(lastAlphaNum) == '\n')) { //get past any white space, but not end of line
      lastAlphaNum++;
    }
    if((command.charAt(lastAlphaNum) == '\n')) {
      argOffset = MAX_CMD_LENGTH; // end of line, without a argument 
      return 0;
    } 
    if (isalnum(command.charAt(lastAlphaNum))) { //start of argument
      argOffset = lastAlphaNum;
    } else {
      argOffset = MAX_CMD_LENGTH; // problem argument must be alpha/numeric 
      return 0;
    }
    while (isalnum(command.charAt(lastAlphaNum)) || (command.charAt(lastAlphaNum) == ',') ) { //find end of argument
      lastAlphaNum++;
    }
    if(lastAlphaNum > argOffset) {
      argOffsetEnd = lastAlphaNum;
      return argOffsetEnd - argOffset;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

uint8_t argumentCount(uint8_t argSize) {
  if (argSize == (argOffsetEnd - argOffset)) { // findArgument needs to have updated the argument offset
    uint8_t count = 0;
    uint8_t offset = argOffset;
    // skip past firts argument, add it after counting the delimiters
    while (offset < argOffsetEnd) {
      if(command.charAt(offset) == ',') {
        delimter[count]= offset;
        count++;
      }
      offset++;
    }
    count ++;
    return count;
  } else {
    return 0;
  }
}
