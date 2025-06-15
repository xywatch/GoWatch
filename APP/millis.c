#include "common.h"
#include "sys.h"
#include "led.h"
#include "adc.h"
#include "bme280.h"

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

extern char imu_run;
u8 HistoryCount = 0;
extern HistoryData historydat[12];
extern float DS3231_Temp;
extern int altitude;
bool bme_flag = 0;
u8 bme_time = 5;
u8 log_time = 1;
extern bool bme_enable;

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

    // const float offset = 0f;  // 0 偏置校准
    // const float scale = 1f;    // 1 比例校准

    float calibratedVoltage = rawVoltage * scale + offset;
    
    // 限制电压范围
    // if(calibratedVoltage > 4.22f) calibratedVoltage = 4.22f;
    // if(calibratedVoltage < 3.0f) calibratedVoltage = 3.0f;
    
    return calibratedVoltage;
}

// 定时器中断函数处理。 //TIM2通用定时器
void TIM3_IRQHandler(void)
{
    static u8 count = 0;
    static u8 adcCount = 0;
    static float voltageSum = 0;
    const u8 ADC_AVERAGE_COUNT = 5;  // 每5次采样取平均

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    { // 判断中断标志是否发生
        // wifi_wait_data_hander();
        milliseconds++;
        // update = true;

        if (milliseconds % 200 == 0)
        {
            update = true;
            //++timeDate.time.secs;   //每0.2秒钟更新一次时间;
        }

        if (milliseconds % (bme_time * 1000) == 0)
        {
            bme_flag = 1;
        }

        if (milliseconds % 2000 == 0) {  // 缩短采样间隔到2秒
            float rawVoltage = Get_Adc_Average(0, 50) * (3.3f / 4096.0f) * 2.0f;
            voltageSum += rawVoltage;
            adcCount++;
            
            if(adcCount >= ADC_AVERAGE_COUNT) {
                float avgVoltage = voltageSum / ADC_AVERAGE_COUNT;
                float filteredVoltage = batteryFilter(avgVoltage);
                BatteryVol = calibrateVoltage(filteredVoltage);
                // avgVoltage: 4.025420, filteredVoltage: 4.027704, BatteryVol: 4.109089
                // avgVoltage: 3.992226, filteredVoltage: 4.010188, BatteryVol: 4.090697
                // 校准后会比较大 BatteryVol > filteredVoltage
                printf("avgVoltage: %f, filteredVoltage: %f, BatteryVol: %f\n", avgVoltage, filteredVoltage, BatteryVol);
                
                voltageSum = 0;
                adcCount = 0;
            }

            if (bme_enable)
            {
                count++;
            }
        }

        if (count > 6 * log_time)
        { // 每10分钟更新一次数据
            count = 0;
            historydat[HistoryCount].hour = timeDate.time.hour;
            historydat[HistoryCount].min = timeDate.time.mins;
            historydat[HistoryCount].temp = bme280_data.T;  // DS3231_Temp ? 温度
            historydat[HistoryCount].shidu = bme280_data.H; // 湿度
            historydat[HistoryCount].height = altitude;

            if (++HistoryCount > 11)
            {
                HistoryCount = 0;
            }
        }

        if (milliseconds % 20 == 0) // 5 太小了, 会卡死时钟
        {
            // 有时会卡死, imu_run 在 mpu_task.c中, 卡死ds3231时钟
            if (imu_run)
            {
                u8 ret = mpu_dmp_get_data(&pitch, &roll, &yaw);
                printf("ret: %d, pitch: %.2f, roll: %.2f, yaw: %.2f\n", ret, pitch, roll, yaw);
            }
        }
    }

    TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update); // 手动清除中断标志位
}
