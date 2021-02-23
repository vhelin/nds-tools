
#ifndef _MAIN_H
#define _MAIN_H

struct sample {
	char name[23];
	int length;
	int finetune;
	int volume;
	int loopStart;
	int loopLength;
	int realID;
	int filePointer;
	unsigned char *data;
};

#endif
