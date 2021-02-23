
#include "ndsARM7.h"

#include "defines.h"
#include "memory.h"
#include "bios.h"
#include "firmware.h"



void firmwareRead(u32 address, u32 size, u8 *buffer) {

  s32 i;

  while (REG_SPICNT & BIT_SPICNT_BUSY);

  // write the command and wait for it to complete
  REG_SPICNT = BIT_SPICNT_ENABLE | SPICNT_BAUD_4MHZ | SPICNT_DEVICE_FIRMWARE | BIT_SPICNT_CONTINUOUS;
	REG_SPIDATA = FIRMWARE_READ;

  while (REG_SPICNT & BIT_SPICNT_BUSY);

	// set the address
  REG_SPIDATA =  (address >> 16) & 0xFF;
  while (REG_SPICNT & BIT_SPICNT_BUSY);
  REG_SPIDATA =  (address >> 8) & 0xFF;
  while (REG_SPICNT & BIT_SPICNT_BUSY);
  REG_SPIDATA =  address & 0xFF;
  while (REG_SPICNT & BIT_SPICNT_BUSY);

  for (i = 0; i < size; i++) {
    REG_SPIDATA = 0;
		while (REG_SPICNT & BIT_SPICNT_BUSY);
    buffer[i] = REG_SPIDATA & 0xFF;
  }

	REG_SPICNT = 0;
}
