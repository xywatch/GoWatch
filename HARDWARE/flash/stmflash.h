#ifndef __STMFLASH_H
#define __STMFLASH_H

#include "stm32l1xx.h"

uint32_t STMFLASH_ReadWord(uint32_t Address);               // 读取32位数据
FLASH_Status STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite);  // 写入数据
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead);  // 读取数据
void Test_Write(uint32_t WriteAddr, uint32_t WriteData);    // 测试写入

#endif

















