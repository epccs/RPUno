#ifndef Timers_h
#define Timers_h

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

extern void initTimers(void);
extern unsigned long millis(void);
extern unsigned long micros(void);

#endif // Timers_h
