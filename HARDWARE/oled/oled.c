#include "common.h"
#include "oled.h"

//-------------------------------------------------------------------------
/**************************************************************************/
// 坐标说明
/*														 x(0~127)
                                                 ------------------>
                                                |
                                                |
                                                |y(0~63)
                                                |
                                                |
                                                v

[x, y] => [x + y*128]
*/
u8 oledBuffer[FRAME_BUFFER_SIZE]; // 128*8

void OLED_Init(void)
{
    OLED_DriverInit();

    if (!appConfig.display180)
    {
        WriteCmd(0xA1);
        WriteCmd(0XC8);
    }
    else
    {
        WriteCmd(0xA0);
        WriteCmd(0xC0);
    }
}

// 清屏函数
// color:要清屏的填充色
void OLED_Clear(u16 color)
{
    // ClearScreen();
}

/*
void OLED_Flush(void)
{

    // OLED_FILL(oledBuffer);
    u8 i, j;
    unsigned char *p;
    p = oledBuffer;

    for (i = 0; i < 8; i++)
    {
        WriteCmd(0xb0 + i); // page0-page1

        __SET_COL_START_ADDR();

        // WriteCmd(0x02);		//low column start address
        // WriteCmd(0x10);

        for (j = 0; j < 128; j++)
        {
            if (appConfig.invert)
            {
                WriteDat(~(*p++));
            }
            else
            {
                WriteDat(*p++);
            }
        }
    }
}

*/

// 优化后的OLED_Flush函数
void OLED_Flush(void)
{
    // 使用水平寻址模式，减少寻址命令
    WriteCmd(0x20); // 设置寻址模式
    WriteCmd(0x00); // 水平寻址模式

    // 设置列地址范围
    WriteCmd(0x21); // 设置列地址
    WriteCmd(0x00); // 起始地址
    WriteCmd(0x7F); // 结束地址

    // 设置页地址范围
    WriteCmd(0x22); // 设置页地址
    WriteCmd(0x00); // 起始页
    WriteCmd(0x07); // 结束页

    // 批量发送数据
    const int BATCH_SIZE = 32; // 每次发送32字节
    uint8_t *p = oledBuffer;

    for (int i = 0; i < FRAME_BUFFER_SIZE; i += BATCH_SIZE)
    {
        int len = min(BATCH_SIZE, FRAME_BUFFER_SIZE - i);
        if (appConfig.invert)
        {
            uint8_t tempBuffer[BATCH_SIZE];
            for (int j = 0; j < len; j++)
            {
                tempBuffer[j] = ~p[j];
            }
            WriteDats(tempBuffer, len);
        }
        else
        {
            WriteDats(p, len);
        }
        p += len;
    }
}

void OLED_ClearScreenBuffer(void)
{
    memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
}
