#ifndef __STMFLASH_H
#define __STMFLASH_H

#include "stm32l1xx.h"

uint32_t STMFLASH_ReadWord(uint32_t Address);               // ��ȡ32λ����
FLASH_Status STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite);  // д������
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead);  // ��ȡ����
void Test_Write(uint32_t WriteAddr, uint32_t WriteData);    // ����д��

#endif

















