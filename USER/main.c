#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "common.h"
#include "i2c_soft.h"
#include "adc.h"
#include "ds3231.h"
#include "rtc.h"
#include "nvic.h"
#include "bme280.h"
#include "oled_driver.h"

// extern unsigned char wifi_time_cnt;

void power_pin_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(POWER_ON_PORT == GPIOA ? RCC_APB2Periph_GPIOA : RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = POWER_ON_PIN;        //	 PA1 POWER 控制端口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 速度
    GPIO_Init(POWER_ON_PORT, &GPIO_InitStructure);
    GPIO_SetBits(POWER_ON_PORT, POWER_ON_PIN); // 设为高电平, 开机
}

extern bool bme_flag;
void bme_update(void)
{
    if (bme_flag)
    {
        bme_flag = 0;
        readTrim();
        bme280CompensateH();
        bme280CompensateP();
        bme280CompensateT();
    }
}

void showSpace()
{
    byte width = 52;
    byte height = 48;
    byte x = (128 - width) / 2;
    byte y = (64 - height) / 2;
    u16 delayMs = 100;
    byte count = 2; // 循环次数

    const u8 *space_images[] = {
        space_image1, space_image2, space_image3, space_image4, space_image5,
        space_image6, space_image7, space_image8, space_image9, space_image10};
  
    while (count--)
    {
      for (int i = 0; i < 10; i++)
      {
        draw_bitmap(x, y, space_images[i], width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);
      }
    }
}

extern short pitch_a, roll_a, yaw_a;
void c_setup()
{
    power_pin_init();
    // SystemInit();
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
    delay_init();                                   // 延时函数初始?
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置系统中断优先级分组2

    uart_init(115200);
    // printf("goxxx1");

    OLED_Init(); // 初始化OLED接口
    Adc_Init();  // ADC初始化
    KEY_INT_INIT(); // 按键中断
    millis_init();
    // printf("goxxx2");

    delay_ms(50);
    showSpace();

    console_log(50, "Init MPU...");
    // console_log(50, "SS %d", SystemCoreClock);

    char i = 0;
    int count = 0;

    i = MPU_Init(); // 不要用因为下面mpu_dmp_init会重新init

    while (i != 0 && i != 1 && count++ < 3)
    {
        printf("MPU Init %d\n", i);
        console_log(50, "MPU Init %d", i);
        delay_ms(50);
        i = MPU_Init(); // 不要用因为下面mpu_dmp_init会重新init
    }

    I2C_GPIO_Config();
    i = mpu_dmp_init();
    printf("mpu_dmp_init %d\n", i);
    count = 0;

    while ((++count) < 10 && i)
    {
        // i = MPU_Init(); // 不要用因为下面mpu_dmp_init会重新init
        // printf("MPU_Init %d\n", i);
        //  console_log(1, "MPU Init:%d", i);
        console_log(50, "MPU DUM Error %d", i);
        delay_ms(50);
        i = mpu_dmp_init();
        printf("mpu_dmp_init error %d\n", i);
    }

    if (!i)
    {
        console_log(50, "Init MPU OK");
    }
    else
    {
        console_log(50, "Init MPU Error");
    }
    MPU_INT_Init();

    milliseconds = 0;

#ifndef RTC_SRC
    // 先初始化中断引脚
    DS3231_INT_INIT();  // 初始化DS3231中断引脚
    delay_ms(10);       // 等待引脚配置稳定
    
    // 然后初始化 DS3231
    DS3231_Init();      // DS3231初始化
    delay_ms(10);       // 等待初始化完成

    console_log(50, "DS3231 Init OK");

    // time_init();
    // Date_init();
    // ds3231只用电池供电, 所以必须要焊接电池
    // 如果ds3231有问题, 就会一直卡在Start! 可能5v导致ds3231死机, 此时需要电池下电
    DS3231_Get_Time();
    if (timeDate.time.secs == 0 && timeDate.time.mins == 0 && timeDate.time.hour == 0) {
        console_log(50, "Set default date");
        // 那不是每次都重置了rtc时间
        time_s time = {0, 39, 10, 'A'};
        date_s date = {DAY_THU, 27, MONTH_FEB, 25};
        timeDate.time = time;
        timeDate.date = date;
        DS3231_Set_Date();
        DS3231_Set_Time();
    }
#endif

#ifdef RTC_SRC
    count = 0;
    while (count++ < 500)
    {
        u8 rtcInitRet = RTC_Init();

        if (!rtcInitRet)
        {
            console_log(50, "RTC Init OK");
            break;
        }
        else
        {
            console_log(50, "RTC Init Error: %d", rtcInitRet);
        }
    }
#endif

    appconfig_init();
    // led_init();              // 初始化LED
    buzzer_init();
    buttons_init();

    Bme280_Init();
    console_log(50, "BME Init OK");

    // global_init();
    alarm_init(); // 无法储存闹钟，每次重启以后需要自定义

    pwrmgr_init();
    console_log(50, "START !");

    // memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
    OLED_ClearScreenBuffer();

    // Set watchface
    display_set(watchface_normal);
    display_load(); // 启动表盘
}

bool bme_enable = 1; // 先暂时不要

extern bool MPU6050_WakeUpRequested;
extern bool DeepSleepFlag;
extern bool SleepRequested;
void c_loop()
{
    // mpu6050 唤醒
    if (MPU6050_WakeUpRequested) {
        // 1. 先恢复时钟
        RCC_Configuration();
        delay_ms(10);  // 等待时钟稳定
        
        // 2. 再检查运动
        if (MPU_movecheck()) {
            nvic_wake_up(50);
            SleepRequested = false; // 避免在mpu6050唤醒后处理休眠请求
        }
        else {
            printf("mpu6050 not move\n");
            // 如果mpu6050没有移动, 则会立即执行下面的进入STOP模式
        }
        
        MPU6050_WakeUpRequested = false;
    }

    // printf("SleepRequested %d, MPU6050_WakeUpRequested %d, DeepSleepFlag %d\n", SleepRequested, MPU6050_WakeUpRequested, DeepSleepFlag);

    // mpu6050 int后虽然没有moveCheck成功, 但其实已经唤醒了, 此时DeepSleepFlag=1
    // mpu6050 唤醒后, 也会触发休眠请求, 所以不要在mpu6050唤醒后处理休眠请求
    if (SleepRequested && !MPU6050_WakeUpRequested) {
        SleepRequested = false;
        nvic_sleep(1);
    }

    time_update();

    // 这里会一直返回true, 除非sleep也就不会执行loop()了
    if (pwrmgr_userActive())
    {
        // battery_update();
        buttons_update();
    }

    // mpu_updata();

    buzzer_update();
    // led_update();

#if COMPILE_STOPWATCH
    stopwatch_update();
#endif
    //  global_update();

    if (pwrmgr_userActive())
    {
        alarm_update();

        // 刷新屏幕
        display_update();
    }

    if (bme_enable)
    {
        bme_update();
    }

    pwrmgr_update();

    // 显示完成后清除缓冲区
    // memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
    OLED_ClearScreenBuffer();
}

void c_setup2()
{
    delay_init();
    uart_init(115200);
    printf("goxxx\n");

    while (1)
    {
        printf("goxxx\n");
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); // 低
        GPIO_SetBits(GPIOB, GPIO_Pin_15);   // 高
        delay_ms(500);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15); // 低
        GPIO_SetBits(GPIOB, GPIO_Pin_14);   // 高
        delay_ms(500);
    }
}

int main(void)
{
    c_setup(); // 初始化

    while (1)
    {
        c_loop(); // 循环
    }
}
