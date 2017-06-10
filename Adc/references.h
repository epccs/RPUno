#ifndef References_H
#define References_H

extern uint8_t IsValidValForAvccRef(uint32_t *);
extern uint8_t IsValidValFor1V1Ref(uint32_t *);
extern uint8_t LoadAnalogRefFromEEPROM();
extern uint8_t WriteEeReferenceId();
extern uint8_t WriteEeReferenceAvcc();
extern uint8_t WriteEeReference1V1();

extern uint32_t ref_extern_avcc_uV;
extern uint32_t ref_intern_1v1_uV;

#endif // Analog_H 
