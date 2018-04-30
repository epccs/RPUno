#ifndef CS_H
#define CS_H

#define NUM_CS_PINS 4

typedef struct {
  uint8_t pin; 
} CS_Map;

static const CS_Map cs_pin_map[NUM_CS_PINS] = {
    [0] = { .pin=5 }, // CS0 is controled with DIO5 which is defined in ../lib/pin_num.h
    [1] = { .pin=6 },
    [2] = { .pin=3 },
    [3] = { .pin=4 }
};

extern void CurrSour(void);

#endif // CS_H 
