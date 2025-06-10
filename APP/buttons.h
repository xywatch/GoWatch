/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_
#include "typedefs.h"

/*下面的方式是通过直接操作库函数方式读取IO*/
// #define KEY_UP      GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)   //back
// #define UP_BTN_KEY GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) // left sw3
// #define CONFIRM_BTN_KEY GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) // ok sw2
// #define DOWN_BTN_KEY GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) // right sw1

#define DOWN_BTN_KEY GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) // left sw3
#define CONFIRM_BTN_KEY GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) // ok sw2
#define UP_BTN_KEY GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) // right sw1

/*下面方式是通过位带操作方式读取IO*/
/*
#define UP_BTN_KEY 		PEin(4)   	//PE4
#define CONFIRM_BTN_KEY 		PEin(3)		//PE3
#define DOWN_BTN_KEY 		PEin(2)		//P32
#define WK_UP 	PAin(0)		//PA0
*/

typedef enum
{
	BTN_1 = 0,
	BTN_2 = 1,
	BTN_3 = 2,
	BTN_COUNT = 3 // must be last
} btn_t;

void buttons_init(void);
void buttons_update(void);
void buttons_startup(void);
void buttons_shutdown(void);
button_f buttons_setFunc(btn_t, button_f);
void buttons_setFuncs(button_f, button_f, button_f);
// millis_t buttons_pressTime(btn_t);
bool buttons_isActive(void);
void buttons_wake(void);
millis_t buttons_lastPressedTime(void);

#endif /* BUTTONS_H_ */
