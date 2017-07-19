#ifndef PwrStrg_H
#define PwrStrg_H

extern void Charge(void);
extern void Discharge(void);
extern void Remaining(void);

extern void CheckChrgAccumulation(void);
extern uint8_t init_ChargAccumulation(void);

extern float ChargeAccum(void);
extern float DischargeAccum(void);
extern float RemainingAccum(void);

#endif // PwrStrg_H
