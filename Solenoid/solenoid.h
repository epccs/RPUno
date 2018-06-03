#ifndef Solenoid_H
#define Solenoid_H

// CLI commands
extern void KDelayStart(void);
extern void KRunTime(void);
extern void KDelay(void);
extern void KFlowStop(void);
extern void KRun(void);
extern void KSave(void);
extern void KLoad(void);
extern void KTime(void);
extern void KFlow(void);
extern void KStop(void);

// functions for main() or others
extern void KControl(void);
extern void Reset_All_K(void);
extern void StopK(uint8_t);
extern uint8_t LoadKControlFromEEPROM(uint8_t);
extern uint8_t KLive(uint8_t);
extern uint8_t StartK(uint8_t);
extern void init_K(void);

#define SOLENOID_COUNT 3

#endif // Solenoid_H 
