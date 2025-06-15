#include "mpu6050.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "common.h"
#include "nvic.h"
#include "oled_driver.h"
//////////////////////////////////////////////////////////////////////////////////
// ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
// ALIENTEK��ӢSTM32������V3
// MPU6050 ��������
// ����ԭ��@ALIENTEK
// ������̳:www.openedv.com
// ��������:2015/1/17
// �汾��V1.0
// ��Ȩ���У�����ؾ���
// Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern bool DeepSleepFlag;

// �������û����! �� inv_mpu.c���ʼ��
// ��ʼ��MPU6050
// ����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Init(void)
{
    u8 res;

    // MPU_AD0_CTRL=0;			//����MPU6050��AD0��Ϊ�͵�ƽ,�ӻ���ַΪ:0X68
    I2C_GPIO_Config();
    // MPU_IIC_Init();//��ʼ��IIC����
    u8 noOk = MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X80); // ��λMPU6050
    if (noOk)
    {
        return noOk;
    }
    delay_ms(100);
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X00); // ����MPU6050
    MPU_Set_Gyro_Fsr(3);                     // �����Ǵ�����,��2000dps
    MPU_Set_Accel_Fsr(0);                    // ���ٶȴ�����,��2g
    MPU_Set_Rate(50);                        // ���ò�����50Hz
    MPU_Write_Byte(MPU_INT_EN_REG, 0X00);    // �ر������ж�
    MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // I2C��ģʽ�ر�
    MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00);   // �ر�FIFO
    MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT���ŵ͵�ƽ��Ч
    res = MPU_Read_Byte(MPU_DEVICE_ID_REG);

    printf("addr %d\n", res);

    if (res == MPU_ADDR)
    {                                            // ����ID��ȷ
        MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X01); // ����CLKSEL,PLL X��Ϊ�ο�
        MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X00); // ���ٶ��������Ƕ�����
        MPU_Set_Rate(50);                        // ���ò�����Ϊ50Hz
    }
    else
    {
        return 1;
    }

    return 0;
}
// ����MPU6050�����Ǵ����������̷�Χ
// fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
// ����ֵ:0,���óɹ�
//     ����,����ʧ��
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3); // ���������������̷�Χ
}
// ����MPU6050���ٶȴ����������̷�Χ
// fsr:0,��2g;1,��4g;2,��8g;3,��16g
// ����ֵ:0,���óɹ�
//     ����,����ʧ��
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3); // ���ü��ٶȴ����������̷�Χ
}
// ����MPU6050�����ֵ�ͨ�˲���
// lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
// ����ֵ:0,���óɹ�
//     ����,����ʧ��
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

    return MPU_Write_Byte(MPU_CFG_REG, data); // �������ֵ�ͨ�˲���
}
// ����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
// rate:4~1000(Hz)
// ����ֵ:0,���óɹ�
//     ����,����ʧ��
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
    data = MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data); // �������ֵ�ͨ�˲���
    return MPU_Set_LPF(rate / 2);                     // �Զ�����LPFΪ�����ʵ�һ��
}

