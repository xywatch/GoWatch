#ifndef __LED_H
#define __LED_H
#include "sys.h"
#include "typedefs.h"

// LED�˿ڶ���
#define LED0 PBout(12) // DS0
#define LED1 PAout(6)  // DS1

void led_init(void); // ��ʼ��
void flashAllLed (void);
void turnOffAllLed (void);
#endif
