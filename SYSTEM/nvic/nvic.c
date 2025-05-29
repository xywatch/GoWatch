#include "common.h"
#include "nvic.h"
#include "oled_driver.h"

bool DeepSleepFlag = 0;
extern bool isAlarmTriggered;

void nvic_sleep(u8 source) {
    if (isAlarmTriggered) { // 闹钟触发时, 不进入STOP模式
        return;
    }

    DeepSleepFlag = 1;
    OLED_OFF();
    menuData.isOpen = false;  // 关闭菜单
    printf("to stop mode %d\n", source);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // 进入停机模式 进入低功耗模式, 使用 WFI (Wait For Interrupt) 指令进入 STOP 模式
    printf("exit stop mode %d\n", source); // 按键中断后会执行 唤醒后会继续执行 c_loop() 函数中的代码，而不是重新执行 main() 函数。
}

void nvic_wake_up(u8 source) {
    if (!DeepSleepFlag) { // 如果没有sleep, 则不执行唤醒
        return;
    }
    // 如果不重新RCC_Configuration时钟就会很慢!!亲测!!
    // 进了STOP模式后，PLL停掉了，所以，如果开始的时钟配置，用的是PLL，那么唤醒后，需要重新配置RCC。
    // RCC_Configuration(); // 只有 RCC_Configuration()后, print才有效
    RCC_Configuration(); // 只有 RCC_Configuration()后, print才有效
    DeepSleepFlag = 0;
    OLED_ON();
    if (source != 9) { // alarm中断唤醒时, 不执行动画, 因为alarm唤醒后要立即显示闹钟界面
        exitMeThenRun(display_load);
    }
    userWake(); // 唤醒, 不然以为按钮没动又会进入STOP模式
    printf("wake up %d\n", source);
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

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 启动GPIO时钟 （需要与复用时钟一同启动）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // 配置端口中断需要启用复用时钟

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5); // 定义 GPIO  中断 PA5

    EXTI_InitStruct.EXTI_Line = EXTI_Line5;              // 定义中断线 PA5
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;               // 中断使能
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;     // 中断模式为 中断
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发

    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;        // 中断线
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;           // 使能中断
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级 2
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;        // 子优先级  2
    NVIC_Init(&NVIC_InitStruct);
}

// 中键key要长按才能restart, 因为下降沿触发, 如果弹出了, CONFIRM_BTN_KEY读出来就是高电平了
// 外部中断9~5处理程序
// IO引脚 PA7
void EXTI9_5_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line5); // 清除 LINE 上的中断标志位
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) // PA5
    {
        printf("EXTI9_5_IRQHandler CONFIRM_BTN_KEY=%d, DeepSleepFlag=%d\n", CONFIRM_BTN_KEY, DeepSleepFlag);
        if (CONFIRM_BTN_KEY == 0 && DeepSleepFlag == 1)
        {
            // delay_ms(80); // 为什么要delay? 因为下降沿触发, 如果弹出了, CONFIRM_BTN_KEY读出来就是高电平了
            // 但是因为stop后, 时钟会变慢, delay不准, 去掉delay后也不用长按了, 屏幕也亮了(不然偶尔会不亮)
            // 判断某个线上的中断是否发生, 
            if (CONFIRM_BTN_KEY == 0)
            {
                nvic_wake_up(3);
                printf("wake up by key int\n");
            }
        }
        else
        {
            // EXTI_ClearITPendingBit(EXTI_Line5); // 清除 LINE 上的中断标志位
        }
    }
}

// 初始化 pcf8563 中断 PB4
void RTC_INT_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 启用 GPIOB 时钟和 AFIO 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 配置 PB4 为下拉输入，因为 pcf8563 的INT引脚是低电平有效
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  // PB4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // 下拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 将 PB4 映射到 EXTI4
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

    // 配置 EXTI4
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;  // EXTI4
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;  // 中断模式
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  // 下降沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 配置 NVIC，设置较高优先级
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;  // EXTI4 中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;  // 最高抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  // 最高子优先级
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void RTC_Alarm_Handler(void)
{
    RTC_ClearAlarm(); // 清除闹钟, 中断, 也会清除一个闹钟
    alarm_need_updateAlarm_in_nextLoop(); // 在下一个alarm_update()后再更新alarm

    if (DeepSleepFlag) {
        printf("Waking up from deep sleep\n");
        nvic_wake_up(9);
    }
    userWake();
    alarm_update();
}

// EXTI4 中断处理函数
void EXTI4_IRQHandler(void)
{
    printf("EXTI4_IRQHandler triggered!\n");
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        // 清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line4);

        // 调用 RTC 闹钟处理函数
        RTC_Alarm_Handler();
    }
}

void RCC_Configuration(void)
{
    // RCC时钟的设置
    ErrorStatus HSEStartUpStatus;
    RCC_DeInit();                               /* RCC system reset(for debug purpose) RCC寄存器恢复初始化值*/
    RCC_HSEConfig(RCC_HSE_ON);                  /* Enable HSE 使能外部高速晶振*/
    HSEStartUpStatus = RCC_WaitForHSEStartUp(); /* Wait till HSE is ready 等待外部高速晶振使能完成*/

    if (HSEStartUpStatus == SUCCESS)
    {
        /*设置PLL时钟源及倍频系数*/
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // RCC_PLLMul_x（枚举2~16）是倍频值。当HSE=8MHZ,RCC_PLLMul_9时PLLCLK=72MHZ
        /*设置AHB时钟（HCLK）*/
        RCC_HCLKConfig(RCC_SYSCLK_Div1); // RCC_SYSCLK_Div1——AHB时钟 = 系统时钟(SYSCLK) = 72MHZ（外部晶振8HMZ）
        /*注意此处的设置，如果使用SYSTICK做延时程序，此时SYSTICK(Cortex System timer)=HCLK/8=9MHZ*/
        RCC_PCLK1Config(RCC_HCLK_Div2); // 设置低速AHB时钟（PCLK1）,RCC_HCLK_Div2——APB1时钟 = HCLK/2 = 36MHZ（外部晶振8HMZ）
        RCC_PCLK2Config(RCC_HCLK_Div1); // 设置高速AHB时钟（PCLK2）,RCC_HCLK_Div1——APB2时钟 = HCLK = 72MHZ（外部晶振8HMZ）
        /*注：AHB主要负责外部存储器时钟。APB2负责AD，I/O，高级TIM，串口1。APB1负责DA，USB，SPI，I2C，CAN，串口2，3，4，5，普通TIM */
        // FLASH_SetLatency(FLASH_Latency_2); //设置FLASH存储器延时时钟周期数
        /*FLASH时序延迟几个周期，等待总线同步操作。
        推荐按照单片机系统运行频率：
        0—24MHz时，取Latency_0；
        24—48MHz时，取Latency_1；
        48~72MHz时，取Latency_2*/
        // FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //选择FLASH预取指缓存的模式，预取指缓存使能
        RCC_PLLCmd(ENABLE); // 使能PLL

        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
            ; // 等待PLL输出稳定

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // 选择SYSCLK时钟源为PLL

        while (RCC_GetSYSCLKSource() != 0x08)
            ; // 等待PLL成为SYSCLK时钟源
    }

    /*开始使能程序中需要使用的外设时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE); // APB2外设时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}