// �õ��¶�ֵ
// ����ֵ:�¶�ֵ(������100��)
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
// �õ�������ֵ(ԭʼֵ)
// gx,gy,gz:������x,y,z���ԭʼ����(������)
// ����ֵ:0,�ɹ�
//     ����,�������
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
// �õ����ٶ�ֵ(ԭʼֵ)
// gx,gy,gz:������x,y,z���ԭʼ����(������)
// ����ֵ:0,�ɹ�
//     ����,�������
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
// IIC����д
// addr:������ַ
// reg:�Ĵ�����ַ
// len:д�볤��
// buf:������
// ����ֵ:0,����
//     ����,�������
u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    // printf("MPU_Write_Len: %d %d %d", addr, reg, len);
    u8 i;
    I2C_Start();
    I2C_SendByte((addr << 1) | 0); // ����������ַ+д����

    if (MPU_I2C_WaitAck())
    { // �ȴ�Ӧ��
        I2C_Stop();
        printf("MPU_Write_Len error1");
        return 1;
    }

    I2C_SendByte(reg); // д�Ĵ�����ַ
    MPU_I2C_WaitAck(); // �ȴ�Ӧ��

    for (i = 0; i < len; i++)
    {
        I2C_SendByte(buf[i]); // ��������

        if (MPU_I2C_WaitAck())
        { // �ȴ�ACK
            I2C_Stop();
            printf("MPU_Write_Len error2");
            return 1;
        }
    }

    I2C_Stop();
    return 0;
}
// IIC������
// addr:������ַ
// reg:Ҫ��ȡ�ļĴ�����ַ
// len:Ҫ��ȡ�ĳ���
// buf:��ȡ�������ݴ洢��
// ����ֵ:0,����
//     ����,�������
u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    I2C_Start();
    I2C_SendByte((addr << 1) | 0); // ����������ַ+д����

    if (MPU_I2C_WaitAck())
    { // �ȴ�Ӧ��
        I2C_Stop();
        return 1;
    }

    I2C_SendByte(reg); // д�Ĵ�����ַ
    MPU_I2C_WaitAck(); // �ȴ�Ӧ��
    I2C_Start();
    I2C_SendByte((addr << 1) | 1); // ����������ַ+������
    MPU_I2C_WaitAck();             // �ȴ�Ӧ��

    while (len)
    {
        if (len == 1)
        {
            *buf = I2C_ReceiveByte(); // ������,����nACK
            I2C_NoAck();
        }
        else
        {
            *buf = I2C_ReceiveByte(); // ������,����ACK
            I2C_Ack();
        }

        len--;
        buf++;
    }

    I2C_Stop(); // ����һ��ֹͣ����
    return 0;
}
// IICдһ���ֽ�
// reg:�Ĵ�����ַ
// data:����
// ����ֵ:0,����
//     ����,�������
u8 MPU_Write_Byte(u8 reg, u8 data)
{
    if (!I2C_Start())
    {
        return 99;
    }
    I2C_SendByte((MPU_ADDR << 1) | 0); // ����������ַ+д����

    if (MPU_I2C_WaitAck())
    { // �ȴ�Ӧ��
        I2C_Stop();
        printf("\nMPU_I2C_WaitAck error\n");
        return 200;
    }

    I2C_SendByte(reg);  // д�Ĵ�����ַ
    MPU_I2C_WaitAck();  // �ȴ�Ӧ��
    I2C_SendByte(data); // ��������

    if (MPU_I2C_WaitAck())
    { // �ȴ�ACK
        I2C_Stop();
        printf("MPU_I2C_WaitAck() error");
        return 101;
    }

    I2C_Stop();
    return 0;
}
// IIC��һ���ֽ�
// reg:�Ĵ�����ַ
// ����ֵ:����������
u8 MPU_Read_Byte(u8 reg)
{
    u8 res;
    I2C_Start();
    I2C_SendByte((MPU_ADDR << 1) | 0); // ����������ַ+д����
    MPU_I2C_WaitAck();                 // �ȴ�Ӧ��
    I2C_SendByte(reg);                 // д�Ĵ�����ַ
    MPU_I2C_WaitAck();                 // �ȴ�Ӧ��
    I2C_Start();
    I2C_SendByte((MPU_ADDR << 1) | 1); // ����������ַ+������
    MPU_I2C_WaitAck();                 // �ȴ�Ӧ��
    res = I2C_ReceiveByte();
    I2C_NoAck(); // ��ȡ����,����nACK
    I2C_Stop();  /// ����һ��ֹͣ����
    return res;
}

/*
* roll����x��
* pitch����y��
* yaw����z��
��ΪPCB��MPU����ת��, ����x -> y, y -> x��

    |
    |
 -------x roll
    |
    |
    y pitch
*/

// ��Y�����һζ��ķ�����middle�Ա�
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

