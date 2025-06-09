#include "led.h"

// LED control

void led_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO clocks
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOA, ENABLE);

	// Configure PB12 (LED0)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // Set initial state to OFF

	// Configure PA6 (LED1)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);  // Set initial state to OFF
}

u8 is_turnOn1 = 0;
u8 is_turnOn2 = 0;

void turnOnLed1()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	is_turnOn1 = 1;
}

void turnOffLed1()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	is_turnOn1 = 0;
}


void turnOnLed2()
{
	GPIO_SetBits(GPIOA, GPIO_Pin_6);
	is_turnOn2 = 1;
}

void turnOffLed2()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	is_turnOn2 = 0;
}

void turnOffAllLed () {
	turnOffLed1();
	turnOffLed2();
}

// Á½¸öLEDÂÖÁ÷ÏÔÊ¾
void flashAllLed () {
	if (is_turnOn1) {
		turnOffLed1();
		turnOnLed2();
	}
	else {
		turnOffLed2();
		turnOnLed1();
	}
}