
#include "ndsARM7.h"

#include <dswifi7.h>

#include "defines.h"
#include "memory.h"
#include "bios.h"
#include "shared.h"

#include "sound.h"
#include "touch.h"
#include "ttu.h"
#include "firmware.h"


// startup delay
s32 startupVBICounter __attribute__((section(".bss")));

// free scanlines table
u8  freeScanlines[263] __attribute__((section(".bss")));

// address of the shared memory block
vu16 *sharedMemory __attribute__((section(".bss")));

// the touch structure for VBI (taken from heap, not IRQ stack)
static struct touch touch __attribute__((section(".bss")));

// start scanline for irqHandler()
s32 scanlineStart __attribute__((section(".bss")));


void irqHandler(void) {

  // remember the scanline where this irq started
  scanlineStart = REG_VCOUNT;

  if (REG_IF & BIT_IE_WIFI) {
    // WIFI
    Wifi_Interrupt();

    // end
    IRQ_CHECK |= BIT_IE_WIFI;
    REG_IF |= BIT_IE_WIFI;
  }

  if (REG_IF & BIT_IE_IPC_FIFO_NOT_EMPTY) {
    // handle IPC FIFO not empty
    if (REG_IPCFIFORECV == 0x87654321)
      Wifi_Sync();

    // end
    IRQ_CHECK |= BIT_IE_IPC_FIFO_NOT_EMPTY;
    REG_IF |= BIT_IE_IPC_FIFO_NOT_EMPTY;
  }

  if (REG_IF & BIT_IE_T0) {
    // one tick for the TTU player
    ttuTick();

    // end
    IRQ_CHECK |= BIT_IE_T0;
    REG_IF |= BIT_IE_T0;
  }

  if (REG_IF & BIT_IE_VBI) {
    // handle vblank

    // startup delay
    if (startupVBICounter < 10)
      startupVBICounter++;
    else {
      if (sharedMemory != NULL) {
        // read the touch screen data
        touchReadPosition(&touch);

        // make the data available for the ARM9
        if (touch.pixelX <= 0 || touch.pixelX >= SCREEN_WIDTH - 1 || touch.pixelY <= 0 || touch.pixelY >= SCREEN_HEIGHT - 1)
          sharedMemory[SHARED_TOUCH_PRESSED] = NO;
        else
          sharedMemory[SHARED_TOUCH_PRESSED] = YES;

        sharedMemory[SHARED_TOUCH_PX] = touch.pixelX;
        sharedMemory[SHARED_TOUCH_PY] = touch.pixelY;

        // manage the sound queue
        soundQueuePoll();
      }
    }

    // read X and Y buttons
    if (sharedMemory != NULL)
      sharedMemory[SHARED_KEYS] = REG_KEYXY;

    // WIFI vbi update
    Wifi_Update();

    // end
    IRQ_CHECK |= BIT_IE_VBI;
    REG_IF |= BIT_IE_VBI;
  }

  // mark the used scanlines
  while (scanlineStart != REG_VCOUNT) {
    freeScanlines[scanlineStart++] = 1;
    if (scanlineStart == 263)
      scanlineStart = 0;
  }
}


// dswifi functions
void wifiARM7ARM9Sync(void) {

  REG_IPCFIFOSEND = 0x87654321;
}


int main(int argc, char **argv) {

  u32 i, j;

  sharedMemory = NULL;

  // IPC init
  REG_IPCFIFOCNT = IPCFIFOCNT_SEND_FLUSH | IPCFIFOCNT_ENABLE;

  // enable sound
  REG_POWCNT = BIT_POWCNT_SOUND;

  // disable timer 0
  REG_TM0CNT_H = 0;

  // audio master
  REG_SOUNDCNT = BIT_SOUNDCNT_SOUND_ENABLE | SOUNDCNT_SOUND_VOLUME(127);

  // init startup delay
  startupVBICounter = 0;

  // set up the interrupt handler
  REG_IME = 0; // disable all interrupts
  IRQ_HANDLER = &irqHandler;
  REG_IE = BIT_IE_VBI | BIT_IE_T0 | BIT_IE_WIFI | BIT_IE_IPC_FIFO_NOT_EMPTY;
  REG_IF = ~0;
  REG_DISPSTAT = BIT_DISPSTAT_VBI;
  REG_IME = 1; // enable the interrupts set in REG_IE

  /////////////////////////////////////////////////////////////////////////////
  // FIRMWARE
  /////////////////////////////////////////////////////////////////////////////

  // DOESN'T SEEM TO WORK???

  // read the user name
  //firmwareRead(0x3FA1A,  1, (u8 *)USER_NAME_CHARACTERS);
  //firmwareRead(0x3FA06, 20, (u8 *)USER_NAME);

  // init the touch screen reader
  touchInit();

  /////////////////////////////////////////////////////////////////////////////
  // WIFI
  /////////////////////////////////////////////////////////////////////////////

  // sync with ARM9 and init WIFI
  while (1) {
    // wait for something to arrive
    while (REG_IPCFIFOCNT & IPCFIFOCNT_RECV_STATUS_EMPTY)
      swiWaitForVBlank();

    // is it the WIFI sync number?
    i = REG_IPCFIFORECV;
    if (i == 0x12345678)
      break;
  }

  // wait for the WIFI init code to arrive
  while (REG_IPCFIFOCNT & IPCFIFOCNT_RECV_STATUS_EMPTY)
    swiWaitForVBlank();

  i = REG_IPCFIFORECV;

  // get also the address of the shared memory block
  while (REG_IPCFIFOCNT & IPCFIFOCNT_RECV_STATUS_EMPTY)
    swiWaitForVBlank();

  sharedMemory = (vu16 *)(REG_IPCFIFORECV);

  // continue to init the WIFI
  Wifi_Init(i);

  // init the IPC FIFO
  REG_IPCFIFOCNT = IPCFIFOCNT_ENABLE | IPCFIFOCNT_RECV_IRQ_ON_NOT_EMPTY;

  // allow WIFI lib to notify ARM9
  Wifi_SetSyncHandler(wifiARM7ARM9Sync);

  /////////////////////////////////////////////////////////////////////////////
  // THE REST
  /////////////////////////////////////////////////////////////////////////////

  // init the sound queue
  soundQueueInit();

  /////////////////////////////////////////////////////////////////////////////
  // MAIN LOOP
  /////////////////////////////////////////////////////////////////////////////

  // ARM7 does all its jobs in the interrupts
  while (1) {
    swiWaitForVBlank();

    // one frame has passed, count the scanlines where we had CPU activity (and clear the buffer)
    j = 0;
    for (i = 0; i < 263; i++) {
      if (freeScanlines[i] == 1) {
        freeScanlines[i] = 0;
        j++;
      }
    }

    // pass it to ARM9
    sharedMemory[SHARED_ARM7_FREE] = 263 - j;
  }

  return 0;
}


