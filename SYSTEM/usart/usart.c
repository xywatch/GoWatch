#include "sys.h"
#include "usart.h"	  

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
//#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART1_RX_BUF[USART1_MAX_RECV_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART1_RX_STA=0;       //����״̬���	  

//u16 timertest=0; ////��ʱ������


//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
//u16 USART_RX_STA=0;       //����״̬���	  


#if 1
//��ʱ��2�жϷ������		    
void TIM2_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   

		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //���TIMx�����жϱ�־    
	    USART1_RX_STA|=1<<7; //��ǽ������
		TIM2_Set(0);		   //�ر�TIM4  
		
		//timertest++;
		//if(timertest>1000)
		//{
		//	timertest=0;
		//	printf("%u\n",timertest);
		//}
		
	}   
}
//����TIM2�Ŀ���
//sta:0���ر�;1,����;
void TIM2_Set(u8 sta)
{
	if(sta)
	{ 
		TIM_SetCounter(TIM2,0);  //���������
		TIM_Cmd(TIM2, ENABLE);   //ʹ��TIMx	
	}else TIM_Cmd(TIM2, DISABLE);//�رն�ʱ��4
}
//����TIM2Ԥװ������ֵ
void TIM2_SetARR(u16 period)
{
     TIM_SetCounter(TIM2,0); //���������
	 TIM2->ARR&=0x00;        //����Ԥװ������ֵΪ0
	 TIM2->ARR|= period;     //����Ԥװ������ֵ
}
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 

//ͨ�ö�ʱ���жϳ�ʼ��
//����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��		 
void TIM2_Init(u16 arr,u16 psc)
{	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��//TIM2ʱ��ʹ��    
	
	//��ʱ��TIM2��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM4�ж�,��������ж�

	 	  
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
}




#endif



//��ʼ��IO ����1 
//bound:������
void uart_init(u32 bound){

	//enable gpio clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	
	//enable usart1 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);



	GPIO_InitTypeDef GPIO_InitStructure;	
	USART_DeInit(USART1);  //��λ����1
	//connect PA.9 to usart1's tx
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	//connect PA.10 to usart1's rx
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);	
	/* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);    
    /* Configure USART Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_10;
    GPIO_Init(GPIOA,&GPIO_InitStructure);


    //GPIO�˿�����
    //GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//�GPIOAʱ��
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//ʹ��USART1��
 	//USART_DeInit(USART1);  //��λ����1
	 //USART1_TX   PA.9
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	//�����������
    //GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//����
    //GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
    
    //USART_ClearFlag(USART1,USART_FLAG_TC); 
    
	TIM2_Init(99,4700); 		//15ms�ж�
	USART1_RX_STA=0;		//����
	TIM2_Set(0);			//�رն�ʱ��2

}


//��������
void usart_sendbyte(unsigned char * byte, unsigned char len)
{
	unsigned char i = 0;
	USART_ClearFlag(USART1,USART_FLAG_TC); //�����������͵�һ���ֽڶ�ʧ
	for(i = 0; i < len; i++)
	{
		USART_SendData(USART1, byte[i]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {}   // ���ݷ������ʱ  TC��־��Ϊ1 ���� Ҳ��ѡ��TXE ���ͼĴ�����
		
	}
}



#if EN_USART1_RX   //���ʹ���˽���
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 res;	    
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART1);
		
		if((USART1_RX_STA&(1<<7))==0)
		{	
			
			if(USART1_RX_STA<USART1_MAX_RECV_LEN)		//�����Խ�������
			{
				
				//if(!Lora_mode)
				{
					TIM_SetCounter(TIM2,0); //���������      
					if(USART1_RX_STA==0)TIM2_Set(1);	 	//ʹ�ܶ�ʱ��4���ж� 
				}
				USART1_RX_BUF[USART1_RX_STA++]=res;		//��¼���յ���ֵ	 				
			}else 
			{
				USART1_RX_STA|=1<<7;					//ǿ�Ʊ�ǽ������
			}
	   }		
	}  	
} 
#endif	

