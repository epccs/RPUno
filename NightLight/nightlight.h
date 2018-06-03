#ifndef NightLight_H
#define NightLight_H

// CLI commands
extern void DelayStart(void);
extern void RunTime(void);
extern void Delay(void);
extern void AHrStop(void);
extern void Run(void);
extern void Save(void);
extern void Load(void);
extern void Time(void);
extern void Stop(void);
extern void LedControl(void);
extern void Reset_All_LED(void);

// functions for main() or others
extern void StopLED(uint8_t led);
extern uint8_t LoadLedControlFromEEPROM(uint8_t);
extern uint8_t Live(uint8_t);
extern uint8_t StartLed(uint8_t);
extern void init_Led(void);

#define LEDSTRING_COUNT 4

#endif // NightLight_H 
