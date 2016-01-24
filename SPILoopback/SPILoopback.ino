/*
  Send Data to look at with scope

  Loop Back test for the SPI port of an Arduino

  modified 30 Nov 2015
  by Ronald Sutherland
 */

#include <SPI.h> // https://www.arduino.cc/en/Reference/SPI
// A clock slow enough for an attiny85 @ 1MHz, is reasonable
#define SPI_CLOCK     (1000000/6)
uint8_t outByte;
uint8_t inByte; 
int led = 13;
uint8_t error = 0; 


void setup() {
  pinMode(led, OUTPUT); // setting the LED pin as output
  Serial.begin(9600);
  SPI.begin(); // initialize the SPI port as master
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));
  delay(100);  
}

void loop() {
  digitalWrite(led, HIGH);
  error = 0;   
  for(outByte = 0; outByte <= 255; outByte++)
  {
    inByte = SPI.transfer(outByte);
    if( inByte != outByte) 
    {
      error = 1;
      break;
    }
  }
  if(error)
  {
    Serial.print("error outByte=");
    Serial.print(outByte, DEC);
    Serial.print(" inByte=");
    Serial.print(inByte, DEC);
    Serial.println();
  }
  else 
  {
    Serial.println("no errors");
  }

  delay(1000);
  digitalWrite(led, LOW); 
  delay(1000);
}
