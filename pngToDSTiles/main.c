
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>

#include "defines.h"
#include "png.h"


/* the palette */
int palette[256];
int paletteEntries = 0;


static void _write_u16(FILE *f, int data) {

  fprintf(f, "%c%c", data & 0xFF, (data >> 8) & 0xFF);
}


static void _write_u8(FILE *f, int data) {

  fprintf(f, "%c", data & 0xFF);
}


static int _palette_collect_smash(unsigned char *data, int dX, int dY, int entries) {

  int i, j, color;

  for (i = 0; i < dX*dY; i++) {
    color = (data[i*2 + 0] << 8) | data[i*2 + 1];

    /* do we have the color already? */
    for (j = 0; j < paletteEntries; j++) {
      if (palette[j] == color)
				break;
    }

    /* store the palette index */
    data[i] = j;

    if (j < paletteEntries)
      continue;

    if (paletteEntries == entries) {
      fprintf(stderr, "_PALETTE_COLLECT: The image has more than %d colors!\n", entries);
      return FAILED;
    }

    /* nope -> add the new color */
    palette[paletteEntries++] = color;
  }

  fprintf(stderr, "_PALETTE_COLLECT_SMASH: The image has %d unique colors.\n", paletteEntries);

  return SUCCEEDED;
}


static void _image_squash(unsigned char *data, int dX, int dY, int type) {

  int colors, cR, cG, cB, i, color;

  if (type == GL_RGB)
    colors = 3;
  else
    colors = 4;

  for (i = 0; i < dX*dY; i++) {
    cR = data[i*colors + 0] >> 3;
    cG = data[i*colors + 1] >> 3;
    cB = data[i*colors + 2] >> 3;

    color = (cB << 10) | (cG << 5) | cR;

		if (colors == 4 && data[i*colors + 3] == 0)
			color = 0xFFFF;

    /* write back the color */
    data[i*2 + 0] = color >> 8;
    data[i*2 + 1] = color & 0xFF;
  }
}


int main(int argc, char *argv[]) {

  int dX, dY, type, i, x, y, tX, tY, colors, k, tileX, tileY, bX, bY;
  unsigned char *d;
  char name[256];
  FILE *f;

  if (argc != 5 || (strcmp(argv[1], "-1") != 0 && strcmp(argv[1], "-2") != 0) ||
			(strcmp(argv[2], "-8x8") != 0 && strcmp(argv[2], "-16x16") != 0)) {
    fprintf(stderr, "USAGE: %s -{12} -{8x8/16x16} <PNG FILE> <OUT BASE NAME>\n", argv[0]);
    fprintf(stderr, "COMMANDS:\n");
    fprintf(stderr, " 1  Output 16 color 8x8 tiles\n");
    fprintf(stderr, " 2  Output 256 color 8x8 tiles\n");
    return -1;
  }

	/* determine colors */
  if (strcmp(argv[1], "-1") == 0)
    colors = 16;
  else
    colors = 256;

	/* determine tile size */
	if (strcmp(argv[2], "-8x8") == 0) {
		tileX = 8;
		tileY = 8;
	}
	else if (strcmp(argv[2], "-16x16") == 0) {
		tileX = 16;
		tileY = 16;
	}

	/* import the image */
  if (png_load(argv[3], &dX, &dY, &type, &d) == FAILED)
    return -1;

  if ((dX % tileX) != 0 || (dY % tileY) != 0) {
    fprintf(stderr, "MAIN: The picture DX must be a multiple of %d, and the picture DY must be a multiple of %d.\n", tileX, tileY);
    return -1;
  }

	/* set color 0 (transparent) */
	palette[paletteEntries++] = 0xFFFF;

  /* squash the image -> 16bit */
  _image_squash(d, dX, dY, type);

  /* collect the palette, and make the data palette indexed */
  if (_palette_collect_smash(d, dX, dY, colors) == FAILED)
    return -1;

  /* write the image data */
  sprintf(name, "%s.bin", argv[4]);
  f = fopen(name, "wb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", name);
    return -1;
  }

	if (colors == 16) {
    /* tile loop */
    for (tY = 0; tY < dY / tileY; tY++) {
      for (tX = 0; tX < dX / tileX; tX++) {
				/* 8x8 loop */
				for (bY = 0; bY < tileY/8; bY++) {
					for (bX = 0; bX < tileX/8; bX++) {
						/* pixel loop */
						for (y = 0; y < 8; y++) {
							for (x = 0; x < 8; x += 2) {
								/* merge the two 4bit indices */
								i = (tY*tileY + bY*8 + y)*dX + (tX*tileX + bX*8 + x + 0);
								k = d[i] << 4;
								i = (tY*tileY + bY*8 + y)*dX + (tX*tileX + bX*8 + x + 1);
								k |= d[i];
								_write_u8(f, k);
							}
						}
					}
				}
			}
		}
  }
	else if (colors == 256) {
    /* tile loop */
    for (tY = 0; tY < dY / tileY; tY++) {
      for (tX = 0; tX < dX / tileX; tX++) {
				/* 8x8 loop */
				for (bY = 0; bY < tileY/8; bY++) {
					for (bX = 0; bX < tileX/8; bX++) {
						/* pixel loop */
						for (y = 0; y < 8; y++) {
							for (x = 0; x < 8; x++) {
								i = (tY*tileY + bY*8 + y)*dX + (tX*tileX + bX*8 + x);
								_write_u8(f, d[i]);
							}
						}
					}
				}
			}
		}
	}

  fclose(f);
  free(d);

  /* write the palette data */
  sprintf(name, "%s.pal", argv[4]);
  f = fopen(name, "wb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", name);
    return -1;
  }

	fprintf(stderr, "COL  A R  G  B\n");

  for (i = 0; i < paletteEntries; i++) {
		fprintf(stderr, "%.3d: %.1d %.2d %.2d %.2d\n", i, palette[i] >> 15, (palette[i] >> 0) & 31, (palette[i] >> 5) & 31, (palette[i] >> 10) & 31);
    _write_u16(f, palette[i]);
	}

  fclose(f);

  return 0;
}
