#ifndef BMA_H
#define BMA_H

#include <stdint.h>
#include "bma423.h"
#include "typedefs.h"

#define DIRECTION_TOP_EDGE 0
#define DIRECTION_BOTTOM_EDGE 1
#define DIRECTION_LEFT_EDGE 2
#define DIRECTION_RIGHT_EDGE 3
#define DIRECTION_DISP_UP 4
#define DIRECTION_DISP_DOWN 5

typedef struct bma4_accel Accel;
typedef struct bma4_accel_config Acfg;

bool bma_init(bma4_com_fptr_t , bma4_com_fptr_t ,
              bma4_delay_fptr_t ,
              uint8_t );

void bma_softReset(void);
void bma_shutDown(void);
void bma_wakeUp(void);
bool bma_selfTest(void);

uint8_t bma_getDirection(void);

bool bma_setAccelConfig(Acfg *);
bool bma_getAccelConfig(Acfg *);
bool bma_getAccel(Accel *);
bool bma_getAccelEnable(void);
bool bma_disableAccel(void);
bool bma_enableAccel(bool );

bool bma_setINTPinConfig(struct bma4_int_pin_config , uint8_t );
bool bma_getINT(void);
uint8_t bma_getIRQMASK(void);
bool bma_disableIRQ(uint16_t );
bool bma_enableIRQ(uint16_t );
bool bma_isStepCounter(void);
bool bma_isDoubleClick(void);
bool bma_isTilt(void);
bool bma_isActivity(void);
bool bma_isAnyNoMotion(void);

bool bma_resetStepCounter(void);
uint32_t bma_getCounter(void);

float bma_readTemperature(void);
float bma_readTemperatureF(void);

uint16_t bma_getErrorCode(void);
uint16_t bma_getStatus(void);
uint32_t bma_getSensorTime(void);

const char *bma_getActivity(void);
bool bma_setRemapAxes(struct bma423_axes_remap *);

bool bma_enableFeature(uint8_t , uint8_t );
bool bma_enableStepCountInterrupt(bool);
bool bma_enableTiltInterrupt(bool);
bool bma_enableWakeupInterrupt(bool);
bool bma_enableAnyNoMotionInterrupt(bool);
bool bma_enableActivityInterrupt(bool);

struct bma4_dev *bma_getDevFptr(void);

#endif
