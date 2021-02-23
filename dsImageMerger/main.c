
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"


/* the merged palette */
int palette[256];
int paletteEntries = 0;


static void _write_u16(FILE *f, int data) {

  fprintf(f, "%c%c", data & 0xFF, (data >> 8) & 0xFF);
}


static void _write_u8(FILE *f, int data) {

  fprintf(f, "%c", data & 0xFF);
}


static int _import_image(char *baseName, int colors, unsigned char **dataOut, int *sizeOut, int importData) {

  int fileSizePixels, fileSizePalette, c, i, j, color;
  unsigned char *data, *pal;
  char name[256];
  FILE *f;

  sprintf(name, "%s.pal", baseName);
  f = fopen(name, "rb");
  if (f == NULL) {
    fprintf(stderr, "_IMPORT_IMAGE: Could not open file \"%s\" for reading.\n", name);
    return FAILED;
  }

  /* get the file size */
  fseek(f, 0, SEEK_END);
  fileSizePalette = ftell(f);
  fseek(f, 0, SEEK_SET);

  pal = malloc(fileSizePalette);
  if (pal == NULL) {
    fprintf(stderr, "_IMPORT_IMAGE: Out of memory error.\n");
    fclose(f);
    return FAILED;
  }

  fread(pal, 1, fileSizePalette, f);
  fclose(f);

  /* count the colors */
  c = fileSizePalette / 2;

  fprintf(stderr, "_IMPORT_IMAGE: File \"%s\" has %d colors.\n", name, c);

  /* add the colors */
  for (i = 0; i < c; i++) {
    color = pal[i*2 + 0] | (pal[i*2 + 1] << 8);
    for (j = 0; j < paletteEntries; j++) {
      if (color == palette[j])
				break;
    }

    if (j == paletteEntries) {
      /* add the new color */
      palette[paletteEntries++] = color;
    }

    if (paletteEntries > colors) {
      fprintf(stderr, "_IMPORT_IMAGE: Out of unique colors!\n");
      return FAILED;
    }
  }

	if (importData == YES) {
		sprintf(name, "%s.bin", baseName);
		f = fopen(name, "rb");
		if (f == NULL) {
			fprintf(stderr, "_IMPORT_IMAGE: Could not open file \"%s\" for reading.\n", name);
			return FAILED;
		}

		/* get the file size */
		fseek(f, 0, SEEK_END);
		fileSizePixels = ftell(f);
		fseek(f, 0, SEEK_SET);

		data = malloc(fileSizePixels);
		if (data == NULL) {
			fprintf(stderr, "_IMPORT_IMAGE: Out of memory error.\n");
			fclose(f);
			return FAILED;
		}

		fread(data, 1, fileSizePixels, f);
		fclose(f);

		/* reindex the data */
		if (colors == 256) {
			for (i = 0; i < fileSizePixels; i++) {
				/* read the old color */
				color = data[i];
				color = pal[color*2 + 0] | (pal[color*2 + 1] << 8);

				/* find the new color */
				for (j = 0; j < paletteEntries; j++) {
					if (color == palette[j])
						break;
				}

				if (j == paletteEntries)
					fprintf(stderr, "_IMPORT_IMAGE: Where did we lose color %d?\n", data[i]);

				/* replace */
				data[i] = j;
			}
		}
		else {
			for (i = 0; i < fileSizePixels; i++) {
				/* read the old color */
				color = (data[i] >> 4) & 0xF;
				color = pal[color*2 + 0] | (pal[color*2 + 1] << 8);

				/* find the new color */
				for (j = 0; j < paletteEntries; j++) {
					if (color == palette[j])
						break;
				}

				if (j == paletteEntries)
					fprintf(stderr, "_IMPORT_IMAGE: Where did we lose color %d?\n", (data[i] >> 4) & 0xF);

				color = data[i] & 0xF;

				/* replace */
				data[i] = j << 4;

				/* read the old color */
				color = pal[color*2 + 0] | (pal[color*2 + 1] << 8);

				/* find the new color */
				for (j = 0; j < paletteEntries; j++) {
					if (color == palette[j])
						break;
				}

				if (j == paletteEntries)
					fprintf(stderr, "_IMPORT_IMAGE: Where did we lose color %d?\n", data[i] & 0xF);

				/* replace */
				data[i] = data[i] | j;
			}
		}
	}

  free(pal);

  *dataOut = data;
	*sizeOut = fileSizePixels;

  return SUCCEEDED;
}


static int _write_image(char *baseName, unsigned char *tileData, int size, int colors) {

  char name[256];
  FILE *f;
  int i;

  sprintf(name, "%s.bin", baseName);
  f = fopen(name, "wb");
  if (f == NULL) {
    fprintf(stderr, "_WRITE_IMAGE: Could not open file \"%s\" for writing.\n", name);
    return FAILED;
  }

	for (i = 0; i < size; i++)
		_write_u8(f, tileData[i]);

  fclose(f);

  return SUCCEEDED;
}


int main(int argc, char *argv[]) {

  int colors, i1Size, i2Size, i;
  unsigned char *i1, *i2;
  char name[256];
  FILE *f;

  if (argc != 4 || (strcmp(argv[1], "-1") != 0 && strcmp(argv[1], "-2") != 0)) {
    fprintf(stderr, "USAGE: %s -{12} <BASE NAME 1> <BASE NAME 2>\n", argv[0]);
    fprintf(stderr, "COMMANDS:\n");
    fprintf(stderr, " 1  Merge 16 color palettes\n");
    fprintf(stderr, " 2  Merge 256 color palettes\n");
    return 1;
  }

  if (strcmp(argv[1], "-1") == 0)
    colors = 16;
  else
    colors = 256;

  /* import the images */
  if (_import_image(argv[2], colors, &i1, &i1Size, NO) == FAILED)
    return 1;
  if (_import_image(argv[3], colors, &i2, &i2Size, YES) == FAILED)
    return 1;

  fprintf(stderr, "MAIN: The combined palette has %d unique colors.\n", paletteEntries);

  /* export the second image */
  if (_write_image(argv[3], i2, i2Size, colors) == FAILED)
    return 1;

  /* write the palette data (both images have now the same palette) */
  sprintf(name, "%s.pal", argv[2]);
  f = fopen(name, "wb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", name);
    return 1;
  }

	fprintf(stderr, "COL  A R  G  B\n");

  for (i = 0; i < paletteEntries; i++) {
		fprintf(stderr, "%.3d: %.1d %.2d %.2d %.2d\n", i, palette[i] >> 15, (palette[i] >> 0) & 31, (palette[i] >> 5) & 31, (palette[i] >> 10) & 31);
    _write_u16(f, palette[i]);
	}

  fclose(f);

  sprintf(name, "%s.pal", argv[3]);
  f = fopen(name, "wb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", name);
    return 1;
  }

  for (i = 0; i < paletteEntries; i++)
    _write_u16(f, palette[i]);

  fclose(f);

  return 0;
}
