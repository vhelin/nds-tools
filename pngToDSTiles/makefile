
CC = gcc
LD = gcc

CFLAGS = -Wall -c -g -O0 -ansi -pedantic
LDFLAGS = -lz -lpng

CFILES = main.c png.c
HFILES = main.h png.h
OFILES = main.o png.o
EXECUT = pngToDSTiles


all: $(OFILES) makefile
	$(LD) $(LDFLAGS) $(OFILES) -o $(EXECUT) -lm

main.o: main.c defines.h
	$(CC) $(CFLAGS) main.c

png.o: png.c defines.h
	$(CC) $(CFLAGS) png.c


$(OFILES): $(HFILES)


clean:
	rm -f $(OFILES) core *~ $(EXECUT)

nice:
	rm -f *~

install:
	make; cp $(EXECUT) /usr/local/bin/
