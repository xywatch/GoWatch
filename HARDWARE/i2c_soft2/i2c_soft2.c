#include "i2c_soft2.h"
#include "delay.h"

// OLED i2c的引脚

void I2C2_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(SCL2_RCC_CLOCK | SDA2_RCC_CLOCK, ENABLE);

    // 初始化SCL管脚
    GPIO_InitStructure.GPIO_Pin = SCL2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SCL2_PORT, &GPIO_InitStructure);

    // 初始化SDA管脚
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
 * 函数名: void I2C_delay(void)
 * 描述  : 短暂延时
 * 输入  : 无
 * 输出  : 无
 * 说明  : 内部定义的i可以优化速度，经测试最低到5还能写入
 ***************************************************************************/
static void I2C2_delay(void)
{
    delay_us(3);
}
/**************************************************************************
 * 函数名: void I2C_Start(void)
 * 描述  : 起始信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
int I2C2_Start(void)
{
    I2C2_SDA_OUT(); // sda线输出
    SDA2_H;
    SCL2_H;
    I2C2_delay();

    if (!SDA2_read)
    {
        return 0; // SDA线为低电平则总线忙,退出
    }

    SDA2_L;
    I2C2_delay();

    if (SDA2_read)
    {
        return 0; // SDA线为高电平则总线出错,退出
    }

    SDA2_L;
    I2C2_delay();
    return 1;
}
/**************************************************************************
 * 函数名: I2C_Stop(void)
 * 描述  : 终止信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
void I2C2_Stop(void)
{
    I2C2_SDA_OUT(); // sda线输出
    SCL2_L;
    SDA2_L;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();
    SDA2_H;
    I2C2_delay();
}
/**************************************************************************
 * 函数名: void I2C_Ack(void)
 * 描述  : 应答信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
void I2C2_Ack(void)
{
    SCL2_L;
    I2C2_SDA_OUT(); // SDA设置为输出

    SDA2_L;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();
    SCL2_L;
    I2C2_delay();
}
/**************************************************************************
 * 函数名: void I2C_NoAck(void)
 * 描述  : 无应答信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
void I2C2_NoAck(void)
{
    SCL2_L;
    I2C2_SDA_OUT(); // SDA设置为输出
    I2C2_delay();
    SDA2_H;
    I2C2_delay();
    SCL2_H;
    I2C2_delay();
    SCL2_L;
    I2C2_delay();
}
/**************************************************************************
* 函数名: u8 I2C_WaitAck(void)
* 描述  : 等待应答信号
* 输入  : 无
* 输出  : TRUE : 有应答
           FALSE : 无应答
* 说明  :
***************************************************************************/
int I2C2_WaitAck(void)
{
    u8 ucErrTime = 0;
    I2C2_SDA_IN(); // SDA设置为输入
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
 * 函数名: void I2C_SendByte(u8 SendByte)
 * 描述  : 发送一个字节
 * 输入  : SendByte : 字节数据
 * 输出  : 无
 * 说明  : 数据从高位到低位
 ***************************************************************************/
void I2C2_SendByte(u8 SendByte)
{
    u8 i = 8;
    I2C2_SDA_OUT(); // SDA设置为输出

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
 * 函数名: u8 I2C_ReceiveByte(void)
 * 描述  : 读取一个字节
 * 输入  : 无
 * 输出  : 字节数据
 * 说明  : ReceiveByte : 数据从高位到低位
 ***************************************************************************/
u8 I2C2_ReceiveByte(void)
{
    u8 i = 8;
    u8 ReceiveByte = 0;
    I2C2_SDA_IN(); // SDA设置为输入

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
 * @brief 扫描 I2C 总线上的设备地址
 * @param 无
 * @return 无
 */
void I2C2_ScanDevices(void) {
    printf("2Starting I2C device scan...\r\n");

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        // 尝试发送设备地址（写模式）
        I2C2_Start();
        I2C2_SendByte(addr << 1);  // 转为8-bit写地址
        if (I2C2_WaitAck() == 1) {
            printf("2addr found: 0x%02X \r\n", addr); // 发现设备，打印地址
        }
        I2C2_Stop();
    }
    printf("2Scan complete.\r\n");
}