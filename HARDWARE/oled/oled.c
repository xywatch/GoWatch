#include "oled.h"
#include "stdlib.h"
#include "usart.h"
#include "delay.h"
#include "oled_driver.h"
#include "util.h"
#include "common.h"

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

void OLED_ClearScreenBuffer(void)
{
    memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
}
