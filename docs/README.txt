
# 文件结构

USER/GOWATCH.uvprojx 这个文件可以修改Group, 头文件, 其它的GOWATCH.xxx文件都是自动生成的, 不要修改
Group不能嵌套

# POWER_ON
PA1

main.c
void power_pin_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;        //	 PA1 POWER 控制端口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 速度
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_1); // 设为高电平, 开机
}

my_menu.c
// 关机
void ShutDown(void)
{
    // display_startCRTAnim(CRTANIM_CLOSE);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

# 
PA1=TIM2_CH2
TIM2_CH1用于beep播放音乐, 导致TIM2_CH2也受影响, 播放时可能会拉低PA1, 关机. 不会!! 只配置了TIM2_CH1
对TIM2_CH2没有影响, 感觉是电压不稳定导致
播放音乐, mpu都会导致重启

排查到是电池管理芯片TP4057有问题, 电压不稳定

# OLED

PB15 SCL
PB14 SDA
oled_driver.h
oled_driver.c

# OLED 1.3 & 0.96

oled_driver.h

#if defined(SSD1306) 0.96
#define __SET_COL_START_ADDR() 	do { \
        ssd1306_write_byte(0x00, SSD1306_CMD); \
        ssd1306_write_byte(0x10, SSD1306_CMD); \
    } while(false)
#elif defined(SH1106) 1.3
#define __SET_COL_START_ADDR() 	do { \
        ssd1306_write_byte(0x02, SSD1306_CMD); \
        ssd1306_write_byte(0x10, SSD1306_CMD); \
    } while(false)
#endif


# key

PB1 上 右
PA7 中
PA3 下 左
buttons.h
buttons.c

改成

PB1 上 右
PA5 中
PA4 下 左
buttons.h
buttons.c

nvic.c 按键中断也要配置

# 内部RTC与DS3231切换

config.h

#define RTC_SRC   ///在无外部时钟时间 请注销这里          这里有BUG ，需要配合修改millis.c里面的z中断计时软件

# DS3231, MPU6050 SCL2, SDA2
SCL2 PB8
SDA2 PB9

在 i2c_soft.h中改成

SCL2 PA7
SDA2 PB0

# font

english.h 默认的

# mpu6050

mpu/mpu6050.h

//如果AD0脚(9脚)接地,IIC地址为0X68(不包含最低位).
//如果接V3.3,则IIC地址为0X69(不包含最低位).
//注意！！！！！！！！！！！！
//默认0x68时会与DS3231的0xD0,0XD1冲突故选择AD0上拉至VCC
#define MPU_ADDR				0X69// INV_MPU.c内的0x68已经均改为0x69

外接要改成 0x68

# mpu方向设置 inv_mpu.v

//陀螺仪方向设置 默认
/*
https://blog.csdn.net/BinHeon/article/details/110925168
--------------->y
     |
     |
     |
    \ /
     x
     Z正方向屏幕向外

static signed char gyro_orientation[9] = { 1, 0, 0,
                                           0, 1, 0,
                                           0, 0, 1};
                                           
变成
------------>x
   |
   y
*/

static signed char gyro_orientation[9] = { 
    0, 1, 0,
    1, 0, 0,
    0, 0, 1
};

                                           
变成
x<-----
   |
   y
*/

static signed char gyro_orientation[9] = { 
    0, -1, 0,
    1, 0, 0,
    0, 0, 1
};


变成
    y
   |
   |
------------>x

*/

static signed char gyro_orientation[9] = { 
    0, 1, 0,
    -1, 0, 0,
    0, 0, 1
};