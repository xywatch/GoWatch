#include "common.h"
#include "nvic.h"
#include "oled_driver.h"

bool DeepSleepFlag = 0;
extern bool isAlarmTriggered;

void nvic_sleep(u8 source) {
    if (isAlarmTriggered) { // ���Ӵ���ʱ, ������STOPģʽ
        return;
    }

    DeepSleepFlag = 1;
    OLED_OFF();
    menuData.isOpen = false;  // �رղ˵�
    printf("to stop mode %d\n", source);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // ����ͣ��ģʽ ����͹���ģʽ, ʹ�� WFI (Wait For Interrupt) ָ����� STOP ģʽ
    printf("exit stop mode %d\n", source); // �����жϺ��ִ�� ���Ѻ�����ִ�� c_loop() �����еĴ��룬����������ִ�� main() ������
}

void nvic_wake_up(u8 source) {
    if (!DeepSleepFlag) { // ���û��sleep, ��ִ�л���
        return;
    }
    // ���������RCC_Configurationʱ�Ӿͻ����!!�ײ�!!
    // ����STOPģʽ��PLLͣ���ˣ����ԣ������ʼ��ʱ�����ã��õ���PLL����ô���Ѻ���Ҫ��������RCC��
    // RCC_Configuration(); // ֻ�� RCC_Configuration()��, print����Ч
    RCC_Configuration(); // ֻ�� RCC_Configuration()��, print����Ч
    DeepSleepFlag = 0;
    OLED_ON();
    if (source != 9) { // alarm�жϻ���ʱ, ��ִ�ж���, ��Ϊalarm���Ѻ�Ҫ������ʾ���ӽ���
        exitMeThenRun(display_load);
    }
    userWake(); // ����, ��Ȼ��Ϊ��ťû���ֻ����STOPģʽ
    printf("wake up %d\n", source);
}

// PB1 �� ��
// PA7 ��
// PA3 �� ��
// v8:
// UP_BTN_KEY: PB1 �� ��
// CONFIRM_BTN_KEY: PA5 ��
// DOWN_BTN_KEY: PA4 �� ��

// �����жϳ�ʼ�� PA5
void KEY_INT_INIT(void) 
{
    NVIC_InitTypeDef NVIC_InitStruct; // ����ṹ�����
    EXTI_InitTypeDef EXTI_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // ����GPIOʱ�� ����Ҫ�븴��ʱ��һͬ������
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // ���ö˿��ж���Ҫ���ø���ʱ��

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5); // ���� GPIO  �ж� PA5

    EXTI_InitStruct.EXTI_Line = EXTI_Line5;              // �����ж��� PA5
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;               // �ж�ʹ��
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     // �ж�ģʽΪ �ж�
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // �½��ش���

    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;        // �ж���
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;           // ʹ���ж�
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // ��ռ���ȼ� 2
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;        // �����ȼ�  2
    NVIC_Init(&NVIC_InitStruct);
}

// �м�keyҪ��������restart, ��Ϊ�½��ش���, ���������, CONFIRM_BTN_KEY���������Ǹߵ�ƽ��
// �ⲿ�ж�9~5�������
// IO���� PA7
void EXTI9_5_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line5); // ��� LINE �ϵ��жϱ�־λ
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) // PA5
    {
        printf("EXTI9_5_IRQHandler CONFIRM_BTN_KEY=%d, DeepSleepFlag=%d\n", CONFIRM_BTN_KEY, DeepSleepFlag);
        if (CONFIRM_BTN_KEY == 0 && DeepSleepFlag == 1)
        {
            // delay_ms(80); // ΪʲôҪdelay? ��Ϊ�½��ش���, ���������, CONFIRM_BTN_KEY���������Ǹߵ�ƽ��
            // ������Ϊstop��, ʱ�ӻ����, delay��׼, ȥ��delay��Ҳ���ó�����, ��ĻҲ����(��Ȼż���᲻��)
            // �ж�ĳ�����ϵ��ж��Ƿ���, 
            if (CONFIRM_BTN_KEY == 0)
            {
                nvic_wake_up(3);
                printf("wake up by key int\n");
            }
        }
        else
        {
            // EXTI_ClearITPendingBit(EXTI_Line5); // ��� LINE �ϵ��жϱ�־λ
        }
    }
}

