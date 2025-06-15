#include "mpu6050.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "common.h"
#include "nvic.h"
#include "oled_driver.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK精英STM32开发板V3
// MPU6050 驱动代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2015/1/17
// 版本：V1.0
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2009-2019
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern bool DeepSleepFlag;

// 这个代码没用了! 在 inv_mpu.c里初始化
// 初始化MPU6050
// 返回值:0,成功
//    其他,错误代码
u8 MPU_Init(void)
{
    u8 res;

    // MPU_AD0_CTRL=0;			//控制MPU6050的AD0脚为低电平,从机地址为:0X68
    I2C_GPIO_Config();
    // MPU_IIC_Init();//初始化IIC总线
    u8 noOk = MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X80); // 复位MPU6050
    if (noOk)
    {
        return noOk;
    }
    delay_ms(100);
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X00); // 唤醒MPU6050
    MPU_Set_Gyro_Fsr(3);                     // 陀螺仪传感器,±2000dps
    MPU_Set_Accel_Fsr(0);                    // 加速度传感器,±2g
    MPU_Set_Rate(50);                        // 设置采样率50Hz
    MPU_Write_Byte(MPU_INT_EN_REG, 0X00);    // 关闭所有中断
    MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // I2C主模式关闭
    MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00);   // 关闭FIFO
    MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT引脚低电平有效
    res = MPU_Read_Byte(MPU_DEVICE_ID_REG);

    printf("addr %d\n", res);

    if (res == MPU_ADDR)
    {                                            // 器件ID正确
        MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X01); // 设置CLKSEL,PLL X轴为参考
        MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X00); // 加速度与陀螺仪都工作
        MPU_Set_Rate(50);                        // 设置采样率为50Hz
    }
    else
    {
        return 1;
    }

    return 0;
}
// 设置MPU6050陀螺仪传感器满量程范围
// fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3); // 设置陀螺仪满量程范围
}
// 设置MPU6050加速度传感器满量程范围
// fsr:0,±2g;1,±4g;2,±8g;3,±16g
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3); // 设置加速度传感器满量程范围
}
// 设置MPU6050的数字低通滤波器
// lpf:数字低通滤波频率(Hz)
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_LPF(u16 lpf)
{
    u8 data = 0;

    if (lpf >= 188)
    {
        data = 1;
    }
    else if (lpf >= 98)
    {
        data = 2;
    }
    else if (lpf >= 42)
    {
        data = 3;
    }
    else if (lpf >= 20)
    {
        data = 4;
    }
    else if (lpf >= 10)
    {
        data = 5;
    }
    else
    {
        data = 6;
    }

    return MPU_Write_Byte(MPU_CFG_REG, data); // 设置数字低通滤波器
}
// 设置MPU6050的采样率(假定Fs=1KHz)
// rate:4~1000(Hz)
// 返回值:0,设置成功
//     其他,设置失败
u8 MPU_Set_Rate(u16 rate)
{
    u8 data;

    if (rate > 1000)
    {
        rate = 1000;
    }

    if (rate < 4)
    {
        rate = 4;
    }

    data = 1000 / rate - 1;
    data = MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data); // 设置数字低通滤波器
    return MPU_Set_LPF(rate / 2);                     // 自动设置LPF为采样率的一半
}

