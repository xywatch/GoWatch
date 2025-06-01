#ifndef __I2C_SOFT2_H
#define __I2C_SOFT2_H
#include "sys.h"

#define SCL2_PIN GPIO_Pin_15 // PB15
#define SDA2_PIN GPIO_Pin_14 // PB14

#define SCL2_PORT GPIOB
#define SDA2_PORT GPIOB
#define SCL2_RCC_CLOCK RCC_APB2Periph_GPIOB
#define SDA2_RCC_CLOCK RCC_APB2Periph_GPIOB

#define SCL2_H GPIOB->BSRR = GPIO_Pin_15
#define SCL2_L GPIOB->BRR = GPIO_Pin_15

#define SDA2_H GPIOB->BSRR = GPIO_Pin_14
#define SDA2_L GPIOB->BRR = GPIO_Pin_14

#define SCL2_read GPIOB->IDR & GPIO_Pin_15
#define SDA2_read GPIOB->IDR & GPIO_Pin_14

// #define I2C_PageSize  8  //24C02Ã¿Ò³8×Ö½Ú
void I2C2_GPIO_Config(void);
int I2C2_Start(void);
void I2C2_Stop(void);
void I2C2_Ack(void);
void I2C2_NoAck(void);
int I2C2_WaitAck(void);
void I2C2_ScanDevices(void);
void I2C2_SendByte(u8);
u8 I2C2_ReceiveByte(void);
#endif
