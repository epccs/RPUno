#ifndef CCtest_H
#define CCtest_H

// Load Control bits: LD0 is digital 10, LD1 is digital 11, LD2 is digital 12, LD3 is digital 13, 
#define LD0 10
#define LD1 11
#define LD2 12
#define LD3 13
#define START_LD_STEP 0
#define END_LD_STEP 15

extern void CCtest(void);
extern void load_step(uint8_t step);
extern void init_load(void);
extern void init_pv(void);

#endif // CCtest_H 