// 得到温度值
// 返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
    u8 buf[2];
    short raw;
    float temp;
    MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
    raw = ((u16)buf[0] << 8) | buf[1];
    temp = 36.53 + ((double)raw) / 340;
    return temp * 100;
    ;
}
// 得到陀螺仪值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
u8 MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
    u8 buf[6], res;
    res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);

    if (res == 0)
    {
        *gx = ((u16)buf[0] << 8) | buf[1];
        *gy = ((u16)buf[2] << 8) | buf[3];
        *gz = ((u16)buf[4] << 8) | buf[5];
    }

    return res;
    ;
}
// 得到加速度值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
    u8 buf[6], res;
    res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);

    if (res == 0)
    {
        *ax = ((u16)buf[0] << 8) | buf[1];
        *ay = ((u16)buf[2] << 8) | buf[3];
        *az = ((u16)buf[4] << 8) | buf[5];
    }

    return res;
}
// IIC连续写
// addr:器件地址
// reg:寄存器地址
// len:写入长度
// buf:数据区
// 返回值:0,正常
//     其他,错误代码
u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    // printf("MPU_Write_Len: %d %d %d", addr, reg, len);
    u8 i;
    I2C_Start();
    I2C_SendByte((addr << 1) | 0); // 发送器件地址+写命令

    if (MPU_I2C_WaitAck())
    { // 等待应答
        I2C_Stop();
        printf("MPU_Write_Len error1");
        return 1;
    }

    I2C_SendByte(reg); // 写寄存器地址
    MPU_I2C_WaitAck(); // 等待应答

    for (i = 0; i < len; i++)
    {
        I2C_SendByte(buf[i]); // 发送数据

        if (MPU_I2C_WaitAck())
        { // 等待ACK
            I2C_Stop();
            printf("MPU_Write_Len error2");
            return 1;
        }
    }

    I2C_Stop();
    return 0;
}
// IIC连续读
// addr:器件地址
// reg:要读取的寄存器地址
// len:要读取的长度
// buf:读取到的数据存储区
// 返回值:0,正常
//     其他,错误代码
u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    I2C_Start();
    I2C_SendByte((addr << 1) | 0); // 发送器件地址+写命令

    if (MPU_I2C_WaitAck())
    { // 等待应答
        I2C_Stop();
        return 1;
    }

    I2C_SendByte(reg); // 写寄存器地址
    MPU_I2C_WaitAck(); // 等待应答
    I2C_Start();
    I2C_SendByte((addr << 1) | 1); // 发送器件地址+读命令
    MPU_I2C_WaitAck();             // 等待应答

    while (len)
    {
        if (len == 1)
        {
            *buf = I2C_ReceiveByte(); // 读数据,发送nACK
            I2C_NoAck();
        }
        else
        {
            *buf = I2C_ReceiveByte(); // 读数据,发送ACK
            I2C_Ack();
        }

        len--;
        buf++;
    }

    I2C_Stop(); // 产生一个停止条件
    return 0;
}
// IIC写一个字节
// reg:寄存器地址
// data:数据
// 返回值:0,正常
//     其他,错误代码
u8 MPU_Write_Byte(u8 reg, u8 data)
{
    if (!I2C_Start())
    {
        return 99;
    }
    I2C_SendByte((MPU_ADDR << 1) | 0); // 发送器件地址+写命令

    if (MPU_I2C_WaitAck())
    { // 等待应答
        I2C_Stop();
        printf("\nMPU_I2C_WaitAck error\n");
        return 200;
    }

    I2C_SendByte(reg);  // 写寄存器地址
    MPU_I2C_WaitAck();  // 等待应答
    I2C_SendByte(data); // 发送数据

    if (MPU_I2C_WaitAck())
    { // 等待ACK
        I2C_Stop();
        printf("MPU_I2C_WaitAck() error");
        return 101;
    }

    I2C_Stop();
    return 0;
}
// IIC读一个字节
// reg:寄存器地址
// 返回值:读到的数据
u8 MPU_Read_Byte(u8 reg)
{
    u8 res;
    I2C_Start();
    I2C_SendByte((MPU_ADDR << 1) | 0); // 发送器件地址+写命令
    MPU_I2C_WaitAck();                 // 等待应答
    I2C_SendByte(reg);                 // 写寄存器地址
    MPU_I2C_WaitAck();                 // 等待应答
    I2C_Start();
    I2C_SendByte((MPU_ADDR << 1) | 1); // 发送器件地址+读命令
    MPU_I2C_WaitAck();                 // 等待应答
    res = I2C_ReceiveByte();
    I2C_NoAck(); // 读取数据,发送nACK
    I2C_Stop();  /// 产生一个停止条件
    return res;
}

/*
* roll：绕x轴
* pitch：绕y轴
* yaw：绕z轴
因为PCB上MPU方向转了, 可能x -> y, y -> x了

    |
    |
 -------x roll
    |
    |
    y pitch
*/

// 绕Y轴左右晃动的幅度与middle对比
u8 MPU_Roll_Detect(float middle)
{
    float pitch_a, roll_a, yaw_a;

    mpu_dmp_get_data(&pitch_a, &roll_a, &yaw_a);

    if (pitch_a - middle < -20)
    {
        return 2;
    }
    else if (pitch_a - middle > 20)
    {
        return 1;
    }

    return 0;
}

// 方向相反的
// 绕X轴左右晃动的幅度与middle对比
u8 MPU_Roll_Detect2(float middle)
{
    float pitch_a, roll_a, yaw_a;

    mpu_dmp_get_data(&pitch_a, &roll_a, &yaw_a);

    if (roll_a - middle < -20)
    {
        return 2;
    }
    else if (roll_a - middle > 20)
    {
        return 1;
    }

    return 0;
}

u8 MPU_Pitch_Detect(void)
{
    short pitch_a, roll_a, yaw_a;

    static bool left_flag = 0;
    static bool right_flag = 0;

    MPU_Get_Gyroscope(&pitch_a, &roll_a, &yaw_a);

    if (pitch_a > 3000)
    {
        left_flag = 1;
        right_flag = 0;
    }
    else if (pitch_a < -3000)
    {
        right_flag = 1;
        left_flag = 0;
    }
    else
    {
        if (left_flag && !right_flag)
        {
            left_flag = 0;
            return 1;
        }

        if (right_flag && !left_flag)
        {
            right_flag = 0;
            return 2;
        }

        left_flag = 0;
        right_flag = 0;
    }

    return 0;
}

