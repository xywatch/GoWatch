#include "i2c_soft2.h"
#include "delay.h"

// OLED i2c������

void I2C2_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(SCL2_RCC_CLOCK | SDA2_RCC_CLOCK, ENABLE);

    // ��ʼ��SCL�ܽ�
    GPIO_InitStructure.GPIO_Pin = SCL2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SCL2_PORT, &GPIO_InitStructure);

    // ��ʼ��SDA�ܽ�
    GPIO_InitStructure.GPIO_Pin = SDA2_PIN;
    GPIO_Init(SDA2_PORT, &GPIO_InitStructure);
}

void I2C2_SDA_IN()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SDA2_PORT, &GPIO_InitStructure);
}

void I2C2_SDA_OUT()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SDA2_PORT, &GPIO_InitStructure);
}

/**************************************************************************
 * ������: void I2C_delay(void)
 * ����  : ������ʱ
 * ����  : ��
 * ���  : ��
 * ˵��  : �ڲ������i�����Ż��ٶȣ���������͵�5����д��
 ***************************************************************************/
static void I2C2_delay(void)
{
    delay_us(3);
}
/**************************************************************************
 * ������: void I2C_Start(void)
 * ����  : ��ʼ�ź�
 * ����  : ��
 * ���  : ��
 * ˵��  :
 ***************************************************************************/
int I2C2_Start(void)
{
    I2C2_SDA_OUT(); // sda�����
    SDA2_H;
    SCL2_H;
    I2C2_delay();

    if (!SDA2_read)
    {
        return 0; // SDA��Ϊ�͵�ƽ������æ,�˳�
    }

    SDA2_L;
    I2C2_delay();

    if (SDA2_read)
    {
        return 0; // SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
    }

    SDA2_L;
    I2C2_delay();
    return 1;
}
/**************************************************************************
 * ������: I2C_Stop(void)
 * ����  : ��ֹ�ź�
 * ����  : ��
 * ���  : ��
 * ˵��  :
 ***************************************************************************/
void I2C2_Stop(void)
{
    I2C2_SDA_OUT(); // sda�����
    SCL2_L;
    SDA2_L;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();
    SDA2_H;
    I2C2_delay();
}
/**************************************************************************
 * ������: void I2C_Ack(void)
 * ����  : Ӧ���ź�
 * ����  : ��
 * ���  : ��
 * ˵��  :
 ***************************************************************************/
void I2C2_Ack(void)
{
    SCL2_L;
    I2C2_SDA_OUT(); // SDA����Ϊ���

    SDA2_L;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();
    SCL2_L;
    I2C2_delay();
}
/**************************************************************************
 * ������: void I2C_NoAck(void)
 * ����  : ��Ӧ���ź�
 * ����  : ��
 * ���  : ��
 * ˵��  :
 ***************************************************************************/
void I2C2_NoAck(void)
{
    SCL2_L;
    I2C2_SDA_OUT(); // SDA����Ϊ���
    I2C2_delay();
    SDA2_H;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();
    SCL2_L;
    I2C2_delay();
}
/**************************************************************************
* ������: u8 I2C_WaitAck(void)
* ����  : �ȴ�Ӧ���ź�
* ����  : ��
* ���  : TRUE : ��Ӧ��
           FALSE : ��Ӧ��
* ˵��  :
***************************************************************************/
int I2C2_WaitAck(void)
{
    u8 ucErrTime = 0;
    I2C2_SDA_IN(); // SDA����Ϊ����
    SCL2_L;
    I2C2_delay();
    SDA2_H;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();

    while (SDA2_read)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            I2C2_Stop();
            return 0;
        }
    }

    SCL2_L;
    return 1;
}


/**************************************************************************
 * ������: void I2C_SendByte(u8 SendByte)
 * ����  : ����һ���ֽ�
 * ����  : SendByte : �ֽ�����
 * ���  : ��
 * ˵��  : ���ݴӸ�λ����λ
 ***************************************************************************/
void I2C2_SendByte(u8 SendByte)
{
    u8 i = 8;
    I2C2_SDA_OUT(); // SDA����Ϊ���

    while (i--)
    {
        SCL2_L;
        I2C2_delay();

        if (SendByte & 0x80)
        {
            SDA2_H;
        }
        else
        {
            SDA2_L;
        }

        SendByte <<= 1;
        I2C2_delay();

        SCL2_H;
        I2C2_delay();
    }

    SCL2_L;
}


/**************************************************************************
 * ������: u8 I2C_ReceiveByte(void)
 * ����  : ��ȡһ���ֽ�
 * ����  : ��
 * ���  : �ֽ�����
 * ˵��  : ReceiveByte : ���ݴӸ�λ����λ
 ***************************************************************************/
u8 I2C2_ReceiveByte(void)
{
    u8 i = 8;
    u8 ReceiveByte = 0;
    I2C2_SDA_IN(); // SDA����Ϊ����

    SDA2_H;

    while (i--)
    {
        ReceiveByte <<= 1;
        SCL2_L;
        I2C2_delay();
        SCL2_H;
        I2C2_delay();

        if (SDA2_read)
        {
            ReceiveByte |= 0x01;
        }
    }

    SCL2_L;
    return ReceiveByte;
}

/**
 * @brief ɨ�� I2C �����ϵ��豸��ַ
 * @param ��
 * @return ��
 */
void I2C2_ScanDevices(void) {
    printf("2Starting I2C device scan...\r\n");

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        // ���Է����豸��ַ��дģʽ��
        I2C2_Start();
        I2C2_SendByte(addr << 1);  // תΪ8-bitд��ַ
        if (I2C2_WaitAck() == 1) {
            printf("2addr found: 0x%02X \r\n", addr); // �����豸����ӡ��ַ
        }
        I2C2_Stop();
    }
    printf("2Scan complete.\r\n");
}