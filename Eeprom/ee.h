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

extern void EEread(void);
extern void EEwrite(void);

#endif // ee_H 
