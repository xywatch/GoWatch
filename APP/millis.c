#include "common.h"

extern float BatteryVol; // 改成u16会导致无法正常显示
millis_t milliseconds;

// Initialise library
// 1s = 1000ms 每一ms中断一次
// 50 * 1440 / 72M = 1000/M = 1ms
void millis_init()
{
    // Timer settings
    // 1、配置嵌套中断控制器NVIC
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//优先级分组
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM3_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic);

    // 2、定时器初始化配置
    TIM_TimeBaseInitTypeDef tim; // 结构体
    // 优先级函数调用
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 开启时钟
    TIM_DeInit(TIM3);                                    // ？？？？？
    tim.TIM_ClockDivision = TIM_CKD_DIV1;                // 采样分频
    tim.TIM_CounterMode = TIM_CounterMode_Up;            // 向上计数
    tim.TIM_Period = 50 - 1;                             // 之前是50, 应该是49; //自动重装载寄存器的值
    tim.TIM_Prescaler = 1440 - 1;                        // 时钟预分频
    // tim.TIM_RepetitionCounter=
    TIM_TimeBaseInit(TIM3, &tim);         // 初始化结构体
    TIM_ClearFlag(TIM3, TIM_FLAG_Update); // 清除溢出中断标志
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE); // 开启时钟
}

// Get current milliseconds
millis_t millis_get()
{
    millis_t ms;
    ms = milliseconds;
    return ms;
}

extern int altitude;
u8 log_time = 1;

// 电池电压滤波
static float batteryFilter(float newValue) {
    static float filteredValue = 0;
    static float lastValue = 0;
    const float alpha = 0.3f;    // 增大滤波系数，让电压变化更快响应
    const float jumpThreshold = 0.1f; // 电压突变阈值
    
    if(filteredValue == 0) {
        filteredValue = newValue;
        lastValue = newValue;
        return newValue;
    }
    
    // 检测电压突变
    if(fabs(newValue - lastValue) > jumpThreshold) {
        // 电压突变时减小滤波系数，加快响应
        filteredValue = 0.6f * newValue + 0.4f * filteredValue;
    } else {
        // 正常滤波
        filteredValue = alpha * newValue + (1-alpha) * filteredValue;
    }
    
    lastValue = newValue;
    return filteredValue;
}

// 电池电压校准
static float calibrateVoltage(float rawVoltage) {
    // 根据实测电压校准
    const float offset = -0.12f;  // 0 偏置校准
    const float scale = 1.05f;    // 1 比例校准

    float calibratedVoltage = rawVoltage * scale + offset;
    
    return calibratedVoltage;
}

// 定时器中断函数处理
void TIM3_IRQHandler(void)
{
    static u8 adcCount = 0;
    static float voltageSum = 0;
    const u8 ADC_AVERAGE_COUNT = 5;  // 每5次采样取平均

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        milliseconds++;

        if (milliseconds % 200 == 0) // 每200ms更新一次时间
        {
            update = true;
        }

        if (milliseconds % 2000 == 0) {  // 每2秒采样一次
            printf("adc start\n");
            // 这里导致卡死
            float rawVoltage = Get_Adc_Average(0, 50) * (3.3f / 4096.0f) * 2.0f;
            voltageSum += rawVoltage;
            adcCount++;
            
            if(adcCount >= ADC_AVERAGE_COUNT) {
                float avgVoltage = voltageSum / ADC_AVERAGE_COUNT;
                float filteredVoltage = batteryFilter(avgVoltage);
                BatteryVol = calibrateVoltage(filteredVoltage);
                printf("avgVoltage: %f, filteredVoltage: %f, BatteryVol: %f\n", avgVoltage, filteredVoltage, BatteryVol);
                
                voltageSum = 0;
                adcCount = 0;
            }
        }
    }

    TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update); // 清除中断标志位
}
