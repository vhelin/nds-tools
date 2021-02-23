
#include "ndsARM7.h"

#include "defines.h"
#include "memory.h"
#include "bios.h"

#include "personalData.h"
#include "touch.h"
#include "firmware.h"


// private variables
static s32 touchXScale;
static s32 touchYScale;
static s32 touchXOffset;
static s32 touchYOffset;


void touchInit(void) {

	// these values are not in the firmware?
  touchXScale = ((PERSONAL_DATA_TOUCH_CALIBRATION_X2PX - PERSONAL_DATA_TOUCH_CALIBRATION_X1PX) << 19) /
    (PERSONAL_DATA_TOUCH_CALIBRATION_X2 - PERSONAL_DATA_TOUCH_CALIBRATION_X1);
  touchYScale = ((PERSONAL_DATA_TOUCH_CALIBRATION_Y2PX - PERSONAL_DATA_TOUCH_CALIBRATION_Y1PX) << 19) /
    (PERSONAL_DATA_TOUCH_CALIBRATION_Y2 - PERSONAL_DATA_TOUCH_CALIBRATION_Y1);
  touchXOffset = ((PERSONAL_DATA_TOUCH_CALIBRATION_X1 + PERSONAL_DATA_TOUCH_CALIBRATION_X2) * touchXScale -
		  ((PERSONAL_DATA_TOUCH_CALIBRATION_X1PX + PERSONAL_DATA_TOUCH_CALIBRATION_X2PX) << 19)) >> 1;
  touchYOffset = ((PERSONAL_DATA_TOUCH_CALIBRATION_Y1 + PERSONAL_DATA_TOUCH_CALIBRATION_Y2) * touchYScale -
		  ((PERSONAL_DATA_TOUCH_CALIBRATION_Y1PX + PERSONAL_DATA_TOUCH_CALIBRATION_Y2PX) << 19)) >> 1;
}


u16 touchReadValue(u32 command) {

  u16 result;

  // wait as long as the serial is busy
  while (REG_SPICNT & BIT_SPICNT_BUSY)
    swiDelay(1);

  // write the command and wait for it to complete
  REG_SPICNT = BIT_SPICNT_ENABLE | SPICNT_BAUD_2MHZ | SPICNT_DEVICE_TOUCH | BIT_SPICNT_CONTINUOUS;
  REG_SPIDATA = command;

  // wait as long as the serial is busy
  while (REG_SPICNT & BIT_SPICNT_BUSY)
    swiDelay(1);

  // write the second command and read the first half of the data
  REG_SPIDATA = 0;

  // wait as long as the serial is busy
  while (REG_SPICNT & BIT_SPICNT_BUSY)
    swiDelay(1);

  result = REG_SPIDATA;

  // read the rest of the data
  REG_SPICNT = BIT_SPICNT_ENABLE | 0x201;
  REG_SPIDATA = 0;

  // wait as long as the serial is busy
  while (REG_SPICNT & BIT_SPICNT_BUSY)
    swiDelay(1);

  return ((result & 0x7F) << 5) | (REG_SPIDATA >> 3);
}


s32 touchReadRetry(s32 measure, s32 retry, s32 range) {

  s32 i, currentValue, currentRange;
  s32 previousValue;

	previousValue = touchReadValue(measure | 1);

  /* re-read the data until we get a stabile answer */
  for (i = 0; i < retry; i++) {
    currentValue = touchReadValue(measure | 1);

    currentRange = previousValue - currentValue;
    if (currentRange < 0)
      currentRange = -currentRange;

    if (currentRange <= range)
      return currentValue;
  }

  return 0;
}


void touchReadPosition(struct touch *touch) {

  s16 pX, pY;

  touch->touchX = touchReadRetry(TSC_MEASURE_X, 7, 30);
  touch->touchY = touchReadRetry(TSC_MEASURE_Y, 7, 30);

  pX = (touch->touchX * touchXScale - touchXOffset + (touchXScale>>1)) >> 19;
  pY = (touch->touchY * touchYScale - touchYOffset + (touchYScale>>1)) >> 19;

  if (pX < 0)
    pX = 0;
  if (pY < 0)
    pY = 0;
  if (pX > SCREEN_WIDTH - 1)
    pX = SCREEN_WIDTH - 1;
  if (pY > SCREEN_HEIGHT - 1)
    pY = SCREEN_HEIGHT - 1;

  touch->pixelX = pX;
  touch->pixelY = pY;
}
