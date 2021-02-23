
#ifndef _BIOS_H
#define _BIOS_H

extern void swiDelay(u32 duration);
extern void swiIntrWait(s32 waitForSet, u32 flags);
extern void swiWaitForVBlank(void);
extern int  swiDivide(s32 numerator, s32 divisor);
extern int  swiRemainder(s32 numerator, s32 divisor);
extern void swiDivMod(s32 numerator, s32 divisor, s32 *result, s32 *remainder);
extern int  swiSqrt(s32 value);

#endif
