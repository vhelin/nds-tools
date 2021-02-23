
#ifndef _SHARED_H
#define _SHARED_H

// address of the shared memory block
extern vu16 *sharedMemory;

/* variables shared by ARM7 and ARM9 */

///////////////////////////////////////////////////////////////////////////////
// touch screen
///////////////////////////////////////////////////////////////////////////////

#define SHARED_TOUCH_PX      0x0000
#define SHARED_TOUCH_PY      0x0001
#define SHARED_TOUCH_PRESSED 0x0002

///////////////////////////////////////////////////////////////////////////////
// keys
///////////////////////////////////////////////////////////////////////////////

#define SHARED_KEYS          0x0003

///////////////////////////////////////////////////////////////////////////////
// ARM7 free CPU time
///////////////////////////////////////////////////////////////////////////////

#define SHARED_ARM7_FREE     0x0004

///////////////////////////////////////////////////////////////////////////////
// sound queue
///////////////////////////////////////////////////////////////////////////////

#define SOUND_QUEUE_ITEMS_MAX 32

#define SOUND_QUEUE_ARM7_INDEX  0x0010
#define SOUND_QUEUE_ARM9_INDEX  0x0011

// the number of items in the queue
#define SOUND_QUEUE_ITEMS_N     0x0012

// is the queue ready?
#define SOUND_QUEUE_READY       0x0013

// the items
#define SOUND_QUEUE_ITEMS       0x0020

// each item (16 bytes) consists of
// * command ID (u32)
// * data 1     (u32)
// * data 2     (u32)
// * data 3     (u32)

// command IDs
#define SOUND_QUEUE_COMMAND_PLAY_SFX          0
#define SOUND_QUEUE_COMMAND_PLAY_TTU          1
#define SOUND_QUEUE_COMMAND_STOP_TTU          2
#define SOUND_QUEUE_COMMAND_SET_MASTER_VOLUME 3
#define SOUND_QUEUE_COMMAND_SET_ENGINE_SOUND  4
#define SOUND_QUEUE_COMMAND_TOGGLE_SFX        5

#endif
