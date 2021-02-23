
#ifndef _MEMORYMANAGER_H
#define _MEMORYMANAGER_H

void  memoryManagerInit(void);
void *memoryManagerMalloc(int size);
void  memoryManagerFree(void *ptr);

#endif
