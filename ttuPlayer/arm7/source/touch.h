
#ifndef _TOUCH_H
#define _TOUCH_H

struct touch {
  s16 touchX;
  s16 touchY;
  s16 pixelX;
  s16 pixelY;
};

void touchInit(void);
void touchReadPosition(struct touch *touch);

#endif
