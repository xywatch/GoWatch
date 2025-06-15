#ifndef __LED_H
#define __LED_H
#include "sys.h"
#include "typedefs.h"

// LED端口定义
#define LED0 PBout(12) // DS0
#define LED1 PAout(6)  // DS1

void led_init(void); // 初始化
void flashAllLed (void);
void turnOffAllLed (void);
#endif
