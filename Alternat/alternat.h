#ifndef Alternat_H
#define Alternat_H

extern void EnableAlt(void);
extern void AltCount(void);

extern void check_if_alt_should_be_on(uint8_t, float, float);

extern  uint8_t alt_enable;
extern unsigned int alt_count;

#endif // Alternat_H 
