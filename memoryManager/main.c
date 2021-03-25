
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "memoryManager.h"



int main(int argc, char *argv[]) {

  unsigned char *m1, *m2;
  int i;

  memoryManagerInit();

  memoryManagerPrint(64);

  m1 = memoryManagerMalloc(15);
  if (m1 == NULL) {
    fprintf(stderr, "MAIN: Out of memory error (1).\n");
    return 0;
  }
  for (i = 0; i < 15; i++)
    m1[i] = i;

  memoryManagerPrint(64);

  m2 = memoryManagerMalloc(7);
  if (m2 == NULL) {
    fprintf(stderr, "MAIN: Out of memory error (2).\n");
    return 0;
  }
  for (i = 0; i < 7; i++)
    m2[i] = 0xAA;

  memoryManagerPrint(64);

  memoryManagerFree(m1);

  memoryManagerPrint(64);

  m1 = memoryManagerMalloc(7);
  if (m1 == NULL) {
    fprintf(stderr, "MAIN: Out of memory error (3).\n");
    return 0;
  }
  for (i = 0; i < 7; i++)
    m1[i] = 0xCC;

  memoryManagerPrint(64);

  memoryManagerFree(m1);
  memoryManagerPrint(64);
  memoryManagerFree(m2);
  memoryManagerPrint(64);

  m1 = memoryManagerMalloc(64-8);
  if (m1 == NULL) {
    fprintf(stderr, "MAIN: Out of memory error (4).\n");
    return 0;
  }
  for (i = 0; i < 64-8; i++)
    m1[i] = 0x00;

  memoryManagerPrint(64);

  return 0;
}
