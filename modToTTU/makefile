
CC = gcc
LD = gcc

CFLAGS = -Wall -c -g -O0 -ansi -pedantic
LDFLAGS = 

CFILES = main.c
HFILES = main.h
OFILES = main.o
EXECUT = modToTTU


all: $(OFILES) makefile
	$(LD) $(LDFLAGS) $(OFILES) -o $(EXECUT) -lm

main.o: main.c defines.h
	$(CC) $(CFLAGS) main.c


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ $(EXECUT)

nice:
	rm -f *~

install:
	make; cp $(EXECUT) /usr/local/bin/
