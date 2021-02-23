
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "main.h"



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

	unsigned char *data;
	int fileSize;
  FILE *f;

  if (argc != 3) {
    fprintf(stderr, "USAGE: %s <IN BIN> <OUT RLE>\n", argv[0]);
    return 1;
  }

	/********************************************************************************/
	/* INPUT */
	/********************************************************************************/

	/* read the BIN file */
  f = fopen(argv[1], "rb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for reading.\n", argv[1]);
    return 1;
  }

  /* get the file size */
  fseek(f, 0, SEEK_END);
  fileSize = ftell(f);
  fseek(f, 0, SEEK_SET);

  data = malloc(fileSize);
  if (data == NULL) {
    fprintf(stderr, "MAIN: Out of memory error.\n");
    fclose(f);
    return 1;
  }

  fread(data, 1, fileSize, f);
  fclose(f);

	/********************************************************************************/
	/* OUTPUT (RLE) */
	/********************************************************************************/

	{
		int i, count, b1;

		/* write the RLE file */
		f = fopen(argv[2], "wb");
		if (f == NULL) {
			fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", argv[2]);
			return 1;
		}

		/* header */
		_write_u8(f, 'R');
		_write_u8(f, 'L');
		_write_u8(f, 'E');
		_write_u8(f, 'b');

		/* original size */
		_write_u32(f, fileSize);

		/* compress */
		i = 0;
		while (i < fileSize) {

#ifdef U16
			/* output the data word */
			_write_u16(f, data[i+0] | (data[i+1] << 8));
			b1 = data[i++];
			b2 = data[i++];
			count = 1;
			while (count < 0xFFFF && i < fileSize) {
				if (b1 != data[i+0] || b2 != data[i+1])
					break;
				i += 2;
				count++;
			}
			/* output the count */
			_write_u16(f, count);
#endif

			/* output the data word */
			_write_u8(f, data[i]);
			b1 = data[i++];
			count = 1;
			while (count < 0xFF && i < fileSize) {
				if (b1 != data[i])
					break;
				i++;
				count++;
			}
			/* output the count */
			_write_u8(f, count);
		}

		fclose(f);
	}

  return 0;
}
