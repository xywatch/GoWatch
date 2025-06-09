#include "common.h"
#include "nvic.h"
#include "oled_driver.h"

bool DeepSleepFlag = 0;
extern bool isAlarmTriggered;

void nvic_sleep(u8 source)
{
    if (isAlarmTriggered)
    { // ���Ӵ���ʱ, ������STOPģʽ
        return;
    }

    DeepSleepFlag = 1;
    OLED_OFF();
    turnOffAllLed();
    menuData.isOpen = false; // �رղ˵�
    printf("to stop mode %d\r\n", source);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // ����ͣ��ģʽ ����͹���ģʽ, ʹ�� WFI (Wait For Interrupt) ָ����� STOP ģʽ
    printf("exit stop mode %d\r\n", source);                      // �����жϺ��ִ�� ���Ѻ�����ִ�� c_loop() �����еĴ��룬����������ִ�� main() ������
}

void nvic_wake_up(u8 source)
{
    if (!DeepSleepFlag)
    { // ���û��sleep, ��ִ�л���
        return;
    }
    // ���������RCC_Configurationʱ�Ӿͻ����!!�ײ�!!
    // ����STOPģʽ��PLLͣ���ˣ����ԣ������ʼ��ʱ�����ã��õ���PLL����ô���Ѻ���Ҫ��������RCC��
    RCC_Configuration(); // ֻ�� RCC_Configuration()��, print����Ч
    DeepSleepFlag = 0;
    OLED_ON();
    if (source != 9)
    { // alarm�жϻ���ʱ, ��ִ�ж���, ��Ϊalarm���Ѻ�Ҫ������ʾ���ӽ���
        exitMeThenRun(display_load);
    }
    userWake(); // ����, ��Ȼ��Ϊ��ťû���ֻ����STOPģʽ
    printf("wake up %d\r\n", source);
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

    // ʹ�� GPIOA �� SYSCFG ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);    // ����GPIOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // ���ö˿��ж���Ҫ����SYSCFGʱ��

    // ���� PA5 Ϊ����ģʽ
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // ����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // ����
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // �� PA5 ӳ�䵽 EXTI5
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5); // ���� GPIO �ж� PA5

    // ���� EXTI5
    EXTI_InitStruct.EXTI_Line = EXTI_Line5;              // �����ж��� PA5
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;               // �ж�ʹ��
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     // �ж�ģʽΪ �ж�
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // �½��ش���
    EXTI_Init(&EXTI_InitStruct);

    // ���� NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;        // �ж���
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;           // ʹ���ж�
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // ��ռ���ȼ� 2
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;        // �����ȼ� 4
    NVIC_Init(&NVIC_InitStruct);
}

// �м�keyҪ��������restart, ��Ϊ�½��ش���, ���������, CONFIRM_BTN_KEY���������Ǹߵ�ƽ��
// �ⲿ�ж�9~5�������
// IO���� PA5
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) // PA5
    {
        EXTI_ClearITPendingBit(EXTI_Line5); // ��� LINE �ϵ��жϱ�־λ
        printf("EXTI9_5_IRQHandler CONFIRM_BTN_KEY=%d, DeepSleepFlag=%d\n", CONFIRM_BTN_KEY, DeepSleepFlag);
        if (CONFIRM_BTN_KEY == 0 && DeepSleepFlag == 1)
        {
            // delay_ms(80); // ΪʲôҪdelay? ��Ϊ�½��ش���, ���������, CONFIRM_BTN_KEY���������Ǹߵ�ƽ��
            // ������Ϊstop��, ʱ�ӻ����, delay��׼, ȥ��delay��Ҳ���ó�����, ��ĻҲ����(��Ȼż���᲻��)
            // �ж�ĳ�����ϵ��ж��Ƿ���
            if (CONFIRM_BTN_KEY == 0)
            {
                nvic_wake_up(3);
                printf("wake up by key int\n");
            }
        }
    }
}

// ��ʼ�� pcf8563 �ж� PB4
void RTC_INT_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // ʹ�� GPIOB �� SYSCFG ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);    // ����GPIOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // ���ö˿��ж���Ҫ����SYSCFGʱ��

    // ���� PB4 Ϊ�������룬��Ϊ pcf8563 ��INT�����ǵ͵�ƽ��Ч
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;      // PB4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;   // ����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // ����
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // �� PB4 ӳ�䵽 EXTI4
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

    // ���� EXTI4
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;              // EXTI4
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     // �ж�ģʽ
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // �½��ش���
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // ���� NVIC�����ýϸ����ȼ�
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;          // EXTI4 �ж�ͨ��
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // �����ռ���ȼ�
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;        // ��������ȼ�
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void RTC_Alarm_Handler(void)
{
    RTC_ClearAlarm();                     // �������, �ж�, Ҳ�����һ������
    alarm_need_updateAlarm_in_nextLoop(); // ����һ��alarm_update()���ٸ���alarm

    printf("RTC_Alarm_Handler\n");
    if (DeepSleepFlag)
    {
        printf("Waking up from deep sleep\n");
        nvic_wake_up(9);
    }

    userWake();
}

