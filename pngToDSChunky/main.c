
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

  int dX, dY, type, i;
  unsigned char *d;
  char name[256];
  FILE *f;

  if (argc != 4 || (strcmp(argv[1], "-1") != 0 && strcmp(argv[1], "-2") != 0 && strcmp(argv[1], "-3") != 0)) {
    fprintf(stderr, "USAGE: %s -{dpq} <PNG FILE> <OUT BASE NAME>\n", argv[0]);
    fprintf(stderr, "COMMANDS:\n");
    fprintf(stderr, " 1  Output 16 color palette image\n");
    fprintf(stderr, " 2  Output 256 color palette image\n");
    fprintf(stderr, " 3  Output 16bit direct color image\n");
    return -1;
  }

  if (png_load(argv[2], &dX, &dY, &type, &d) == FAILED)
    return -1;

  /* squash the image -> 16bit */
  _image_squash(d, dX, dY, type);

  /* set color 0 (transparent) */
  palette[paletteEntries++] = 0xFFFF;

  /* collect colors */
  if (strcmp(argv[1], "-1") == 0) {
    if (_palette_collect_smash(d, dX, dY, 16) == FAILED)
      return -1;
  }
  else if (strcmp(argv[1], "-2") == 0) {
    if (_palette_collect_smash(d, dX, dY, 256) == FAILED)
      return -1;
  }

  /* write the image data */
  sprintf(name, "%s.bin", argv[3]);
  f = fopen(name, "wb");
  if (f == NULL) {
    fprintf(stderr, "MAIN: Could not open file \"%s\" for writing.\n", name);
    return -1;
  }

  /* write DX and DY */
  /*
    _write_u16(f, dX);
    _write_u16(f, dY);
  */

  if (strcmp(argv[1], "-1") == 0) {
    /* 16 color image */
    for (i = 0; i < dX*dY; i += 4) {
      fprintf(f, "%c", (d[i+1] << 4) | d[i+0]);
      fprintf(f, "%c", (d[i+3] << 4) | d[i+2]);
    }
  }
  else if (strcmp(argv[1], "-2") == 0) {
    /* 256 color image */
    for (i = 0; i < dX*dY; i++) {
      fprintf(f, "%c", d[i]);
    }
  }
  else if (strcmp(argv[1], "-3") == 0) {
    /* 16bit image */
    for (i = 0; i < dX*dY*2; i += 2) {
      _write_u16(f, (d[i+0] << 8) | d[i+1]);
    }
  }

  fclose(f);
  free(d);

  /* write the palette data */
  if (strcmp(argv[1], "-1") == 0 || strcmp(argv[1], "-2") == 0) {
    sprintf(name, "%s.pal", argv[3]);
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
  }

  return 0;
}
