
# �ļ��ṹ

USER/GOWATCH.uvprojx ����ļ������޸�Group, ͷ�ļ�, ������GOWATCH.xxx�ļ������Զ����ɵ�, ��Ҫ�޸�
Group����Ƕ��

# POWER_ON
PA1

main.c
void power_pin_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;        //	 PA1 POWER ���ƶ˿�
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // �ٶ�
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_1); // ��Ϊ�ߵ�ƽ, ����
}

my_menu.c
// �ػ�
void ShutDown(void)
{
    // display_startCRTAnim(CRTANIM_CLOSE);
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

# 
PA1=TIM2_CH2
TIM2_CH1����beep��������, ����TIM2_CH2Ҳ��Ӱ��, ����ʱ���ܻ�����PA1, �ػ�. ����!! ֻ������TIM2_CH1
��TIM2_CH2û��Ӱ��, �о��ǵ�ѹ���ȶ�����
��������, mpu���ᵼ������

�Ų鵽�ǵ�ع���оƬTP4057������, ��ѹ���ȶ�

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

PB1 �� ��
PA7 ��
PA3 �� ��
buttons.h
buttons.c

�ĳ�

PB1 �� ��
PA5 ��
PA4 �� ��
buttons.h
buttons.c

nvic.c �����ж�ҲҪ����

# �ڲ�RTC��DS3231�л�

config.h

#define RTC_SRC   ///�����ⲿʱ��ʱ�� ��ע������          ������BUG ����Ҫ����޸�millis.c�����z�жϼ�ʱ���

# DS3231, MPU6050 SCL2, SDA2
SCL2 PB8
SDA2 PB9

�� i2c_soft.h�иĳ�

SCL2 PA7
SDA2 PB0

# font

english.h Ĭ�ϵ�

# mpu6050

mpu/mpu6050.h

//���AD0��(9��)�ӵ�,IIC��ַΪ0X68(���������λ).
//�����V3.3,��IIC��ַΪ0X69(���������λ).
//ע�⣡����������������������
//Ĭ��0x68ʱ����DS3231��0xD0,0XD1��ͻ��ѡ��AD0������VCC
#define MPU_ADDR				0X69// INV_MPU.c�ڵ�0x68�Ѿ�����Ϊ0x69

���Ҫ�ĳ� 0x68

# mpu�������� inv_mpu.v

//�����Ƿ������� Ĭ��
/*
https://blog.csdn.net/BinHeon/article/details/110925168
--------------->y
     |
     |
     |
    \ /
     x
     Z��������Ļ����

static signed char gyro_orientation[9] = { 1, 0, 0,
                                           0, 1, 0,
                                           0, 0, 1};
                                           
���
------------>x
   |
   y
*/

static signed char gyro_orientation[9] = { 
    0, 1, 0,
    1, 0, 0,
    0, 0, 1
};

                                           
���
x<-----
   |
   y
*/

static signed char gyro_orientation[9] = { 
    0, -1, 0,
    1, 0, 0,
    0, 0, 1
};


���
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