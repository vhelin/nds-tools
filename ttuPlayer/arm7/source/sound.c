
#include "ndsARM7.h"

#include "defines.h"
#include "memory.h"
#include "bios.h"
#include "shared.h"

#include "sound.h"
#include "ttu.h"


// divide the hardware channels into dedicated groups
#define SOUND_CHANNELS_SFX    4
#define SOUND_CHANNELS_MUSIC  4
#define SOUND_CHANNELS_PLANES 8

// the SFX channel index
static s32 soundSFXChannel __attribute__((section(".bss")));

// the SFX status
static s32 soundSFXEnabled __attribute__((section(".bss")));

// channel data pointer table
static s32 soundChannelDataPtr[16] __attribute__((section(".bss")));


void soundQueueInit(void) {

  s32 i;

  // zero the play data table
  for (i = 0; i < 16; i++)
    soundChannelDataPtr[i] = 0;

  // reset the SFX player
  soundSFXChannel = 0;
  soundSFXEnabled = YES;

  // reset the queue
  sharedMemory[SOUND_QUEUE_ARM7_INDEX] = 0;
  sharedMemory[SOUND_QUEUE_ARM9_INDEX] = 0;
  sharedMemory[SOUND_QUEUE_ITEMS_N] = 0;
  sharedMemory[SOUND_QUEUE_READY] = YES;
}


void soundQueuePoll(void) {

  u32 command, data1, data2, data3, data4, data5, data6;
  s32 i;
  vu32 *queue;

  queue = (vu32 *)&sharedMemory[SOUND_QUEUE_ITEMS];

  while (sharedMemory[SOUND_QUEUE_ITEMS_N] > 0) {
    // parse the next command
    i = sharedMemory[SOUND_QUEUE_ARM7_INDEX] << 2;

    command = queue[i++];
    data1 = queue[i++];
    data2 = queue[i++];
    data3 = queue[i];

    // parse data4, data5 and data6 from command
    data4 = (command >>  8) & 0xFF;
    data5 = (command >> 16) & 0xFF;
    data6 = command >> 24;

    command &= 0xFF;

    switch (command) {
    case SOUND_QUEUE_COMMAND_SET_MASTER_VOLUME:
      REG_SOUNDCNT = BIT_SOUNDCNT_SOUND_ENABLE | SOUNDCNT_SOUND_VOLUME(data1);
      break;

    case SOUND_QUEUE_COMMAND_TOGGLE_SFX:
      // enable / disable the sound effects
      soundSFXEnabled = data1;

      if (data1 == NO) {
        // reset all SFX channels
        for (i = 0; i < SOUND_CHANNELS_SFX; i++)
          SOUND_CHANNEL_CR(i) = 0;
        for (i = 0; i < SOUND_CHANNELS_PLANES; i++)
          SOUND_CHANNEL_CR(SOUND_CHANNELS_SFX + SOUND_CHANNELS_MUSIC + i) = 0;
      }

      break;

    case SOUND_QUEUE_COMMAND_PLAY_SFX:
      // play the sound effect on the next (round robin) channel
      if (soundSFXEnabled == NO)
        break;

      // reset the channel
      SOUND_CHANNEL_CR(soundSFXChannel) = 0;

      SOUND_CHANNEL_TIMER(soundSFXChannel)  = data1; // frequency
      SOUND_CHANNEL_SOURCE(soundSFXChannel) = data2; // address
      SOUND_CHANNEL_LENGTH(soundSFXChannel) = data3; // length

      SOUND_CHANNEL_CR(soundSFXChannel) = BIT_SOUNDxCNT_ENABLE | SOUNDxCNT_VOLUME(data5) | SOUNDxCNT_PANNING(data4) | SOUNDxCNT_REPEAT_ONE_SHOT | SOUNDxCNT_FORMAT_IMA_ADPCM;

      soundSFXChannel++;
      if (soundSFXChannel == SOUND_CHANNELS_SFX)
        soundSFXChannel = 0;

      break;

    case SOUND_QUEUE_COMMAND_PLAY_TTU:
      // start to play the given TTU
      ttuStart(SOUND_CHANNELS_SFX, SOUND_CHANNELS_MUSIC, (u32 *)data1, (u32 *)data2);
      break;

    case SOUND_QUEUE_COMMAND_STOP_TTU:
      // stop the currently playing TTU
      ttuStop();
      break;

    case SOUND_QUEUE_COMMAND_SET_ENGINE_SOUND:
      // change the engine sound parameters for plane data6 [0-7]
      if (soundSFXEnabled == NO)
        break;

      i = SOUND_CHANNELS_SFX + SOUND_CHANNELS_MUSIC + data6;

      // reset the channel if the sample is different
      if (soundChannelDataPtr[i] != data2) {
        SOUND_CHANNEL_CR(i) = 0;

        // remember the pointer
        soundChannelDataPtr[i] = data2;
      }

      SOUND_CHANNEL_TIMER(i)  = data1; // frequency
      SOUND_CHANNEL_SOURCE(i) = data2; // address
      SOUND_CHANNEL_LENGTH(i) = data3; // length
      SOUND_CHANNEL_REPEAT_POINT(i) = 0;

      SOUND_CHANNEL_CR(i) = BIT_SOUNDxCNT_ENABLE | SOUNDxCNT_VOLUME(data5) | SOUNDxCNT_PANNING(data4) | SOUNDxCNT_REPEAT_LOOP | SOUNDxCNT_FORMAT_PCM8;

      break;

    default:
      // we should never come here
      break;
    }

    sharedMemory[SOUND_QUEUE_ARM7_INDEX]++;
    if (sharedMemory[SOUND_QUEUE_ARM7_INDEX] > SOUND_QUEUE_ITEMS_MAX)
      sharedMemory[SOUND_QUEUE_ARM7_INDEX] = 0;

    sharedMemory[SOUND_QUEUE_ITEMS_N]--;
  }
}
