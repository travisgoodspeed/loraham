#ifndef TOUCH_H_
#define TOUCH_H_

void touchInit();

typedef struct {
  unsigned int x1;
  unsigned int y1;
  unsigned int x2;
  unsigned int y2;
  int id;  
} TouchRegion;

void setTouchRegions(TouchRegion *newRegions);
int getTouchRegionId();

#endif // TOUCH_H_
