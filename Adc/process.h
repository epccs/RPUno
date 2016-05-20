#ifndef PROCESS_H
#define PROCESS_H

// Got your attention,  just so you know it may or not work with other versions.
#if (__GNUC__ * 100 + __GNUC_MINOR__) != 409
#error "This library was used with gcc-avr package (4.9.2+Atmel3.5.0-1) on Ubuntu 16.04"
#endif


/* tentative external linkage */
extern void ProcessCmd(void);

/* internal linkage */
//extern void IdCommand(void);

#endif // PROCESS_H 
