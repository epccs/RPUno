#ifndef ChrgAccum_H
#define ChrgAccum_H

extern void Charge(unsigned long);
extern uint8_t ResetChargeAccum(void);

extern void CheckChrgAccumulation(uint8_t);
extern uint8_t init_ChargAccumulation(uint8_t);

extern float ChargeAccum(uint8_t);

#endif // ChrgAccum_H
