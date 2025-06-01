#ifndef BMA423_WATCH_H
#define BMA423_WATCH_H

#include "common.h"
#include "bma.h"

bool bmaConfig(void);
void showAccelerometer(void);
uint16_t initBma423(void);
uint8_t bma423_get_direction(void);
float bma423_get_temp(void);
void resetStepCounter (void);
void enableTiltWrist (bool );
void enableDoubleTap(bool );

#endif
