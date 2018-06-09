#ifndef NightLight_H
#define NightLight_H

// CLI commands
extern void NLDelayStart(void);
extern void NLRunTime(void);
extern void NLDelay(void);
extern void NLAHrStop(void);
extern void NLRun(void);
extern void NLSave(void);
extern void NLLoad(void);
extern void NLTime(void);
extern void NLStop(void);

// functions for main() or others
extern void LedControl(void);
extern void Reset_All_LED(void);
extern void StopLED(uint8_t);
extern uint8_t LoadLedControlFromEEPROM(uint8_t);
extern uint8_t NLLive(uint8_t);
extern uint8_t StartLed(uint8_t);
extern void init_Led(void);

#define LEDSTRING_COUNT 4

#endif // NightLight_H 