// �����෴��
// ��X�����һζ��ķ�����middle�Ա�
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

    pitch_a = roll_a; // ��x��

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
    
    // ��ȡ���ٶȺ�����������
    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0 && MPU_Get_Gyroscope(&gx, &gy, &gz) == 0)
    {
        // ������ٶȱ仯
        float acc_magnitude = sqrt(ax*ax + ay*ay + az*az);
        
        // ���������
        // 1. ���ٶȱ仯������ֵ����ʾ���˶���
        // 2. ���������ݳ�����ֵ����ʾ����ת��
        if (acc_magnitude > 15000 ||  // ���ٶȱ仯��ֵ
            abs(gx) > 1000 ||         // �����Ǳ仯
            abs(gy) > 1000 ||         // ����Ǳ仯
            abs(gz) > 1000)           // ƫ���Ǳ仯
        {
            delay_ms(100);  // ������ʱ�������Ӧ�ٶ�
            
            // �ٴ�ȷ���˶�
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

// �ж�����

void Free_Fall_Interrupt(void) // ���������ж�
{
    MPU_Write_Byte(MPU6050_RA_FF_THR, 0x01); // ����������ֵ
    MPU_Write_Byte(MPU6050_RA_FF_DUR, 0x01); // ����������ʱ��20ms ��λ1ms �Ĵ���0X20
}
void Motion_Interrupt(void) // �˶��ж�
{
    // MPU_Write_Byte(MPU6050_RA_MOT_THR, 0x25); // ���ü��ٶ���ֵΪ74mg, ����
    MPU_Write_Byte(MPU6050_RA_MOT_THR, 0x05); // �˶���ֵ I2C_Write(GYRO_ADDRESS,MOT_THR,0x25);  0x01 ��ζ����ֵΪ 1mg������һ���ǳ����е�����
    MPU_Write_Byte(MPU6050_RA_MOT_DUR, 0x14); // ���ʱ��20ms ��λ1ms �Ĵ���0X20
}
void Zero_Motion_Interrupt(void) // ��ֹ�ж�
{
    MPU_Write_Byte(MPU6050_RA_ZRMOT_THR, 0x20); // ��ֹ��ֵ
    MPU_Write_Byte(MPU6050_RA_ZRMOT_DUR, 0x20); // ��ֹ���ʱ��20ms ��λ1ms �Ĵ���0X20
}

void MPU_INT_Init(void)
{
    // �������塢�˶�����ֹ�ж� ��ѡһ
    // Free_Fall_Interrupt(); // ���������ж�
    Motion_Interrupt(); // �˶��ж�
    // Zero_Motion_Interrupt(); // ��ֹ�ж�

    MPU_Write_Byte(MPU_CFG_REG, 0x04);       // �����ⲿ���Ų�����DLPF���ֵ�ͨ�˲���
    MPU_Write_Byte(MPU_ACCEL_CFG_REG, 0x1C); // ���ٶȴ��������̺͸�ͨ�˲�������

    MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X1C);  // INT���ŵ͵�ƽƽʱ
    MPU_Write_Byte(MPU_INT_EN_REG, 0x40);   // �ж�ʹ�ܼĴ���

    // MPU_Write_Byte(MPU_INT_EN_REG, 0x80 | 0x40);  // ͬʱʹ���˶��;�ֹ����ж�

    // MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80);	// INT����0X80�͵�ƽ����
    // MPU_Write_Byte(MPU_INTBP_CFG_REG,0X9c);	// INT����0X9c�ߵ�ƽ����
    // MPU_Write_Byte(MPU_INT_EN_REG, 0X01); // ����FIFO�ж�

    /*
    #define MPU_RA_CONFIG           0x1A
    #define MPU_RA_ACCEL_CONFIG     0x1C
    #define MPU_RA_INT_PIN_CFG      0x37
    #define MPU_RA_INT_ENABLE       0x38

    MPU_Write_Byte(MPU_RA_CONFIG,0x04);           //�����ⲿ���Ų�����DLPF���ֵ�ͨ�˲���
    MPU_Write_Byte(MPU_RA_ACCEL_CONFIG,0x1C);     //���ٶȴ��������̺͸�ͨ�˲�������
    MPU_Write_Byte(MPU_RA_INT_PIN_CFG,0X1C);      //INT���ŵ͵�ƽƽʱ
    MPU_Write_Byte(MPU_RA_INT_ENABLE,0x40);       //�ж�ʹ�ܼĴ���
    */
   MPU_INT_Init_Config();
}

// int <--> PC13
// ���� PC13 Ϊ����ģʽ���������ڲ��������衣
// �� PC13 ӳ�䵽 EXTI13������Ϊ�����ش�����
void MPU_INT_Init_Config(void) {
    // ���� GPIOB ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // ���� PB11 Ϊ����ģʽ�������ڲ���������
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // ��������ģʽ
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // ���� SYSCFG ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // �� PB11 ӳ�䵽 EXTI11
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);

    // ���� EXTI11
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line13;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising; // �����ش���
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // ���� NVIC
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn; // EXTI11 ���ж�ͨ��
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void MPU_INT_Disable (void) {
    MPU_Write_Byte(MPU_INT_EN_REG, 0x00);   // �����ж�
}

bool MPU6050_WakeUpRequested; // �Ƿ�������, ��mpu6050���Ѻ������Ϊtrue
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        // ����жϱ�־
        EXTI_ClearITPendingBit(EXTI_Line13); // �������վͻ�һֱִ��, ����

        printf("mpu int\n");

        // u8 int_status = MPU_Read_Byte(MPU_INT_STATUS_REG);
        
        // if (int_status & 0x80) {  // �˶�����ж�
        //     // �����˶����
        //     printf("mpu int motion\n");
        // }
        // else if (int_status & 0x40) {  // ��ֹ����ж�
        //     // ����ֹ���
        //     printf("mpu int zero\n");
        // }

        if (DeepSleepFlag == 1) {
            // ����һ����־������ѭ���д���������
            MPU6050_WakeUpRequested = true;
            return;
            /*
            // ����ѭ���д���������
            // �Ȼָ�ʱ��, ��Ȼ��׼
            RCC_Configuration();
            if (MPU_movecheck()) {
                // ���������RCC_Configurationʱ�Ӿͻ����!!�ײ�!!
                // ����STOPģʽ��PLLͣ���ˣ����ԣ������ʼ��ʱ�����ã��õ���PLL����ô���Ѻ���Ҫ��������RCC��
                DeepSleepFlag = 0;
                OLED_ON();
                animation_start(display_load, ANIM_MOVE_OFF);
                
                printf("wake up my mpu6050\n");
            } else {
                // ���жϴ�������ֱ�ӽ���STOPģʽ�ǲ���ȫ��
                // ���½���STOPģʽ, �Ῠ��
                // DeepSleepFlag = 1;
                // OLED_OFF();
                // printf("mpu to stop mode\n");
                // PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // ����ͣ��ģʽ ����͹���ģʽ
            }
            */
        }
    }
}