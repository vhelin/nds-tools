
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// the amount of memory our manager can manage (number of u32s)
#define MEMORY_MANAGER_HEAP_SIZE (64/4)

// the memory
unsigned int memoryManagerMemoryMap[MEMORY_MANAGER_HEAP_SIZE];


void memoryManagerInit(void) {

  unsigned char *m = (unsigned char *)&memoryManagerMemoryMap[0];
  int i;

  // reset the memory
  for (i = 0; i < MEMORY_MANAGER_HEAP_SIZE*4; i++)
    m[i] = 0xFF;

  // set the initial block
  memoryManagerMemoryMap[0] = 64/8;   // the size of the empty block plus one
  memoryManagerMemoryMap[1] = 0;      // the index of the next empty block
}


void memoryManagerPrint(int bytes) {

  unsigned char *m = (unsigned char *)&memoryManagerMemoryMap[0];
  int i;

  for (i = 0; i < bytes; i++) {
    fprintf(stderr, "%.2x", m[i]);
    if ((i & 3) == 3)
      fprintf(stderr, " ");
  }
  fprintf(stderr, "\n");
}


void *memoryManagerMalloc(int size) {

  unsigned int *m = memoryManagerMemoryMap;
  int i, oldSize, oldPointer;
  void *ptr;

  if (size <= 0)
    return 0;

  // align the size by 8 bytes
  i = size & 7;
  if (i > 0)
    size += 8 - i;

  // make the size 8 byte blocks
  size >>= 3;

  // add the header (two u32s)
  size += 1;

  // find the first fitting block
  i = 0;
  while (i < MEMORY_MANAGER_HEAP_SIZE && m[i+0] < size && m[i+1] > 0)
    i += m[i+1] << 1;

  // out of memory?
  if (i >= MEMORY_MANAGER_HEAP_SIZE || m[i+0] < size)
    return 0;

  // nope -> allocate
  oldSize = m[i+0];
  oldPointer = m[i+1];
  ptr = &m[i+2];

  m[i+0] = 0;
  m[i+1] = size;

  // calculate the new free block
  i += size << 1;
  size = oldSize - size;
  if (size > 0) {
    // the new free block size > 0! mark it
    m[i+0] = size;
    m[i+1] = size;
  }

  return ptr;
}


void memoryManagerFree(void *ptr) {

  unsigned int *m = memoryManagerMemoryMap;
  unsigned int *block;
  int i, j;

  if (ptr == 0)
    return;

  // TODO: a range check?

  // free the given block
  block = (unsigned int *)ptr;
  block[-2] = block[-1];

  // run a free block merge (TODO: don't do this every free() call?)
  i = 0;
  while (i < MEMORY_MANAGER_HEAP_SIZE) {
    if (m[i] == 0) {
      // skip allocated blocks
      i += m[i+1] << 1;
    }
    else {
      // this block is free! is the next one, as well?
      j = i + (m[i+1] << 1);
      if (j >= MEMORY_MANAGER_HEAP_SIZE)
        break;

      if (m[j] == 0) {
        // nope, skip this one
        i = j;
      }
      else {
        // yes! merge these two!
        m[i] += m[j];
        m[i+1] = m[i];
      }
    }
  }
}
