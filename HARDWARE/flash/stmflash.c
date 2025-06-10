#include "stmflash.h"
#include "delay.h"
#include "usart.h"

// Flash��ʼ��ַ
#define FLASH_START_ADDR    0x08000000
// Flash������ַ
#define FLASH_END_ADDR      0x0801FFFF  // 128KB
// ҳ��С
#define FLASH_PAGE_SIZE     2048        // 2KB
// ��ҳ��
#define FLASH_PAGE_NUM      64          // 128KB/2KB = 64ҳ

// ��ȡָ����ַ������
uint32_t STMFLASH_ReadWord(uint32_t Address)
{
	return *(__IO uint32_t*)Address;
}

// д�����ݵ�ָ����ַ
FLASH_Status STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t pageAddr;
	uint32_t i;
	uint32_t primask;
	
	// ����ַ�Ƿ���Ч
	if(WriteAddr < FLASH_START_ADDR || WriteAddr >= FLASH_END_ADDR)
	{
		printf("Invalid address: 0x%08X\n", WriteAddr);
		return FLASH_ERROR_PROGRAM;
	}
	
	// ����ַ����
	if(WriteAddr & 0x3)
	{
		printf("Address not aligned: 0x%08X\n", WriteAddr);
		return FLASH_ERROR_PROGRAM;
	}
	
	// �����ж�
	primask = __get_PRIMASK();
	__disable_irq();
	
	// ���Flash״̬�Ĵ���
	printf("Initial FLASH->SR: 0x%08X\n", FLASH->SR);
	printf("Initial FLASH->PECR: 0x%08X\n", FLASH->PECR);
	
	// �ȴ�Flash����
	while(FLASH->SR & FLASH_SR_BSY)
	{
		printf("Waiting for Flash to be ready...\n");
		delay_ms(10);
	}
	
	// ���Flash���������Ƚ���
	if(FLASH->PECR & FLASH_PECR_PELOCK)
	{
		printf("Flash is locked, unlocking...\n");
		FLASH_Unlock();
	}
	
	// ���������״̬
	printf("After unlock - FLASH->PECR: 0x%08X\n", FLASH->PECR);
	
	// ��ȫ����Flash״̬
	FLASH->SR = 0x00000000;  // �������״̬λ
	FLASH->PECR &= ~(FLASH_PECR_PROG | FLASH_PECR_ERASE);  // ���PROG��ERASEλ
	
	// ������д����־
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_ENDHV|FLASH_FLAG_READY|FLASH_FLAG_WRPERR|FLASH_FLAG_PGAERR|FLASH_FLAG_SIZERR|FLASH_FLAG_OPTVERR|FLASH_FLAG_OPTVERRUSR|FLASH_FLAG_RDERR);
	
	// ���Flash״̬
	status = FLASH_GetStatus();
	printf("Flash Status after reset and clear: %d\n", status);
	
	// ���Flash״̬�Ĵ���
	printf("After reset - FLASH->SR: 0x%08X\n", FLASH->SR);
	printf("After reset - FLASH->PECR: 0x%08X\n", FLASH->PECR);
	
	// ����ҳ��ַ
	pageAddr = WriteAddr & ~(FLASH_PAGE_SIZE - 1);
	printf("Page Address: 0x%08X\n", pageAddr);
	
	// ���õ�ѹ��Χ
	FLASH_SetLatency(FLASH_Latency_1);
	
	// ���д����
	uint32_t wrp = FLASH_OB_GetWRP();
	printf("Write Protection: 0x%08X\n", wrp);
	
	// �ȴ���һ���������
	status = FLASH_WaitForLastOperation(0x5000);
	printf("Wait for last operation status: %d\n", status);
	
	if(status == FLASH_COMPLETE)
	{
		// ����PROGλ
		FLASH->PECR |= FLASH_PECR_PROG;
		
		// ����ERASEλ
		FLASH->PECR |= FLASH_PECR_ERASE;
		
		// ������ú��״̬
		printf("Before erase - FLASH->PECR: 0x%08X\n", FLASH->PECR);
		
		// д��0x00000000��ҳ��ʼ��ַ������ҳ
		*(__IO uint32_t*)pageAddr = 0x00000000;
		
		// �ȴ��������
		status = FLASH_WaitForLastOperation(0x5000);
		printf("Erase operation status: %d\n", status);
		
		// ���������״̬
		printf("After erase - FLASH->SR: 0x%08X\n", FLASH->SR);
		
		// ���ERASEλ
		FLASH->PECR &= ~FLASH_PECR_ERASE;
		
		// ���PROGλ
		FLASH->PECR &= ~FLASH_PECR_PROG;
		
		// �ٴ�������д����־
		FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_ENDHV|FLASH_FLAG_READY|FLASH_FLAG_WRPERR|FLASH_FLAG_PGAERR|FLASH_FLAG_SIZERR|FLASH_FLAG_OPTVERR|FLASH_FLAG_OPTVERRUSR|FLASH_FLAG_RDERR);
	}
	
	// ���Flash״̬
	status = FLASH_GetStatus();
	printf("Flash Status after erase: %d\n", status);
	
	if(status == FLASH_COMPLETE)
	{
		// д������
		for(i = 0; i < NumToWrite; i++)
		{
			// �ȴ���һ���������
			status = FLASH_WaitForLastOperation(0x5000);
			if(status != FLASH_COMPLETE)
			{
				break;
			}
			
			// д������
			*(__IO uint32_t*)WriteAddr = pBuffer[i];
			
			// �ȴ�д�����
			status = FLASH_WaitForLastOperation(0x5000);
			if(status != FLASH_COMPLETE)
			{
				break;
			}
			
			WriteAddr += 4;
		}
	}
	
	// ����Flash
	FLASH_Lock();
	
	// �ָ��ж�״̬
	if(!primask)
	{
		__enable_irq();
	}
	
	return status;
}

// ��ȡ����
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead)
{
	uint16_t i;
	
	for(i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr);
		ReadAddr += 4;
	}
}

// ����д��
void Test_Write(uint32_t WriteAddr, uint32_t WriteData)
{
	STMFLASH_Write(WriteAddr, &WriteData, 1);
}
