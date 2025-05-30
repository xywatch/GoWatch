#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H
#include "typedefs.h"

#define SSD1306 1 // 0.96, 也可在1.3的微雪显示器中打开, 只是右边有一竖乱码
// #define SH1106 1 // 1.3

#if defined(SSD1306)
#define __SET_COL_START_ADDR() \
	do                         \
	{                          \
		WriteCmd(0x00);        \
		WriteCmd(0x10);        \
	} while (0)
#elif defined(SH1106)
#define __SET_COL_START_ADDR() \
	do                         \
	{                          \
		WriteCmd(0x02);        \
		WriteCmd(0x10);        \
	} while (0)
#endif

#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_15) // SCL=0
#define OLED_SCLK_Set() GPIO_SetBits(GPIOB, GPIO_Pin_15)   // SCL=1

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_14) // SDA=0
#define OLED_SDIN_Set() GPIO_SetBits(GPIOB, GPIO_Pin_14)   // SDA=1


void WriteCmd(unsigned char cmd);
void WriteDat(unsigned char Dat);
void WriteDats(unsigned char *dat, uint8_t len);

void SW_IIC_Configuration(void);

void OLED_InitIt(void);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_FILL(unsigned char BMP[]);
void OLED_DriverInit(void);

#endif //__OLED_DRIVER_H
