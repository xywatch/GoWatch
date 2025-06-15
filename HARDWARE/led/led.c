#include "led.h"
#include "common.h"

void led_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE); // 使能PA,PD端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 关闭jtag，使能SWD，可以用SWD模式调试

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;		  // LED0-->PB.12 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);			  // 根据设定参数初始化GPIOB.12
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);				  // PB.12 输出低

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // PA6
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置成上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);			 // 初始化GPIOA6
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

// 两个LED轮流显示
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