u8 MPU_Pitch_Detect2(void)
{
    short pitch_a, roll_a, yaw_a;

    static bool left_flag = 0;
    static bool right_flag = 0;

    MPU_Get_Gyroscope(&pitch_a, &roll_a, &yaw_a);

    pitch_a = roll_a; // 用x的

    if (pitch_a > 3000)
    {
        left_flag = 1;
        right_flag = 0;
    }
    else if (pitch_a < -3000)
    {
        right_flag = 1;
        left_flag = 0;
    }
    else
    {
        if (left_flag && !right_flag)
        {
            left_flag = 0;
            return 1;
        }

        if (right_flag && !left_flag)
        {
            right_flag = 0;
            return 2;
        }

        left_flag = 0;
        right_flag = 0;
    }

    return 0;
}

/*

u8 MPU_Roll_Detect(float middle)
{
    float pitch_a, roll_a, yaw_a;

    mpu_dmp_get_data(&pitch_a, &roll_a, &yaw_a);

    if(pitch_a - middle < -20) {
        return 2;
    } else if(pitch_a - middle > 20) {
        return 1;
    }

    return 0;
}

u8 MPU_Pitch_Detect(void)
{
    short pitch_a, roll_a, yaw_a;

    static bool left_flag = 0;
    static bool right_flag = 0;

    MPU_Get_Gyroscope(&pitch_a, &roll_a, &yaw_a);

    if(pitch_a > 3000) {
        left_flag = 1;
        right_flag = 0;
    } else if(pitch_a < -3000) {
        right_flag = 1;
        left_flag = 0;
    } else {
        if(left_flag && !right_flag) {
            left_flag = 0;
            return 1;
        }

        if(right_flag && !left_flag) {
            right_flag = 0;
            return 2;
        }

        left_flag = 0;
        right_flag = 0;
    }

    return 0;
}

*/

short mpu_pitch_a, mpu_roll_a, mpu_yaw_a;
/*
u8 MPU_movecheck(void)
{
    if (MPU_Get_Gyroscope(&mpu_pitch_a, &mpu_roll_a, &mpu_yaw_a) == 0)
    {
        if (mpu_pitch_a > 2300 || mpu_pitch_a < -2300)
        {
            delay_ms(300);

            while (MPU_Get_Gyroscope(&mpu_pitch_a, &mpu_roll_a, &mpu_yaw_a))
            {
            };

            if (mpu_pitch_a > 2300 || mpu_pitch_a < -2300)
            {
                return true;
            }
        }
    }

    return false;
}
*/

