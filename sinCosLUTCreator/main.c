
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"


int main(int argc, char *argv[]) {

  int steps, s, data, mode = MODE_NONE, size;
  float angle, step, amplitude;
  char tmp[256];
  double res;


  if (argc != 7) {
    fprintf(stderr, "USAGE: %s <SIN/COS> <BYTE/SHORT/ASCII/JAVABYTE> <AMPLITUDE> <START ANGLE [0, 360[> <NUMBER OF STEPS> <STEP SIZE>\n", argv[0]);
    return 1;
  }

  sscanf(argv[1], "%255s", tmp);

  if (strcmp(tmp, "SIN") == 0)
    mode = MODE_SIN;
  else if (strcmp(tmp, "COS") == 0)
    mode = MODE_COS;
  else {
    fprintf(stderr, "The mode needs to be SIN or COS.\n");
    return 1;
  }

  sscanf(argv[2], "%255s", tmp);

  if (strcmp(tmp, "BYTE") == 0)
    size = SIZE_BYTE;
  else if (strcmp(tmp, "SHORT") == 0)
    size = SIZE_SHORT;
  else if (strcmp(tmp, "ASCII") == 0)
    size = SIZE_ASCII;
  else if (strcmp(tmp, "JAVABYTE") == 0)
    size = SIZE_JAVA_BYTE;
  else {
    fprintf(stderr, "The size need to be BYTE, SHORT, ASCII or JAVABYTE.\n");
    return 1;
  }

  sscanf(argv[3], "%f", &amplitude);
  sscanf(argv[4], "%f", &angle);
  sscanf(argv[5], "%d", &steps);
  sscanf(argv[6], "%f", &step);

  if (size == SIZE_JAVA_BYTE)
    fprintf(stdout, "byte[] sinTable = { ");

  for (s = 0; s < steps; s++) {
    /* calculate the 16bit data word */
    if (mode == MODE_SIN)
      res = sin(angle * 2 * M_PI / 360.0);
    else if (mode == MODE_COS)
      res = cos(angle * 2 * M_PI / 360.0);

    data = (int)(res * amplitude);

    /* output the 16bit data word */
    if (size == SIZE_SHORT)
      fprintf(stdout, "%c%c", data & 0xFF, ((data >> 8) & 0xFF));
    else if (size == SIZE_BYTE)
      fprintf(stdout, "%c", data & 0xFF);
    else if (size == SIZE_JAVA_BYTE)
      fprintf(stdout, "%d, ", data);
    else
      fprintf(stdout, "%d\n", data);

    /* next step */
    angle += step;
    while (angle < 0.0)
      angle += 360.0;
    while (angle >= 360.0)
      angle -= 360.0;
  }

  if (size == SIZE_JAVA_BYTE)
    fprintf(stdout, "}\n");

  return 0;
}
