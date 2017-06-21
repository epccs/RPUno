#ifndef DayNight_H
#define DayNight_H

extern void Day(void);

extern void CheckDayLight(void);
extern uint8_t DayState(void);

extern void Day_AttachWork( void (*)(void) );
extern void Night_AttachWork( void (*)(void) );

#define DAYNIGHT_START_STATE 0
#define DAYNIGHT_DAY_STATE 1
#define DAYNIGHT_EVENING_DEBOUNCE_STATE 2
#define DAYNIGHT_NIGHTWORK_STATE 3
#define DAYNIGHT_NIGHT_STATE 4
#define DAYNIGHT_MORNING_DEBOUNCE_STATE 5
#define DAYNIGHT_DAYWORK_STATE 6
#define DAYNIGHT_FAIL_STATE 7

#endif // DayNight_H 
