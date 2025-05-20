#ifndef __OLED_H
#define __OLED_H
#include "sys.h"
#include "stdlib.h"

void OLED_Init(void);       // 初始化													//关显示
void OLED_Clear(u16 Color); // 清屏
void OLED_Flush(void);
void OLED_ClearScreenBuffer(void);

#endif
