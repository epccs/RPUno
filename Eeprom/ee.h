#ifndef ee_H
#define ee_H

#if defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168__)
#define EEPROM_SIZE 512
#endif

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
#define EEPROM_SIZE 1024
#endif

#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define EEPROM_SIZE 4096
#endif

#ifndef EEPROM_SIZE
#   error your mcu is not supported
#endif

// macros to automaticly cast eeprom access  
#define read_eeprom_byte(address) eeprom_read_byte ((const uint8_t*)address)
#define write_eeprom_byte(address,value) eeprom_write_byte ((uint8_t*)address,(uint8_t)value)
#define read_eeprom_word(address) eeprom_read_word ((const uint16_t*)address)
#define write_eeprom_word(address,value) eeprom_write_word ((uint16_t*)address,(uint16_t)value)
#define read_eeprom_dword(address) eeprom_read_dword ((const uint32_t*)address)
#define write_eeprom_dword(address,value) eeprom_write_dword ((uint32_t*)address,(uint32_t)value)
#define read_eeprom_float(address) eeprom_read_float ((const float *)address)
#define write_eeprom_float(address,value) eeprom_write_float ((float*)address,(float)value)
#define read_eeprom_array(address,value_p,length) eeprom_read_block ((void *)value_p, (const void *)address, length)
#define write_eeprom_array(address,value_p,length) eeprom_write_block ((const void *)value_p, (void *)address, length)

extern void EEread(void);
extern void EEwrite(void);

#endif // ee_H 
