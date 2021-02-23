
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "main.h"


static const int IMA_ADPCMStepTable[89] = {
     7, 8, 9, 10, 11, 12, 13, 14,
    16, 17, 19, 21, 23, 25, 28, 31,
    34, 37, 41, 45, 50, 55, 60, 66,
    73, 80, 88, 97, 107, 118, 130, 143,
   157, 173, 190, 209, 230, 253, 279, 307,
   337, 371, 408, 449, 494, 544, 598, 658,
   724, 796, 876, 963, 1060, 1166, 1282, 1411,
  1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
  3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
  7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
 32767
};


static const int IMA_ADPCMIndexTable[8] =	{
  -1, -1, -1, -1, 2, 4, 6, 8
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

	int fileSize, inputBits, *tmp, i, predictedValue, delta, stepIndex, value, step, diff, ima, out;
	char *data;
  FILE *f;

	inputBits = 0;
	if (argc > 1) {
		if (strcmp("-8", argv[1]) == 0)
			inputBits = 8;
		else if (strcmp("-16", argv[1]) == 0)
			inputBits = 16;
	}

  if (argc != 4 || inputBits == 0) {
    fprintf(stderr, "USAGE: %s -{8/16} <IN RAW> <OUT IMA-ADPCM>\n", argv[0]);
    return 1;
  }

	/********************************************************************************/
	/* INPUT */
	/********************************************************************************/

	/* read the RAW file */
  f = fopen(argv[2], "rb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for reading.\n", argv[2]);
    return 1;
  }

  /* get the file size */
  fseek(f, 0, SEEK_END);
  fileSize = ftell(f);
  fseek(f, 0, SEEK_SET);

  data = malloc(fileSize);
  if (data == NULL) {
    fprintf(stderr, "MAIN: Out of memory error [1].\n");
    fclose(f);
    return 1;
  }

  fread(data, 1, fileSize, f);
  fclose(f);

	/********************************************************************************/
	/* INPUT CONVERSION */
	/********************************************************************************/

	tmp = malloc(sizeof(int) * fileSize);
	if (tmp == NULL) {
		fprintf(stderr, "MAIN: Out of memory error [2].\n");
		return 1;
	}

	if (inputBits == 8) {
		/* 8 -> 16 */
		for (i = 0; i < fileSize; i++)
			tmp[i] = data[i] << 8;
	}
	else {
		/* 16 -> 16 */
		for (i = 0; i < fileSize; i += 2)
			tmp[i >> 1] = (data[i + 1] << 8) | (data[i] & 0xFF);

		fileSize >>= 1;
	}

	/********************************************************************************/
	/* OUTPUT (IMA-ADPCM) */
	/********************************************************************************/

	/* write the IMA-ADPCM file */
	f = fopen(argv[3], "wb");
	if (f == NULL) {
		fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", argv[3]);
		return 1;
	}

#ifdef WRITE_HEADER
	/* header */
	_write_u8(f, 'I');
	_write_u8(f, 'M');
	_write_u8(f, 'A');
	_write_u8(f, 'a');

	/* original size */
	_write_u32(f, fileSize);
#endif

	/* initial value */
	predictedValue = tmp[0];
	delta = tmp[1] - tmp[0];
	if (delta < 0)
		delta = -delta;
	if (delta > 32767)
		delta = 32767;
	stepIndex = 0;
	while (IMA_ADPCMStepTable[stepIndex] < delta)
		stepIndex++;

	_write_u8(f, predictedValue);
	_write_u8(f, predictedValue >> 8);
	_write_u8(f, stepIndex);
	_write_u8(f, 0);

	i = 2;
	ima = 0;
	out = 0;
	while (i < fileSize) {
		delta = tmp[i++] - predictedValue;
		if (delta >= 0)
			value = 0;
		else {
			value = 8;
			delta = -delta;
		}

		step = IMA_ADPCMStepTable[stepIndex];
		diff = step >> 3;
		if (delta > step) {
			value |= 4;
			delta -= step;
			diff += step;
		}
		step >>= 1;
		if (delta > step) {
			value |= 2;
			delta -= step;
			diff += step;
		}
		step >>= 1;
		if (delta > step) {
			value |= 1;
			diff += step;
		}

		if (value & 8)
			predictedValue -= diff;
		else
			predictedValue += diff;
		if (predictedValue < -0x8000)
			predictedValue = -0x8000;
		else if (predictedValue > 0x7fff)
			predictedValue = 0x7fff;

		stepIndex += IMA_ADPCMIndexTable[value & 7];
		if (stepIndex < 0)
			stepIndex = 0;
		else if (stepIndex > 88)
			stepIndex = 88;

		if (out == 0) {
			out++;
			ima = value;
		}
		else {
			out = 0;
			_write_u8(f, ima | (value << 4));
		}
	}

	/* is the last nibble single? */
	if (out == 1)
		_write_u8(f, ima);

	fclose(f);

  return 0;
}