u8 MPU_movecheck(void)
{
    short ax, ay, az;
    short gx, gy, gz;
    
    // 获取加速度和陀螺仪数据
    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0 && MPU_Get_Gyroscope(&gx, &gy, &gz) == 0)
    {
        // 计算加速度变化
        float acc_magnitude = sqrt(ax*ax + ay*ay + az*az);
        
        // 检测条件：
        // 1. 加速度变化超过阈值（表示有运动）
        // 2. 陀螺仪数据超过阈值（表示有旋转）
        if (acc_magnitude > 15000 ||  // 加速度变化阈值
            abs(gx) > 1000 ||         // 俯仰角变化
            abs(gy) > 1000 ||         // 横滚角变化
            abs(gz) > 1000)           // 偏航角变化
        {
            delay_ms(100);  // 缩短延时，提高响应速度
            
            // 再次确认运动
            if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0 && MPU_Get_Gyroscope(&gx, &gy, &gz) == 0)
            {
                acc_magnitude = sqrt(ax*ax + ay*ay + az*az);
                if (acc_magnitude > 15000 || 
                    abs(gx) > 1000 || 
                    abs(gy) > 1000 || 
                    abs(gz) > 1000)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

// 中断配置

void Free_Fall_Interrupt(void) // 自由落体中断
{
    MPU_Write_Byte(MPU6050_RA_FF_THR, 0x01); // 自由落体阈值
    MPU_Write_Byte(MPU6050_RA_FF_DUR, 0x01); // 自由落体检测时间20ms 单位1ms 寄存器0X20
}
void Motion_Interrupt(void) // 运动中断
{
    // MPU_Write_Byte(MPU6050_RA_MOT_THR, 0x25); // 设置加速度阈值为74mg, 不行
    MPU_Write_Byte(MPU6050_RA_MOT_THR, 0x05); // 运动阈值 I2C_Write(GYRO_ADDRESS,MOT_THR,0x25);  0x01 意味着阈值为 1mg，这是一个非常敏感的设置
    MPU_Write_Byte(MPU6050_RA_MOT_DUR, 0x14); // 检测时间20ms 单位1ms 寄存器0X20
}
void Zero_Motion_Interrupt(void) // 静止中断
{
    MPU_Write_Byte(MPU6050_RA_ZRMOT_THR, 0x20); // 静止阈值
    MPU_Write_Byte(MPU6050_RA_ZRMOT_DUR, 0x20); // 静止检测时间20ms 单位1ms 寄存器0X20
}

void MPU_INT_Init(void)
{
    // 自由落体、运动、静止中断 三选一
    // Free_Fall_Interrupt(); // 自由落体中断
    Motion_Interrupt(); // 运动中断
    // Zero_Motion_Interrupt(); // 静止中断

    MPU_Write_Byte(MPU_CFG_REG, 0x04);       // 配置外部引脚采样和DLPF数字低通滤波器
    MPU_Write_Byte(MPU_ACCEL_CFG_REG, 0x1C); // 加速度传感器量程和高通滤波器配置

    MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X1C);  // INT引脚低电平平时
    MPU_Write_Byte(MPU_INT_EN_REG, 0x40);   // 中断使能寄存器

    // MPU_Write_Byte(MPU_INT_EN_REG, 0x80 | 0x40);  // 同时使能运动和静止检测中断

    // MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80);	// INT引脚0X80低电平触发
    // MPU_Write_Byte(MPU_INTBP_CFG_REG,0X9c);	// INT引脚0X9c高电平触发
    // MPU_Write_Byte(MPU_INT_EN_REG, 0X01); // 开启FIFO中断

    /*
    #define MPU_RA_CONFIG           0x1A
    #define MPU_RA_ACCEL_CONFIG     0x1C
    #define MPU_RA_INT_PIN_CFG      0x37
    #define MPU_RA_INT_ENABLE       0x38

    MPU_Write_Byte(MPU_RA_CONFIG,0x04);           //配置外部引脚采样和DLPF数字低通滤波器
    MPU_Write_Byte(MPU_RA_ACCEL_CONFIG,0x1C);     //加速度传感器量程和高通滤波器配置
    MPU_Write_Byte(MPU_RA_INT_PIN_CFG,0X1C);      //INT引脚低电平平时
    MPU_Write_Byte(MPU_RA_INT_ENABLE,0x40);       //中断使能寄存器
    */
   MPU_INT_Init_Config();
}

// int <--> PC13
// 配置 PC13 为输入模式，并启用内部上拉电阻。
// 将 PC13 映射到 EXTI13，配置为上升沿触发。
void MPU_INT_Init_Config(void) {
    // 启用 GPIOB 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // 配置 PB11 为输入模式，启用内部上拉电阻
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 输入上拉模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 启用 SYSCFG 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // 将 PB11 映射到 EXTI11
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);

    // 配置 EXTI11
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line13;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // 上升沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 配置 NVIC
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; // EXTI11 的中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void MPU_INT_Disable (void) {
    MPU_Write_Byte(MPU_INT_EN_REG, 0x00);   // 禁用中断
}

bool MPU6050_WakeUpRequested; // 是否请求唤醒, 在mpu6050唤醒后会设置为true
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        // 清除中断标志
        EXTI_ClearITPendingBit(EXTI_Line13); // 如果不清空就会一直执行, 卡死

        printf("mpu int\n");

        // u8 int_status = MPU_Read_Byte(MPU_INT_STATUS_REG);
        
        // if (int_status & 0x80) {  // 运动检测中断
        //     // 处理运动检测
        //     printf("mpu int motion\n");
        // }
        // else if (int_status & 0x40) {  // 静止检测中断
        //     // 处理静止检测
        //     printf("mpu int zero\n");
        // }

        if (DeepSleepFlag == 1) {
            // 设置一个标志，在主循环中处理唤醒流程
            MPU6050_WakeUpRequested = true;
            return;
            /*
            // 在主循环中处理唤醒流程
            // 先恢复时钟, 不然不准
            RCC_Configuration();
            if (MPU_movecheck()) {
                // 如果不重新RCC_Configuration时钟就会很慢!!亲测!!
                // 进了STOP模式后，PLL停掉了，所以，如果开始的时钟配置，用的是PLL，那么唤醒后，需要重新配置RCC。
                DeepSleepFlag = 0;
                OLED_ON();
                animation_start(display_load, ANIM_MOVE_OFF);
                
                printf("wake up my mpu6050\n");
            } else {
                // 在中断处理函数中直接进入STOP模式是不安全的
                // 重新进入STOP模式, 会卡死
                // DeepSleepFlag = 1;
                // OLED_OFF();
                // printf("mpu to stop mode\n");
                // PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // 进入停机模式 进入低功耗模式
            }
            */
        }
    }
}