// EXTI4 �жϴ�����
void EXTI4_IRQHandler(void)
{
    printf("EXTI4_IRQHandler triggered!\n");
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        // ����жϱ�־λ
        EXTI_ClearITPendingBit(EXTI_Line4);

        // ���� RTC ���Ӵ�����
        RTC_Alarm_Handler();
    }
}

// int <--> PC13
void BMA_INT_INIT(void)
{
    // ���� GPIOC ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    // ���� PC13 Ϊ����ģʽ�������ڲ���������
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // ����
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // ���� SYSCFG ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // �� PC13 ӳ�䵽 EXTI13
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);

    // ���� EXTI13
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line13;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // �����ش���
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // ���� NVIC
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; // EXTI13 ���ж�ͨ��
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        // ����жϱ�־
        EXTI_ClearITPendingBit(EXTI_Line13); // �������վͻ�һֱִ��, ����

        if (DeepSleepFlag)
        {
            printf("Waking up from deep sleep\n");
            nvic_wake_up(10);
        }
        userWake();

        // wakeup���ӡ����Ч

        printf("EXTI_Line13 int\n");

        // ����Ƿ���BMA423����
        uint16_t int_status = bma_getInterruptStatus();
        if (int_status)
        {
            // uint32_t stepCount = sensor.getStepCounter();
            printf("BMA423 int status: %d\n", int_status);
            if (bma_isDoubleClick())
            {
                printf("is double click\n");
            }
            else if (bma_isTilt())
            {
                printf("is wrist tilt\n");
            }
            else if (bma_isStepCounter())
            {
                printf("is step count\n");
                uint32_t stepCount = bma_getStepCount();
                printf("stepCount: %d\n", stepCount);
            }
        }
    }
}

/*
ԭ�����ӣ�https://blog.csdn.net/u012523921/article/details/102792648`
*/
void RCC_Configuration(void)
{
    // ��STM32L151�У�����ʹ�ܳ�ʼ��HSIʱ�ӣ���ȻADCû����
    Adc_HSI_Enable();

    __IO uint32_t count = 0; //__IO ��volatile�Ķ��壬��ʾÿ��ʹ�ö�Ҫ�����ؼĴ�����ȡֵ

    RCC_HSEConfig(RCC_HSE_ON);
    while ((RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) && (count != HSE_STARTUP_TIMEOUT))
    {
        count++;
    }
    if ((RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET) && (count != HSE_STARTUP_TIMEOUT))
    {
        FLASH_ReadAccess64Cmd(ENABLE);
        FLASH_PrefetchBufferCmd(ENABLE);
        FLASH_SetLatency(FLASH_Latency_1);
        RCC_HCLKConfig(RCC_SYSCLK_Div1); // RCC_SYSCLK_Div1:   AHB clock = SYSCLK
        RCC_PCLK2Config(RCC_HCLK_Div1);  // RCC_HCLK_Div1:  APB2 clock = HCLK
        RCC_PCLK1Config(RCC_HCLK_Div1);  // 16MHz  RCC_HCLK_Div1:  APB1 clock = HCLK
        // RCC_PLLMul_6: PLL clock source multiplied by 6
        // RCC_PLLDiv_3: PLL Clock output divided by 3
        // RCC_PLLConfig(RCC_PLLSource_HSE, RCC_PLLMul_6, RCC_PLLDiv_3); // PLL = 8MHz * 6 /3 =16MHz
        RCC_PLLConfig(RCC_PLLSource_HSE, RCC_PLLMul_8, RCC_PLLDiv_2); // PLL = 8MHz * 8 /2 =32MHz
        RCC_PLLCmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
            ;
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // RCC_SYSCLKSource_PLLCLK: PLL selected as system clock source
        while (RCC_GetSYSCLKSource() != 0x0C)
            ; //  - 0x0C: PLL used as system clock
    }
    else
    {
        RCC_HSEConfig(RCC_HSE_OFF);
        RCC_DeInit();
        RCC_HSICmd(ENABLE);
        FLASH_ReadAccess64Cmd(ENABLE);
        FLASH_PrefetchBufferCmd(ENABLE);
        FLASH_SetLatency(FLASH_Latency_1);
        while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
            ;
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
        while (RCC_GetSYSCLKSource() != 0x04)
            ; //- 0x04: HSI used as system clock
    }
}
