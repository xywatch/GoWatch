#include "stmflash.h"
#include "delay.h"
#include "usart.h"

// Flash起始地址
#define FLASH_START_ADDR    0x08000000
// Flash结束地址
#define FLASH_END_ADDR      0x0801FFFF  // 128KB
// 页大小
#define FLASH_PAGE_SIZE     2048        // 2KB
// 总页数
#define FLASH_PAGE_NUM      64          // 128KB/2KB = 64页

// 读取指定地址的数据
uint32_t STMFLASH_ReadWord(uint32_t Address)
{
	return *(__IO uint32_t*)Address;
}

// 写入数据到指定地址
FLASH_Status STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t pageAddr;
	uint32_t i;
	uint32_t primask;
	
	// 检查地址是否有效
	if(WriteAddr < FLASH_START_ADDR || WriteAddr >= FLASH_END_ADDR)
	{
		printf("Invalid address: 0x%08X\n", WriteAddr);
		return FLASH_ERROR_PROGRAM;
	}
	
	// 检查地址对齐
	if(WriteAddr & 0x3)
	{
		printf("Address not aligned: 0x%08X\n", WriteAddr);
		return FLASH_ERROR_PROGRAM;
	}
	
	// 禁用中断
	primask = __get_PRIMASK();
	__disable_irq();
	
	// 检查Flash状态寄存器
	printf("Initial FLASH->SR: 0x%08X\n", FLASH->SR);
	printf("Initial FLASH->PECR: 0x%08X\n", FLASH->PECR);
	
	// 等待Flash空闲
	while(FLASH->SR & FLASH_SR_BSY)
	{
		printf("Waiting for Flash to be ready...\n");
		delay_ms(10);
	}
	
	// 如果Flash被锁定，先解锁
	if(FLASH->PECR & FLASH_PECR_PELOCK)
	{
		printf("Flash is locked, unlocking...\n");
		FLASH_Unlock();
	}
	
	// 检查解锁后的状态
	printf("After unlock - FLASH->PECR: 0x%08X\n", FLASH->PECR);
	
	// 完全重置Flash状态
	FLASH->SR = 0x00000000;  // 清除所有状态位
	FLASH->PECR &= ~(FLASH_PECR_PROG | FLASH_PECR_ERASE);  // 清除PROG和ERASE位
	
	// 清除所有错误标志
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_ENDHV|FLASH_FLAG_READY|FLASH_FLAG_WRPERR|FLASH_FLAG_PGAERR|FLASH_FLAG_SIZERR|FLASH_FLAG_OPTVERR|FLASH_FLAG_OPTVERRUSR|FLASH_FLAG_RDERR);
	
	// 检查Flash状态
	status = FLASH_GetStatus();
	printf("Flash Status after reset and clear: %d\n", status);
	
	// 检查Flash状态寄存器
	printf("After reset - FLASH->SR: 0x%08X\n", FLASH->SR);
	printf("After reset - FLASH->PECR: 0x%08X\n", FLASH->PECR);
	
	// 计算页地址
	pageAddr = WriteAddr & ~(FLASH_PAGE_SIZE - 1);
	printf("Page Address: 0x%08X\n", pageAddr);
	
	// 设置电压范围
	FLASH_SetLatency(FLASH_Latency_1);
	
	// 检查写保护
	uint32_t wrp = FLASH_OB_GetWRP();
	printf("Write Protection: 0x%08X\n", wrp);
	
	// 等待上一个操作完成
	status = FLASH_WaitForLastOperation(0x5000);
	printf("Wait for last operation status: %d\n", status);
	
	if(status == FLASH_COMPLETE)
	{
		// 设置PROG位
		FLASH->PECR |= FLASH_PECR_PROG;
		
		// 设置ERASE位
		FLASH->PECR |= FLASH_PECR_ERASE;
		
		// 检查设置后的状态
		printf("Before erase - FLASH->PECR: 0x%08X\n", FLASH->PECR);
		
		// 写入0x00000000到页起始地址来擦除页
		*(__IO uint32_t*)pageAddr = 0x00000000;
		
		// 等待擦除完成
		status = FLASH_WaitForLastOperation(0x5000);
		printf("Erase operation status: %d\n", status);
		
		// 检查擦除后的状态
		printf("After erase - FLASH->SR: 0x%08X\n", FLASH->SR);
		
		// 清除ERASE位
		FLASH->PECR &= ~FLASH_PECR_ERASE;
		
		// 清除PROG位
		FLASH->PECR &= ~FLASH_PECR_PROG;
		
		// 再次清除所有错误标志
		FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_ENDHV|FLASH_FLAG_READY|FLASH_FLAG_WRPERR|FLASH_FLAG_PGAERR|FLASH_FLAG_SIZERR|FLASH_FLAG_OPTVERR|FLASH_FLAG_OPTVERRUSR|FLASH_FLAG_RDERR);
	}
	
	// 检查Flash状态
	status = FLASH_GetStatus();
	printf("Flash Status after erase: %d\n", status);
	
	if(status == FLASH_COMPLETE)
	{
		// 写入数据
		for(i = 0; i < NumToWrite; i++)
		{
			// 等待上一个操作完成
			status = FLASH_WaitForLastOperation(0x5000);
			if(status != FLASH_COMPLETE)
			{
				break;
			}
			
			// 写入数据
			*(__IO uint32_t*)WriteAddr = pBuffer[i];
			
			// 等待写入完成
			status = FLASH_WaitForLastOperation(0x5000);
			if(status != FLASH_COMPLETE)
			{
				break;
			}
			
			WriteAddr += 4;
		}
	}
	
	// 上锁Flash
	FLASH_Lock();
	
	// 恢复中断状态
	if(!primask)
	{
		__enable_irq();
	}
	
	return status;
}

// 读取数据
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead)
{
	uint16_t i;
	
	for(i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr);
		ReadAddr += 4;
	}
}

// 测试写入
void Test_Write(uint32_t WriteAddr, uint32_t WriteData)
{
	STMFLASH_Write(WriteAddr, &WriteData, 1);
}
