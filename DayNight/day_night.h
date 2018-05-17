#ifndef DayNight_H
#define DayNight_H

extern void Day(unsigned long);

extern void CheckDayLight(uint8_t);
extern uint8_t DayState(void);

// Warning the pointer to function will be NULL if it is not set (registered) during initialization.
extern void Day_AttachWork( void (*)(void) );
extern void Night_AttachWork( void (*)(void) );

extern int morning_threshold;
extern int evening_threshold;
extern unsigned long evening_debouce;
extern unsigned long morning_debouce;

#define DAYNIGHT_START_STATE 0
#define DAYNIGHT_DAY_STATE 1
#define DAYNIGHT_EVENING_DEBOUNCE_STATE 2
#define DAYNIGHT_NIGHTWORK_STATE 3
#define DAYNIGHT_NIGHT_STATE 4
#define DAYNIGHT_MORNING_DEBOUNCE_STATE 5
#define DAYNIGHT_DAYWORK_STATE 6
#define DAYNIGHT_FAIL_STATE 7

#endif // DayNight_H 
