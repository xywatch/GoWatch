#include "common.h"
#include "nvic.h"
#include "oled_driver.h"

bool DeepSleepFlag = 0;
extern bool isAlarmTriggered;

void nvic_sleep(u8 source)
{
    if (isAlarmTriggered)
    { // 闹钟触发时, 不进入STOP模式
        return;
    }

    DeepSleepFlag = 1;
    OLED_OFF();
    turnOffAllLed();
    menuData.isOpen = false; // 关闭菜单
    printf("to stop mode %d\r\n", source);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // 进入停机模式 进入低功耗模式, 使用 WFI (Wait For Interrupt) 指令进入 STOP 模式
    printf("exit stop mode %d\r\n", source);                      // 按键中断后会执行 唤醒后会继续执行 c_loop() 函数中的代码，而不是重新执行 main() 函数。
}

void nvic_wake_up(u8 source)
{
    if (!DeepSleepFlag)
    { // 如果没有sleep, 则不执行唤醒
        return;
    }
    // 如果不重新RCC_Configuration时钟就会很慢!!亲测!!
    // 进了STOP模式后，PLL停掉了，所以，如果开始的时钟配置，用的是PLL，那么唤醒后，需要重新配置RCC。
    RCC_Configuration(); // 只有 RCC_Configuration()后, print才有效
    DeepSleepFlag = 0;
    OLED_ON();
    if (source != 9)
    { // alarm中断唤醒时, 不执行动画, 因为alarm唤醒后要立即显示闹钟界面
        exitMeThenRun(display_load);
    }
    userWake(); // 唤醒, 不然以为按钮没动又会进入STOP模式
    printf("wake up %d\r\n", source);
}

// PB1 上 右
// PA7 中
// PA3 下 左
// v8:
// UP_BTN_KEY: PB1 上 右
// CONFIRM_BTN_KEY: PA5 中
// DOWN_BTN_KEY: PA4 下 左

// 按键中断初始化 PA5
void KEY_INT_INIT(void)
{
    NVIC_InitTypeDef NVIC_InitStruct; // 定义结构体变量
    EXTI_InitTypeDef EXTI_InitStruct;

    // 使能 GPIOA 和 SYSCFG 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);    // 启动GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // 配置端口中断需要启用SYSCFG时钟

    // 配置 PA5 为输入模式
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 将 PA5 映射到 EXTI5
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5); // 定义 GPIO 中断 PA5

    // 配置 EXTI5
    EXTI_InitStruct.EXTI_Line = EXTI_Line5;              // 定义中断线 PA5
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;               // 中断使能
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     // 中断模式为 中断
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_Init(&EXTI_InitStruct);

    // 配置 NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;        // 中断线
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;           // 使能中断
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级 2
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;        // 子优先级 4
    NVIC_Init(&NVIC_InitStruct);
}

// 中键key要长按才能restart, 因为下降沿触发, 如果弹出了, CONFIRM_BTN_KEY读出来就是高电平了
// 外部中断9~5处理程序
// IO引脚 PA5
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) // PA5
    {
        EXTI_ClearITPendingBit(EXTI_Line5); // 清除 LINE 上的中断标志位
        printf("EXTI9_5_IRQHandler CONFIRM_BTN_KEY=%d, DeepSleepFlag=%d\n", CONFIRM_BTN_KEY, DeepSleepFlag);
        if (CONFIRM_BTN_KEY == 0 && DeepSleepFlag == 1)
        {
            // delay_ms(80); // 为什么要delay? 因为下降沿触发, 如果弹出了, CONFIRM_BTN_KEY读出来就是高电平了
            // 但是因为stop后, 时钟会变慢, delay不准, 去掉delay后也不用长按了, 屏幕也亮了(不然偶尔会不亮)
            // 判断某个线上的中断是否发生
            if (CONFIRM_BTN_KEY == 0)
            {
                nvic_wake_up(3);
                printf("wake up by key int\n");
            }
        }
    }
}

// 初始化 pcf8563 中断 PB4
void RTC_INT_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 使能 GPIOB 和 SYSCFG 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);    // 启动GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // 配置端口中断需要启用SYSCFG时钟

    // 配置 PB4 为下拉输入，因为 pcf8563 的INT引脚是低电平有效
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;      // PB4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;   // 输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // 下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 将 PB4 映射到 EXTI4
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

    // 配置 EXTI4
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;              // EXTI4
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     // 中断模式
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 配置 NVIC，设置较高优先级
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;          // EXTI4 中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 最高抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;        // 最高子优先级
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void RTC_Alarm_Handler(void)
{
    RTC_ClearAlarm();                     // 清除闹钟, 中断, 也会清除一个闹钟
    alarm_need_updateAlarm_in_nextLoop(); // 在下一个alarm_update()后再更新alarm

    printf("RTC_Alarm_Handler\n");
    if (DeepSleepFlag)
    {
        printf("Waking up from deep sleep\n");
        nvic_wake_up(9);
    }

    userWake();
}

// EXTI4 中断处理函数
void EXTI4_IRQHandler(void)
{
    printf("EXTI4_IRQHandler triggered!\n");
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        // 清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line4);

        // 调用 RTC 闹钟处理函数
        RTC_Alarm_Handler();
    }
}

// int <--> PC13
void BMA_INT_INIT(void)
{
    // 启用 GPIOC 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    // 配置 PC13 为输入模式，启用内部上拉电阻
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 启用 SYSCFG 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 将 PC13 映射到 EXTI13
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);

    // 配置 EXTI13
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line13;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // 上升沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 配置 NVIC
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; // EXTI13 的中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        // 清除中断标志
        EXTI_ClearITPendingBit(EXTI_Line13); // 如果不清空就会一直执行, 卡死

        if (DeepSleepFlag)
        {
            printf("Waking up from deep sleep\n");
            nvic_wake_up(10);
        }
        userWake();

        // wakeup后打印才有效

        printf("EXTI_Line13 int\n");

        // 检查是否是BMA423唤醒
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
原文链接：https://blog.csdn.net/u012523921/article/details/102792648`
*/
void RCC_Configuration(void)
{
    // 在STM32L151中，必须使能初始化HSI时钟，不然ADC没法用
    Adc_HSI_Enable();

    __IO uint32_t count = 0; //__IO 是volatile的定义，表示每次使用都要重新重寄存器里取值

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
