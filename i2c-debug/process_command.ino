#include "i2c-debug.h"

enum parser_states {
    start_seen,
    stop_seen,
    in_hex,
    delimite,  // each command is space delimited
    raad_seen
};

uint8_t parser_state = delimite;
uint8_t next_parser_state = delimite;
uint8_t prev_parser_state = delimite;

uint8_t i2c_address;
uint8_t i2c_data_pos;
uint8_t i2c_data[I2C_DATA_LENGTH];
uint8_t i2c_bytes2read;
uint8_t i2c_start;

char hexparsebuffer[5];
uint8_t hexparsebuffer_i = 0;

// isxdigit(int) is a standared character classification routine in avr-libc
// Checks for a hexadecimal digits, i.e. one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F. 
// http://www.nongnu.org/avr-libc/user-manual/group__ctype.html


// Parses ASCII [0-9A-F] hexadecimal to byte value
// avr-libc has itoa(int val, char *s, int radix)
// http://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html
uint8_t ardubus_hex2byte(uint8_t hexchar)
{
  if ((0x40 < hexchar) && (hexchar < 0x47)) // A-F
  {
    return (hexchar - 0x41) + 10; 
  }
  if ((0x60 < hexchar) && (hexchar < 0x67)) // a-f
  {
    return (hexchar - 0x61) + 10; 
  }
  if ((0x2f < hexchar) && (hexchar < 0x3a)) // 0-9
  {
    return (hexchar - 0x30);
  }
  return 0x0; // Failure.
}

uint8_t ardubus_hex2byte(uint8_t hexchar0, uint8_t hexchar1)
{
    return (ardubus_hex2byte(hexchar0) << 4) | ardubus_hex2byte(hexchar1);
}

int ardubus_hex2int(uint8_t hexchar0, uint8_t hexchar1, uint8_t hexchar2, uint8_t hexchar3)
{
    return ardubus_hex2byte(hexchar0, hexchar1) << 8 | ardubus_hex2byte(hexchar2, hexchar3);
}

uint8_t parse_hex(char *parsebuffer)
{
  uint8_t len = strlen(parsebuffer);
  if (len > 2)
  {
    //Serial.println(parsebuffer);
    return 0xff;
  }
  if (len == 2)
  {
    return ardubus_hex2byte(parsebuffer[0], parsebuffer[1]);
  }
  return ardubus_hex2byte(parsebuffer[0]);
}

void parse_into_hexparsebufer_from_command(uint8_t current_pos)
{
  if (prev_parser_state == delimite) // went past first hex when state was set
  {
    if (isxdigit(incoming_command[current_pos-1]))
    {
      hexparsebuffer[hexparsebuffer_i++] = incoming_command[current_pos-1];
    }
  }
  if (isxdigit(incoming_command[current_pos]))
  {
    hexparsebuffer[hexparsebuffer_i++] = incoming_command[current_pos];
    if (hexparsebuffer_i > 2)
    {
      Serial.println(F("Can only have byte wide hex strings"));
      clear_command_buffer_and_reset_position();
      return;
    }
  }
}

void invalid_char(int character, uint8_t pos)
{
  Serial.print(F("Invalid character '"));
  Serial.write(character);
  Serial.print(F("' (0x"));
  Serial.print(character, HEX);
  Serial.print(F(") in position "));
  Serial.print(pos, DEC);
  Serial.println(F(" when parsing command"));
}

void clear_command_buffer_and_reset_position() 
{
  memset(incoming_command, 0, MAX_COMMAND_LENGTH+2);
  i2c_address = 0;
  i2c_start = 0;
  for(uint8_t i = 0; i < I2C_DATA_LENGTH; i++) 
  {
    i2c_data[i]=0;
  }
  i2c_data_pos = 0;
  i2c_bytes2read = 0;
  incoming_position = 0;
  have_command = 0;
  parser_state = delimite;
}

// count all read commands retruns stop position
uint8_t count_all_i2c_read(uint8_t current_pos)
{
  while (current_pos < strlen(incoming_command))
  {
    switch (incoming_command[current_pos])
    {
      case 'r':
      case 'R':
          i2c_bytes2read++;
          Serial.print(F("r count: "));
          Serial.println(i2c_bytes2read,DEC);
          if( !(incoming_command[current_pos+1] == ' ') )
          {
            // delimit must follow
            Serial.print(F("space must follow r, not a: "));
            invalid_char(incoming_command[current_pos], current_pos);
            clear_command_buffer_and_reset_position();
            return 0;
          }
          break;
      case ' ': // command delimiter
          break;
      case ']': // stop signifier
          return current_pos;
      default:
          Serial.print(F("after r only another r or ] may follow: "));
          invalid_char(incoming_command[current_pos], current_pos);
          clear_command_buffer_and_reset_position();
          return 0;
    }
    current_pos++;
  }
}

