#ifndef Adc_h
#define Adc_h

#define ADC_CHANNELS 8
extern volatile int adc[];
extern volatile uint8_t adc_channel;
extern volatile uint8_t ADC_auto_conversion;
extern volatile uint8_t analog_reference;

// define adc referance modes (note DEFAULT is gone, use EXTERNAL_AVCC in its place)
// EXTERNAL_AVCC: connects the analog reference to AVCC power supply with capacitor on AREF pin. 
// INTERNAL_1V1: a built-in 1.1V reference with bypass capacitor on AREF pin.
// INTERNAL_2V56: a built-in 2.56V reference with bypass capacitor on AREF pin.
// INTERNAL_2V56WOBP: a built-in 2.56V reference with out a bypass capacitor on AREF pin.
// EXTERNAL_AREF: the voltage applied to the AREF pin, with band gap turned off

// ADREFSMASK is used to clear all referance select (REFS) bits befor setting the needed bits
#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define ADREFSMASK (1<<REFS2) | (1<<REFS1) | (1<<REFS0)
#define EXTERNAL_AVCC 0
#define EXTERNAL_AREF (1<<REFS0)
#define INTERNAL_1V1 (1<<REFS1) 
#define INTERNAL_2V56WOBP (1<<REFS2) | (1<<REFS1) 
#define INTERNAL_2V56 (1<<REFS2) | (1<<REFS1) | (1<<REFS0)
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define ADREFSMASK (1<<REFS1) | (1<<REFS0)
#define EXTERNAL_AREF 0
#define EXTERNAL_AVCC (1<<REFS0)
#define INTERNAL_1V1 (1<<REFS1)
#define INTERNAL_2V56 (1<<REFS1) | (1<<REFS0)
#endif

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || (__AVR_ATmega168P__) || defined (__AVR_ATmega168__)
#define ADREFSMASK (1<<REFS1) | (1<<REFS0)
#define EXTERNAL_AREF 0
#define EXTERNAL_AVCC (1<<REFS0)
#define INTERNAL_1V1 (1<<REFS1) | (1<<REFS0)
#endif

#ifndef EXTERNAL_AVCC
#   error your mcu is not supported
#endif
extern void init_ADC_single_conversion(uint8_t reference);

#define FREE_RUNNING 1
#define BURST_MODE 0
extern void enable_ADC_auto_conversion(uint8_t free_run);

extern void analogReference(uint8_t mode);
extern int analogRead(uint8_t channel);

#endif // Adc_h
