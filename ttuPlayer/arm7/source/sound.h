
#ifndef _SOUND_H
#define _SOUND_H

// the frequency converter
#define SOUND_FREQUENCY_TO_TMR(frequency) (-(0x1000000 / (frequency)))

void soundQueueInit(void);
void soundQueuePoll(void);

#endif
