#ifndef __NVIC_H
#define __NVIC_H
#include "sys.h"

void KEY_INT_INIT(void);
void RTC_INT_INIT(void);
void RCC_Configuration(void);
void nvic_sleep(u8 source);
void nvic_wake_up(u8 source);
#endif
