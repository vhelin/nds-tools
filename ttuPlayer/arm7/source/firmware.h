
#ifndef _FIRMWARE_H
#define _FIRMWARE_H

// firmware commands
#define FIRMWARE_READ_ID     0x9F
#define FIRMWARE_READ        0x03
#define FIRMWARE_READ_STATUS 0x05

void firmwareRead(u32 address, u32 size, u8 *buffer);

#endif
