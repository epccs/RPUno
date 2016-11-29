#ifndef Solenoid_H
#define Solenoid_H

extern void RunTime(void);
extern void Delay(void);
extern void Run(void);

extern void SolenoidControl(void);
extern void Reset_All_K(void);
extern uint8_t Live(uint8_t);
extern void init_K(void);

#define SOLENOID_COUNT 3

#endif // Solenoid_H 