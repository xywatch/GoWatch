#ifndef __OLED_H
#define __OLED_H
#include "sys.h"
#include "stdlib.h"

void OLED_Init(void);       // ��ʼ��													//����ʾ
void OLED_Clear(u16 Color); // ����
void OLED_Flush(void);
void OLED_ClearScreenBuffer(void);

#endif
