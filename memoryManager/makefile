
CC = gcc
LD = gcc

CFLAGS = -Wall -c -g -O0 -ansi -pedantic -std=c99 
LDFLAGS = 

CFILES = main.c memoryManager.c
HFILES = main.h memoryManager.h
OFILES = main.o memoryManager.o
EXECUT = memoryManager


all: $(OFILES) makefile
	$(LD) $(LDFLAGS) $(OFILES) -o $(EXECUT) -lm

main.o: main.c
	$(CC) $(CFLAGS) main.c

memoryManager.o: memoryManager.c
	$(CC) $(CFLAGS) memoryManager.c


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ $(EXECUT)

nice:
	rm -f *~