void set_new_state_if_char_is_valid_durring_delimit(uint8_t current_pos)
{
  if (parser_state != delimite)
  {
    Serial.println(F("is_valid_char_after_delimit() ran with a parser_state other than delimite"));
    clear_command_buffer_and_reset_position();
    return;
  }
  if (isxdigit(incoming_command[current_pos]))
  {
    // Clear buffer hexparsebuffer
    memset(hexparsebuffer, 0, sizeof(hexparsebuffer));
    hexparsebuffer_i = 0;
    parser_state = in_hex;
    prev_parser_state = delimite;
    return;
  }
  switch (incoming_command[current_pos])
  {
    case ' ': // delimite
        break;
    case '[': // start signifier
        parser_state = start_seen;
        i2c_start = true;
        Serial.println(F("START seen"));
        break;
    case ']': // stop signifier
        Serial.print(F("beginTransmission of "));
        Serial.print(i2c_data_pos, DEC);
        Serial.println(F(" bytes"));
        Wire.beginTransmission(i2c_address);
        for(uint8_t i=0; i<i2c_data_pos; i++)
        {
          Wire.write(i2c_data[i]);
          Serial.print(F("write 0x"));
          Serial.println(i2c_data[i], HEX);
        }
        Wire.endTransmission();
        parser_state = stop_seen;
        break;
    case 'r': 
    case 'R':
        count_all_i2c_read(current_pos);
        if (i2c_bytes2read)
        {
          Serial.print(F("begin read of "));
          Serial.print(i2c_bytes2read, DEC);
          Serial.println(F(" bytes"));
          Wire.requestFrom(i2c_address, i2c_bytes2read);
          i2c_data_pos= 0;
          while(Wire.available())
          {
            i2c_data[i2c_data_pos] = Wire.read();
            Serial.print(F("read 0x"));
            Serial.println(i2c_data[i2c_data_pos], HEX);
            i2c_data_pos++;
          }
          // kill the command now cause read has bypassed the parser 
          clear_command_buffer_and_reset_position(); 
        }
        break;
  }
  prev_parser_state = delimite;
}

void parser_state_options_for_parsed_hex(uint8_t parsed_byte)
{
  if (i2c_start && !(i2c_address)) 
  {
    i2c_data_pos = 0;
    i2c_address = parsed_byte;
    Serial.print(F("address"));
    Serial.print(F("(0x"));
    Serial.print(i2c_address, HEX);
    Serial.println(F(")"));
  }
  else
  { 
    i2c_data[i2c_data_pos] = parsed_byte;
    Serial.print(F("data"));
    Serial.print(F("(0x"));
    Serial.print(i2c_data[i2c_data_pos], HEX);
    Serial.println(F(")"));
    i2c_data_pos++;
  }
}

void process_command()
{
  for(uint8_t i=0; i < strlen(incoming_command); i++)
  {
    if (parser_state == start_seen)
    {
      if (incoming_command[i] == ' ')
      {
        parser_state = delimite;
        prev_parser_state = start_seen;
        continue;
      }
      else
      {
        Serial.print(F("use a space after start, not a: "));
        invalid_char(incoming_command[i], i);
        clear_command_buffer_and_reset_position();
        return;
      }
    }
    if (parser_state == stop_seen)
    {
      if (incoming_command[i] == ' ')
      {
        parser_state = delimite;
        prev_parser_state = stop_seen;
        continue;
      }
      else
      {
        Serial.print(F("stop can only be followed by space, not a: "));
        invalid_char(incoming_command[i], i);
        clear_command_buffer_and_reset_position();
        return;
      }
    }
    if (parser_state == in_hex)
    {
      parse_into_hexparsebufer_from_command(i);
      if (((i+1) >= (strlen(incoming_command)-1)) || (incoming_command[i+1] == ' ')) // (end of string) or space 
      {
        uint8_t parsed_byte = parse_hex(hexparsebuffer);
        parser_state_options_for_parsed_hex(parsed_byte);
        prev_parser_state = in_hex;
        parser_state = delimite;
        continue;
      }
      else
      {
        Serial.print(F("in_hex: "));
        invalid_char(incoming_command[i], i);
        clear_command_buffer_and_reset_position();
        return;
      }
    }
    if (parser_state == delimite)
    {
      set_new_state_if_char_is_valid_durring_delimit(i);
      // note do not update the prev_parser_state, so that multiple spaces can be used
      continue;
    }
  }
  clear_command_buffer_and_reset_position();
}

