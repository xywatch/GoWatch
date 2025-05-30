#include "common.h"
#include "oled.h"

//-------------------------------------------------------------------------
/**************************************************************************/
// ����˵��
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

// ��������
// color:Ҫ���������ɫ
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

// �Ż����OLED_Flush����
void OLED_Flush(void)
{
    // ʹ��ˮƽѰַģʽ������Ѱַ����
    WriteCmd(0x20); // ����Ѱַģʽ
    WriteCmd(0x00); // ˮƽѰַģʽ

    // �����е�ַ��Χ
    WriteCmd(0x21); // �����е�ַ
    WriteCmd(0x00); // ��ʼ��ַ
    WriteCmd(0x7F); // ������ַ

    // ����ҳ��ַ��Χ
    WriteCmd(0x22); // ����ҳ��ַ
    WriteCmd(0x00); // ��ʼҳ
    WriteCmd(0x07); // ����ҳ

    // ������������
    const int BATCH_SIZE = 32; // ÿ�η���32�ֽ�
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
