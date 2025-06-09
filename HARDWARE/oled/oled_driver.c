#include "oled_driver.h"
#include "sys.h"
#include "delay.h"

// PB15 SCL, PB14 SDA
void SW_IIC_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // Enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    
    // Configure I2C pins
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // Set initial state
    GPIO_SetBits(GPIOB, GPIO_Pin_15 | GPIO_Pin_14);
    delay_ms(500);
}

/**********************************************
//IIC Start
**********************************************/
void IIC_Start()
{

    OLED_SCLK_Set();
    OLED_SDIN_Set();
    OLED_SDIN_Clr();
    OLED_SCLK_Clr();
}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop()
{
    OLED_SCLK_Set();
    //	OLED_SCLK_Clr();
    OLED_SDIN_Clr();
    OLED_SDIN_Set();
}

void IIC_Wait_Ack()
{
    OLED_SCLK_Set();
    OLED_SCLK_Clr();
}

/**********************************************
// IIC Write byte
**********************************************/

void Write_IIC_Byte(unsigned char IIC_Byte)
{
    unsigned char i;
    unsigned char m, da;
    da = IIC_Byte;
    OLED_SCLK_Clr();

    for (i = 0; i < 8; i++)
    {
        m = da;
        //	OLED_SCLK_Clr();
        m = m & 0x80;

        if (m == 0x80)
        {
            OLED_SDIN_Set();
        }
        else
        {
            OLED_SDIN_Clr();
        }

        da = da << 1;
        OLED_SCLK_Set();
        OLED_SCLK_Clr();
    }
}

/**********************************************
// IIC Write Command
**********************************************/
void WriteCmd(unsigned char dat)
{
    IIC_Start();
    Write_IIC_Byte(0x78); // Slave address,SA0=0
    IIC_Wait_Ack();
    Write_IIC_Byte(0x00); // write command
    IIC_Wait_Ack();
    Write_IIC_Byte(dat);
    IIC_Wait_Ack();
    IIC_Stop();
}

void WriteDat(unsigned char dat)
{
    IIC_Start();
    Write_IIC_Byte(0x78); // D/C#=0; R/W#=0
    IIC_Wait_Ack();
    Write_IIC_Byte(0x40); // write data
    IIC_Wait_Ack();
    Write_IIC_Byte(dat);
    IIC_Wait_Ack();
    IIC_Stop();
}

void WriteDats(unsigned char *dat, uint8_t len)
{
    if (len == 0) return;

    IIC_Start();
    Write_IIC_Byte(0x78); // D/C#=0; R/W#=0
    IIC_Wait_Ack();
    Write_IIC_Byte(0x40); // write data
    IIC_Wait_Ack();

    // Write all data bytes in one transaction
    for (uint8_t i = 0; i < len; i++)
    {
        Write_IIC_Byte(dat[i]);
        IIC_Wait_Ack();
    }

    IIC_Stop();
}

void OLED_FILL(unsigned char BMP[])
{
    u8 i, j;
    unsigned char *p;
    p = BMP;

    for (i = 0; i < 8; i++)
    {
        WriteCmd(0xb0 + i); // page0-page1
        __SET_COL_START_ADDR();

        // WriteCmd(0x02);		//low column start address
        // WriteCmd(0x10);
        for (j = 0; j < 128; j++)
        {
            WriteDat(*p++);
        }
    }
}

void OLED_InitIt(void)
{
    WriteCmd(0xAE); // display off
    WriteCmd(0x20); // Set Memory Addressing Mode
    WriteCmd(0x10); // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    WriteCmd(0xb0); // Set Page Start Address for Page Addressing Mode,0-7
    WriteCmd(0xc8); // Set COM Output Scan Direction
    WriteCmd(0x00); //---set low column address
    WriteCmd(0x10); //---set high column address
    WriteCmd(0x40); //--set start line address
    WriteCmd(0x81); //--set contrast control register
    WriteCmd(0xff); // 亮度调节 0x00~0xff
    WriteCmd(0xa1); //--set segment re-map 0 to 127
    WriteCmd(0xa6); //--set normal display
    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
    WriteCmd(0x3F); //
    WriteCmd(0xa4); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    WriteCmd(0xd3); //-set display offset
    WriteCmd(0x00); //-not offset
    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
    WriteCmd(0xf0); //--set divide ratio
    WriteCmd(0xd9); //--set pre-charge period
    WriteCmd(0x22); //
    WriteCmd(0xda); //--set com pins hardware configuration
    WriteCmd(0x12);
    WriteCmd(0xdb); //--set vcomh
    WriteCmd(0x20); // 0x20,0.77xVcc
    WriteCmd(0x8d); //--set DC-DC enable
    WriteCmd(0x14); //
    WriteCmd(0xaf); //--turn on oled panel

    OLED_CLS();
}

void OLED_CLS(void) // 清屏
{
    unsigned char m, n;

    for (m = 0; m < 8; m++)
    {
        WriteCmd(0xb0 + m); // page0-page1
        WriteCmd(0x00);     // low column start address
        WriteCmd(0x10);     // high column start address

        for (n = 0; n < 128; n++)
        {
            WriteDat(0x00);
        }
    }
}

void OLED_ON(void)
{
    WriteCmd(0X8D); // 设置电荷泵
    WriteCmd(0X14); // 开启电荷泵
    WriteCmd(0XAF); // OLED唤醒
}

void OLED_OFF(void)
{
    WriteCmd(0X8D); // 设置电荷泵
    WriteCmd(0X10); // 关闭电荷泵
    WriteCmd(0XAE); // OLED休眠
}

// 初始化图形库，请将硬件初始化信息放入此中
void OLED_DriverInit(void)
{
    SW_IIC_Configuration();
    OLED_InitIt();
}