// ��ʼ�� pcf8563 �ж� PB4
void RTC_INT_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // ���� GPIOB ʱ�Ӻ� AFIO ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // ���� PB4 Ϊ�������룬��Ϊ pcf8563 ��INT�����ǵ͵�ƽ��Ч
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  // PB4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // ��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // �� PB4 ӳ�䵽 EXTI4
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

    // ���� EXTI4
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;  // EXTI4
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;  // �ж�ģʽ
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  // �½��ش���
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // ���� NVIC�����ýϸ����ȼ�
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;  // EXTI4 �ж�ͨ��
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;  // �����ռ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  // ��������ȼ�
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void RTC_Alarm_Handler(void)
{
    RTC_ClearAlarm(); // �������, �ж�, Ҳ�����һ������
    alarm_need_updateAlarm_in_nextLoop(); // ����һ��alarm_update()���ٸ���alarm

    if (DeepSleepFlag) {
        printf("Waking up from deep sleep\n");
        nvic_wake_up(9);
    }
    userWake();
    alarm_update();
}

// EXTI4 �жϴ�����
void EXTI4_IRQHandler(void)
{
    printf("EXTI4_IRQHandler triggered!\n");
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        // ����жϱ�־λ
        EXTI_ClearITPendingBit(EXTI_Line4);

        // ���� RTC ���Ӵ�����
        RTC_Alarm_Handler();
    }
}

void RCC_Configuration(void)
{
    // RCCʱ�ӵ�����
    ErrorStatus HSEStartUpStatus;
    RCC_DeInit();                               /* RCC system reset(for debug purpose) RCC�Ĵ����ָ���ʼ��ֵ*/
    RCC_HSEConfig(RCC_HSE_ON);                  /* Enable HSE ʹ���ⲿ���پ���*/
    HSEStartUpStatus = RCC_WaitForHSEStartUp(); /* Wait till HSE is ready �ȴ��ⲿ���پ���ʹ�����*/

    if (HSEStartUpStatus == SUCCESS)
    {
        /*����PLLʱ��Դ����Ƶϵ��*/
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // RCC_PLLMul_x��ö��2~16���Ǳ�Ƶֵ����HSE=8MHZ,RCC_PLLMul_9ʱPLLCLK=72MHZ
        /*����AHBʱ�ӣ�HCLK��*/
        RCC_HCLKConfig(RCC_SYSCLK_Div1); // RCC_SYSCLK_Div1����AHBʱ�� = ϵͳʱ��(SYSCLK) = 72MHZ���ⲿ����8HMZ��
        /*ע��˴������ã����ʹ��SYSTICK����ʱ���򣬴�ʱSYSTICK(Cortex System timer)=HCLK/8=9MHZ*/
        RCC_PCLK1Config(RCC_HCLK_Div2); // ���õ���AHBʱ�ӣ�PCLK1��,RCC_HCLK_Div2����APB1ʱ�� = HCLK/2 = 36MHZ���ⲿ����8HMZ��
        RCC_PCLK2Config(RCC_HCLK_Div1); // ���ø���AHBʱ�ӣ�PCLK2��,RCC_HCLK_Div1����APB2ʱ�� = HCLK = 72MHZ���ⲿ����8HMZ��
        /*ע��AHB��Ҫ�����ⲿ�洢��ʱ�ӡ�APB2����AD��I/O���߼�TIM������1��APB1����DA��USB��SPI��I2C��CAN������2��3��4��5����ͨTIM */
        // FLASH_SetLatency(FLASH_Latency_2); //����FLASH�洢����ʱʱ��������
        /*FLASHʱ���ӳټ������ڣ��ȴ�����ͬ��������
        �Ƽ����յ�Ƭ��ϵͳ����Ƶ�ʣ�
        0��24MHzʱ��ȡLatency_0��
        24��48MHzʱ��ȡLatency_1��
        48~72MHzʱ��ȡLatency_2*/
        // FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //ѡ��FLASHԤȡָ�����ģʽ��Ԥȡָ����ʹ��
        RCC_PLLCmd(ENABLE); // ʹ��PLL

        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
            ; // �ȴ�PLL����ȶ�

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // ѡ��SYSCLKʱ��ԴΪPLL

        while (RCC_GetSYSCLKSource() != 0x08)
            ; // �ȴ�PLL��ΪSYSCLKʱ��Դ
    }

    /*��ʼʹ�ܳ�������Ҫʹ�õ�����ʱ��*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE); // APB2����ʱ��ʹ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}
