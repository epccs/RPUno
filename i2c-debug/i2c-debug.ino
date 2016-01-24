/* by Ronald Sutherland http://epccs.org/hg/open/Uno/
* 
* scanner is from Tod E. Kurt, https://github.com/todbot/arduino-i2c-scanner
* the basic tool is from https://github.com/rambo/I2C
*/

#include "i2c-debug.h"
char incoming_command[MAX_COMMAND_LENGTH + 2];
uint8_t incoming_position;
uint8_t have_command;
unsigned long jiffies_last_blink;

uint8_t start_address = 0x8; // 0-0x7 is reserved (e.g. general call, CBUS, Hs-mode...)
uint8_t end_address = 0x77; // 0x78-0x7F is reserved (e.g. 10-bit...)

/**
 * I2C commands are interpreted as follows
 * 
 * [ -> start
 * ] -> stop
 * 00-FF -> two hex numbers are a byte 0-255
 * r -> read one byte
 *
 * each command is space delimited
 *
 * scan of I2C bus shows all 7 bit devices at startup
 *...
 * addr: 0x38 r70 w71	addr: 0x39  ...
 * addr: 0x3C        	addr: 0x3D ...
 * ...
 *
 * the 0x38 is a PCA9554, ignore the read and write address
 * set the configuration register 3 for outputs (its a DDR)
 * 
 * [ 38 03 00 ]
 * START seen
 * address(0x38)
 * data(0x3)
 * data(0x0)
 * beginTransmission of 2 bytes
 * write 0x3
 * write 0x0
 * 
 * read register 3 back (it will stay addressed with this chip)
 * 
 * [ 38 r ]
 * START seen
 * address(0x38)
 * r count: 1
 * begin read of 1 bytes
 * read 0x0
 * 
 * register 1 will drive the outputs which turn on alternate LED's
 * [ 38 01 AA ]
 * [ 38 01 55 ]
 */

#include <ctype.h>
extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called
void scanI2CBus(byte from_addr, byte to_addr, 
                void(*callback)(byte address, byte result) ) 
{
  byte rc;
  byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr = from_addr; addr <= to_addr; addr++ ) {
    rc = twi_writeTo(addr, &data, 0, 1, 0);
    callback( addr, rc );
  }
}

// Called when address is found in scanI2CBus()
// Feel free to change this as needed
// (like adding I2C comm code to figure out what kind of I2C device is there)
void scanFunc( byte addr, byte result ) {
  Serial.print(F("addr: 0x"));
  Serial.print(addr,HEX);
  if (result == 0) 
  {
    Serial.print( F(" r"));
    Serial.print(addr<<1,HEX);
    Serial.print( F(" w"));
    Serial.print((addr<<1) + 1,HEX);
  }
  else
  {
    Serial.print( F("        "));
  }
  Serial.print( ((addr+1)%4) ? F("\t"):F("\n"));
}

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(115200);
  Wire.begin();
  delay(1000); // Arduino IDE some time to swap between avrdude and serial port 
  digitalWrite(13, LOW);
  have_command = 0;
  jiffies_last_blink = millis();
  clear_command_buffer_and_reset_position();
  Serial.println(F("Scan I2C bus."));
  scanI2CBus( start_address, end_address, scanFunc );
  Serial.println(F("Scan done."));
}

void loop()
{
    read_command_bytes();
    if (have_command) 
    {
      Serial.print(F("process command:"));
      Serial.println(incoming_command);
      process_command();
    }
    if ((millis() - jiffies_last_blink) > 500 ) 
    {
      digitalWrite(13, !digitalRead(13));
      jiffies_last_blink = millis();
    }
}
