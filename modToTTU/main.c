
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "main.h"


/* the number of channels */
int channelsN;

/* the samples */
struct sample samples[31];
int samplesN;

/* the orders */
unsigned char orders[128];
int ordersN;

/* the number of patterns */
int patternsN;

/* the pattern data */
unsigned char *patterns;

/* octave tables (for amiga frequency -> note conversion) */
int periodTable[12*5] = {
	1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 906, /* C-0 to B-0 */
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,           /* C-1 to B-1 */
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,           /* C-2 to B-2 */
	214, 202, 190, 180, 169, 160, 151, 142, 134, 127, 120, 113,           /* C-3 to B-3 */
	107, 101, 95, 90, 84, 80, 75, 71, 67, 63, 60, 56                      /* C-4 to B-4 */
};


static void _write_u32(FILE *f, int data) {

  fprintf(f, "%c%c%c%c", data & 0xFF, (data >> 8) & 0xFF, (data >> 16) & 0xFF, (data >> 24) & 0xFF);
}


/*
static void _write_u16(FILE *f, int data) {

  fprintf(f, "%c%c", data & 0xFF, (data >> 8) & 0xFF);
}
*/


static void _write_u8(FILE *f, int data) {

  fprintf(f, "%c", data & 0xFF);
}


int main(int argc, char *argv[]) {

	unsigned char *mod;
	int fileSize, p;
  FILE *f;

#ifdef PRECALCULATE
	{
		/* precalculate finetunes */
		const int periodTable[16][12] =  {
			{ 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453 },
			{ 850, 802, 757, 715, 674, 637, 601, 567, 535, 505, 477, 450 },
			{ 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474, 447 },
			{ 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470, 444 },
			{ 832, 785, 741, 699, 660, 623, 588, 555, 524, 495, 467, 441 },
			{ 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463, 437 },
			{ 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460, 434 },
			{ 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457, 431 },
			{ 907, 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480 },
			{ 900, 850, 802, 757, 715, 675, 636, 601, 567, 535, 505, 477 },
			{ 894, 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474 },
			{ 887, 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470 },
			{ 881, 832, 785, 741, 699, 660, 623, 588, 555, 524, 494, 467 },
			{ 875, 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463 },
			{ 868, 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460 },
			{ 862, 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457 },
		};

		int finetune, note, octave;
		int freqTable[16*12*5];

    #define AMIGA_VAL 3579545

		for (finetune = 0; finetune < 16; finetune++) {
			for (octave = 0; octave < 5; octave++)	{
				for (note = 0; note < 12; note++) {
					int tempPeriod = (periodTable[finetune][note]*2) >> octave;
					int tempFreq = (AMIGA_VAL / tempPeriod);

					if(tempFreq > 65535)
						tempFreq = 65535;

					freqTable[finetune*12*5 + octave*12 + note] = tempFreq;
					/*
					fprintf(stderr, " %d,", tempFreq);
					*/
					fprintf(stderr, " %d,", tempPeriod);
				}
				fprintf(stderr, "\n");
			}
		}
	}
#endif

  if (argc != 4) {
    fprintf(stderr, "USAGE: %s <IN MOD> <OUT TTU> <OUT TTS>\n", argv[0]);
    return 1;
  }

	/********************************************************************************/
	/* INPUT */
	/********************************************************************************/

	/* read the MOD file */
  f = fopen(argv[1], "rb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for reading.\n", argv[1]);
    return 1;
  }

  /* get the file size */
  fseek(f, 0, SEEK_END);
  fileSize = ftell(f);
  fseek(f, 0, SEEK_SET);

  mod = malloc(fileSize);
  if (mod == NULL) {
    fprintf(stderr, "MAIN: Out of memory error.\n");
    fclose(f);
    return 1;
  }

  fread(mod, 1, fileSize, f);
  fclose(f);

	/********************************************************************************/
	/* FORMAT CHECK */
	/********************************************************************************/

	/* count channels (and check the format as well) */
	{
		char format[5], name[21];
		int i;

		/* format */
		format[0] = mod[1080];
		format[1] = mod[1081];
		format[2] = mod[1082];
		format[3] = mod[1083];
		format[4] = 0;

		if (strcmp("M.K.", format) == 0 || strcmp("4CHN", format) == 0)
			channelsN = 4;
		else if (strcmp("6CHN", format) == 0)
			channelsN = 6;
		else if (strcmp("8CHN", format) == 0)
			channelsN = 8;
		else {
			/* unknown format! */
			fprintf(stderr, "MAIN: Could not parse the module.\n");
			return 0;
		}

		/* name */
		for (i = 0; i < 20; i++)
			name[i] = mod[i];
		name[i] = 0;

		fprintf(stderr, "MAIN: \"%s\" of \"%s\" (%d channels).\n", name, format, channelsN);
	}

	/********************************************************************************/
	/* SAMPLES */
	/********************************************************************************/

	/* load the samples */
	{
		int i, j;

		/* pointer to the sample data */
		p = 20;

		for (i = 0; i < 31; i++) {
			/* name */
			for (j = 0; j < 22; j++) {
				samples[i].name[j] = mod[p++];
				if (samples[i].name[j] == 0)
					samples[i].name[j] = ' ';
			}
			samples[i].name[j] = 0;

			/* length */
			samples[i].length = ((mod[p+0] << 8) | mod[p+1]) << 1;
			p += 2;

			/* finetune */
			samples[i].finetune = mod[p++] & 0xF;

			/* volume */
			samples[i].volume = mod[p++];

			/* loop start */
			samples[i].loopStart = ((mod[p+0] << 8) | mod[p+1]) << 1;
			p += 2;

			/* loop length */
			samples[i].loopLength = ((mod[p+0] << 8) | mod[p+1]) << 1;
			p += 2;

			samples[i].data = NULL;

			fprintf(stderr, "MAIN: Sample %.2d: \"%s\" - size~%.6dBs - looplength~%.6dB - finetune~%d\n", i+1, samples[i].name, samples[i].length, samples[i].loopLength, samples[i].finetune);

			/* fix the loop length */
			if (samples[i].loopLength <= 4)
				samples[i].loopLength = 0;
		}
	}

	/********************************************************************************/
	/* ORDERS */
	/********************************************************************************/

	{
		int j;

		/* read the number of orders */
		ordersN = mod[p++];

		/* skip the zero byte */
		p++;

		fprintf(stderr, "MAIN: Pattern order:");

		/* read the orders */
		patternsN = 0;
		for (j = 0; j < 128; j++) {
			orders[j] = mod[p++];

			/* find the highest referenced pattern */
			if (patternsN < orders[j])
				patternsN = orders[j];

			if (j < ordersN)
				fprintf(stderr, " %d", orders[j]);
		}

		fprintf(stderr, ".\n");
		fprintf(stderr, "MAIN: The highest referenced pattern = %d.\n", patternsN);

		/* fix the value (to the number of patterns) */
		patternsN++;
	}

	/********************************************************************************/
	/* PATTERNS */
	/********************************************************************************/

	{
		int j;

		/* skip the header */
		p += 4;

		/* allocate the pattern data table */
		patterns = malloc(sizeof(unsigned char) * (channelsN * 4 * 64 * patternsN));
		if (patterns == NULL) {
			fprintf(stderr, "MAIN: Out of memory error.\n");
			return 1;
		}

		/* read the data */
		for (j = 0; j < channelsN * 4 * 64 * patternsN; j++)
			patterns[j] = mod[p++];

		fprintf(stderr, "MAIN: Read %dBs worth of pattern data.\n", channelsN * 4 * 64 * patternsN);
	}

	/********************************************************************************/
	/* SAMPLES */
	/********************************************************************************/

	{
		unsigned char tmp[4];
		int j, k;

		for (j = 0; j < 31; j++) {
			/* skip empty samples */
			if (samples[j].length <= 0)
				continue;

			samples[j].data = calloc(sizeof(unsigned char) * samples[j].length + 4, 1);
			if (samples[j].data == NULL) {
				fprintf(stderr, "MAIN: Out of memory error.\n");
				return 1;
			}

			for (k = 0; k < samples[j].length; k++)
				samples[j].data[k] = mod[p++];

			/* change endianess */
			for (k = 0; k < (samples[j].length >> 2) + 1; k++) {
				tmp[0] = samples[j].data[k*4 + 0];
				tmp[1] = samples[j].data[k*4 + 1];
				tmp[2] = samples[j].data[k*4 + 2];
				tmp[3] = samples[j].data[k*4 + 3];
				samples[j].data[k*4 + 0] = tmp[3];
				samples[j].data[k*4 + 1] = tmp[2];
				samples[j].data[k*4 + 2] = tmp[1];
				samples[j].data[k*4 + 3] = tmp[0];
			}
		}
	}

	if (p != fileSize)
		fprintf(stderr, "MAIN: Parsed %dBs, the file size is %dBs.\n", p, fileSize);

	/********************************************************************************/
	/* REMOVE UNUSED SAMPLES */
	/********************************************************************************/

	{
		int i, realID;

		realID = 0;
		for (i = 0; i < 31; i++) {
			if (samples[i].length <= 0)
				samples[i].realID = -1;
			else
				samples[i].realID = realID++;
		}

		samplesN = realID;
	}

	/********************************************************************************/
	/* OUTPUT (TTU) */
	/********************************************************************************/

	{
		int i, j, sample, period, effect, param;

		/* write the TTU file */
		f = fopen(argv[2], "wb");
		if (f == NULL) {
			fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", argv[2]);
			return 1;
		}

		/* header */
		_write_u8(f, 'T');
		_write_u8(f, 'T');
		_write_u8(f, 'U');
		_write_u8(f, '1');

		/* channels */
		_write_u32(f, channelsN);

		/* patterns */
		_write_u32(f, patternsN);

		/* orders */
		_write_u32(f, ordersN);

		for (i = 0; i < ordersN; i++)
			_write_u32(f, orders[i]);

		{
			int closestNote, closestDistance, distance, k;

			for (i = 0; i < channelsN * 64 * patternsN; i++) {
				/* parse the entry */
				j = i << 2;
				sample = (patterns[j+0] & 0xF0) | (patterns[j+2] >> 4);
				period = patterns[j+1] | ((patterns[j+0] & 0xF) << 8);
				effect = patterns[j+2] & 0xF;
				param  = patterns[j+3];

				/* convert the sample ID (0xFF ~ no sample) */
				sample--;
				if (sample >= 0)
					sample = samples[sample].realID;
				sample &= 0xFF;

				/* replace the amiga period value with a note index */
				closestNote = 0;
				closestDistance = 0xffff;

				for (k = 0; k < 12*5; k++) { /* 5 octaves, 12 notes each */
					distance = abs(period - periodTable[k]);

					if (distance < closestDistance) {
						closestNote = k;
						closestDistance = distance;
					}
				}

				/* treat "no notes" differently */
				if (period == 0)
					closestNote = 0xFF;

				/* output the parameters */
				_write_u8(f, closestNote); /* note */
				_write_u8(f, sample);      /* sample */
				_write_u8(f, effect);      /* effect */
				_write_u8(f, param);       /* parameter */
			}
		}

		fclose(f);
	}

	/********************************************************************************/
	/* OUTPUT (SAMPLES) */
	/********************************************************************************/

	{
		int i, j, p = 0;

		/* write the TTS file */
		f = fopen(argv[3], "wb");
		if (f == NULL) {
			fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", argv[3]);
			return 1;
		}

		/* header */
		_write_u8(f, 'T');
		_write_u8(f, 'T');
		_write_u8(f, 'S');
		_write_u8(f, '1');
		p++;

		/* samples */
		_write_u32(f, samplesN);
		p++;

		/* skip the pointer table now, we fill it up later */
		for (i = 0; i < samplesN; i++) {
			_write_u32(f, 0);
			p++;
		}

		for (i = 0; i < 31; i++) {
			/* skip dead samples */
			if (samples[i].length <= 0)
				continue;

			/* store the pointer */
			samples[i].filePointer = p;

			/* fix the volume (0-64 -> 0-126) */
			if (samples[i].volume > 0x3F)
				samples[i].volume = 0x3F;
			samples[i].volume *= 2;

			/* parameters */
			_write_u32(f, samples[i].length / 4);
			_write_u32(f, samples[i].finetune);
			_write_u32(f, samples[i].volume);
			_write_u32(f, samples[i].loopStart / 4);
			_write_u32(f, samples[i].loopLength / 4);
			p += 5;

			/* sample data */
			for (j = 0; j < samples[i].length; j++)
				_write_u8(f, samples[i].data[j]);
			p += samples[i].length / 4;

			/* pad the sample with zeroes, to align the data by four bytes */
			j = samples[i].length & 3;
			if (j > 0) {
				j = 4 - j;
				while (j > 0) {
					_write_u8(f, 0);
					j--;
				}
				p++;
			}
		}

		/* write the pointers */
		fseek(f, 8, SEEK_SET);

		for (i = 0; i < 31; i++) {
			/* skip dead samples */
			if (samples[i].length <= 0)
				continue;

			_write_u32(f, samples[i].filePointer);
		}

		fclose(f);
	}

  return 0;
}
