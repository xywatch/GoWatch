#include "i2c_soft.h"
#include "delay.h"

void I2C_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable GPIO clocks
    RCC_AHBPeriphClockCmd(SCL_RCC_CLOCK | SDA_RCC_CLOCK, ENABLE);

    // Configure SCL pin
    GPIO_InitStructure.GPIO_Pin = SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(SCL_PORT, &GPIO_InitStructure);

    // Configure SDA pin
    GPIO_InitStructure.GPIO_Pin = SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(SDA_PORT, &GPIO_InitStructure);
}

void I2C_SDA_IN()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(SDA_PORT, &GPIO_InitStructure);
}

void I2C_SDA_OUT()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(SDA_PORT, &GPIO_InitStructure);
}

/**************************************************************************
 * 函数名: void I2C_delay(void)
 * 描述  : 短暂延时
 * 输入  : 无
 * 输出  : 无
 * 说明  : 内部定义的i可以优化速度，经测试最低到5还能写入
 ***************************************************************************/
static void I2C_delay(void)
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
int I2C_Start(void)
{
    I2C_SDA_OUT(); // sda线输出
    SDA_H;
    SCL_H;
    I2C_delay();

    if (!SDA_read)
    {
        return 0; // SDA线为低电平则总线忙,退出
    }

    SDA_L;
    I2C_delay();

    if (SDA_read)
    {
        return 0; // SDA线为高电平则总线出错,退出
    }

    SDA_L;
    I2C_delay();
    return 1;
}
/**************************************************************************
 * 函数名: I2C_Stop(void)
 * 描述  : 终止信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
void I2C_Stop(void)
{
    I2C_SDA_OUT(); // sda线输出
    SCL_L;
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SDA_H;
    I2C_delay();
}
/**************************************************************************
 * 函数名: void I2C_Ack(void)
 * 描述  : 应答信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
void I2C_Ack(void)
{
    SCL_L;
    I2C_SDA_OUT(); // SDA设置为输出

    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}
/**************************************************************************
 * 函数名: void I2C_NoAck(void)
 * 描述  : 无应答信号
 * 输入  : 无
 * 输出  : 无
 * 说明  :
 ***************************************************************************/
void I2C_NoAck(void)
{
    SCL_L;
    I2C_SDA_OUT(); // SDA设置为输出
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}
/**************************************************************************
* 函数名: u8 I2C_WaitAck(void)
* 描述  : 等待应答信号
* 输入  : 无
* 输出  : TRUE : 有应答
           FALSE : 无应答
* 说明  :
***************************************************************************/
int I2C_WaitAck(void)
{
    u8 ucErrTime = 0;
    I2C_SDA_IN(); // SDA设置为输入
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();

    while (SDA_read)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            I2C_Stop();
            return 0;
        }
    }

    SCL_L;
    return 1;
}

// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
int MPU_I2C_WaitAck(void)
{
    u8 ucErrTime = 0;
    I2C_SDA_IN(); // SDA设置为输入
    // SCL_L;  I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();

    while (SDA_read)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            I2C_Stop();
            return 1;
        }
    }

    SCL_L;
    return 0;
}
/**************************************************************************
 * 函数名: void I2C_SendByte(u8 SendByte)
 * 描述  : 发送一个字节
 * 输入  : SendByte : 字节数据
 * 输出  : 无
 * 说明  : 数据从高位到低位
 ***************************************************************************/
void I2C_SendByte(u8 SendByte)
{
    u8 i = 8;
    I2C_SDA_OUT(); // SDA设置为输出

    while (i--)
    {
        SCL_L;
        I2C_delay();

        if (SendByte & 0x80)
        {
            SDA_H;
        }
        else
        {
            SDA_L;
        }

        SendByte <<= 1;
        I2C_delay();

        SCL_H;
        I2C_delay();
    }

    SCL_L;
}
/**************************************************************************
 * 函数名: u8 I2C_ReceiveByte(void)
 * 描述  : 读取一个字节
 * 输入  : 无
 * 输出  : 字节数据
 * 说明  : ReceiveByte : 数据从高位到低位
 ***************************************************************************/
u8 I2C_ReceiveByte(void)
{
    u8 i = 8;
    u8 ReceiveByte = 0;
    I2C_SDA_IN(); // SDA设置为输入

    SDA_H;

    while (i--)
    {
        ReceiveByte <<= 1;
        SCL_L;
        I2C_delay();
        SCL_H;
        I2C_delay();

        if (SDA_read)
        {
            ReceiveByte |= 0x01;
        }
    }

    SCL_L;
    return ReceiveByte;
}

// ZRX
// 单字节写入*******************************************

int Single_Write(unsigned char SlaveAddress, unsigned char REG_Address, unsigned char REG_data) // void
{
    I2C_SDA_OUT(); // SDA设置为输出

    if (!I2C_Start())
    {
        return 0;
    }

    I2C_SendByte(SlaveAddress); // 发送设备地址+写信号//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//设置高起始地址+器件地址

    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return 0;
    }

    I2C_SendByte(REG_Address); // 设置低起始地址
    I2C_WaitAck();
    I2C_SendByte(REG_data);
    I2C_WaitAck();
    I2C_Stop();
    delay_ms(5);
    return 1;
}

// 单字节读取*****************************************
unsigned char Single_Read(unsigned char SlaveAddress, unsigned char REG_Address)
{
    unsigned char REG_data;
    I2C_SDA_IN(); // SDA设置为输入

    if (!I2C_Start())
    {
        return 0;
    }

    I2C_SendByte(SlaveAddress); // I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址

    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return 0;
    }

    I2C_SendByte((u8)REG_Address); // 设置低起始地址
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(SlaveAddress + 1);
    I2C_WaitAck();

    REG_data = I2C_ReceiveByte();
    I2C_NoAck();
    I2C_Stop();
    // return TRUE;
    return REG_data;
}


/**
 * @brief 扫描 I2C 总线上的设备地址
 * @param 无
 * @return 无
 */
void I2C_ScanDevices(void) {
    printf("Starting I2C device scan...\r\n");

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        // 尝试发送设备地址（写模式）
        I2C_Start();
        I2C_SendByte(addr << 1);  // 转为8-bit写地址
        if (I2C_WaitAck() == 1) {
            printf("addr found: 0x%02X \r\n", addr); // 发现设备，打印地址
        }
        I2C_Stop();
    }
    printf("Scan complete.\r\n");
}