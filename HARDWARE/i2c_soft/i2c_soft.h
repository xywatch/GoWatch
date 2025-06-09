#ifndef __I2C_SOFT_H
#define __I2C_SOFT_H
#include "sys.h"

#define SCL_PIN GPIO_Pin_7 // PA7
#define SDA_PIN GPIO_Pin_0 // PB0
#define SCL_PORT GPIOA
#define SDA_PORT GPIOB
#define SCL_RCC_CLOCK RCC_AHBPeriph_GPIOA
#define SDA_RCC_CLOCK RCC_AHBPeriph_GPIOB

// #define SCL_H GPIOA->BSRR = GPIO_Pin_7
// #define SCL_L GPIOA->BRR = GPIO_Pin_7

// #define SDA_H GPIOB->BSRR = GPIO_Pin_0
// #define SDA_L GPIOB->BRR = GPIO_Pin_0

// #define SCL_read GPIOA->IDR & GPIO_Pin_7
// #define SDA_read GPIOB->IDR & GPIO_Pin_0

// I2C 引脚操作宏定义
#define SCL_H      GPIO_SetBits(SCL_PORT, SCL_PIN)
#define SCL_L      GPIO_ResetBits(SCL_PORT, SCL_PIN)
#define SDA_H      GPIO_SetBits(SDA_PORT, SDA_PIN)
#define SDA_L      GPIO_ResetBits(SDA_PORT, SDA_PIN)
#define SCL_read   GPIO_ReadInputDataBit(SCL_PORT, SCL_PIN)
#define SDA_read   GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN)

// #define I2C_PageSize  8  //24C02每页8字节
void I2C_GPIO_Config(void);
int I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
int I2C_WaitAck(void);
void I2C_SendByte(u8 SendByte);
u8 I2C_ReceiveByte(void);
// bool I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress);
// bool I2C_BufferWrite(u8* pBuffer, u8 length, u16 WriteAddress, u8 DeviceAddress);
// void I2C_PageWrite(u8* pBuffer, u8 length, u16 WriteAddress, u8 DeviceAddress);
// bool I2C_ReadByte(u8* pBuffer, u8 length, u16 ReadAddress, u8 DeviceAddress);
// void I2C_Test(void);

int Single_Write(unsigned char SlaveAddress, unsigned char REG_Address, unsigned char REG_data); // void
unsigned char Single_Read(unsigned char SlaveAddress, unsigned char REG_Address);

int MPU_I2C_WaitAck(void);
void I2C_ScanDevices(void);

#endif
