/*
    This file (pwm.ino) is part of Capture.
    timer2:A is setup for pwm on pin 11 (e.g. IO11 or MOSI) and is free to use
    pwm [3 thru 252]
*/

void Pwm(){
  if((argumentCount(findArgument()) == 1)) {
    String arg1;
    arg1 = command.substring(argOffset,argOffsetEnd);
    int duty = arg1.toInt(); // a non-digit is 0, so I may need to test for that
    if (duty >= 3 && duty <= 252) {
      analogWrite(11, duty);
    } 
  }
}
