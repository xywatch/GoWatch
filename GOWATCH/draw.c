/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

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
u8	oledBuffer[FRAME_BUFFER_SIZE]; // 128*8
*/

inline static void setBuffByte(byte *, byte, byte, byte); //, byte);
// #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
inline byte pgm_read_byte(const byte *abc)
{
    return *abc;
}

void draw_string_P(const char *string, bool invert, byte x, byte y)
{
    byte len = strlen(string);
    char buff[len + 1];
    strcpy(buff, string);
    draw_string(buff, invert, x, y);
}

void draw_string_center(char *string, bool invert, byte fromX, byte toX, byte y)
{
    byte charCount = 0;

    byte len = (strlen(string) - 1) * 7 + 5;
    byte width = toX - fromX;
    byte startX = (width - len) / 2;
    if (startX < 0)
    {
        startX = 0;
    }
    startX += fromX;

    while (*string)
    {
        char c = *string - 0x20;
        byte xx = startX + (charCount * 7); // 5x8 两个字隔了7, 让两个字隔远点
        draw_bitmap(xx, y, smallFont[(byte)c], SMALLFONT_WIDTH, SMALLFONT_HEIGHT, invert, 0);

        // 这里向左和右加了多余的空白
        if (invert)
        {
            if (xx > 0)
            {
                setBuffByte(oledBuffer, xx - 1, y, 0xFF); //, WHITE);
            }
            if (xx < FRAME_WIDTH - 5)
            {
                setBuffByte(oledBuffer, xx + 5, y, 0xFF); //, WHITE);
            }
        }

        string++;
        charCount++;
    }
}

void draw_string(char *string, bool invert, byte x, byte y)
{
    byte charCount = 0;

    while (*string)
    {
        char c = *string - 0x20;
        byte xx = x + (charCount * 7); // 5x8 两个字隔了7, 让两个字隔远点
        draw_bitmap(xx, y, smallFont[(byte)c], SMALLFONT_WIDTH, SMALLFONT_HEIGHT, invert, 0);

        // 这里向左和右加了多余的空白
        if (invert)
        {
            if (xx > 0)
            {
                setBuffByte(oledBuffer, xx - 1, y, 0xFF); //, WHITE);
            }

            if (xx < FRAME_WIDTH - 5)
            {
                setBuffByte(oledBuffer, xx + 5, y, 0xFF); //, WHITE);
            }
        }

        string++;
        charCount++;
    }
}

void draw_string_min(char *string, bool invert, byte x, byte y)
{
    byte charCount = 0;

    while (*string)
    {
        char c = *string - 0x20;
        byte xx = x + (charCount * 5); // 紧凑
        draw_bitmap(xx, y, smallFont[(byte)c], SMALLFONT_WIDTH, SMALLFONT_HEIGHT, invert, 0);

        // if(invert) {
        //     if(xx > 0) {
        //         setBuffByte(oledBuffer, xx - 1, y, 0xFF);    //, WHITE);
        //     }

        //     if(xx < FRAME_WIDTH - 5) {
        //         setBuffByte(oledBuffer, xx + 5, y, 0xFF);    //, WHITE);
        //     }
        // }

        string++;
        charCount++;
    }
}

// 2维转1维
inline static uint flatXY(byte x, byte y)
{
    uint pos = x + (y / 8) * FRAME_WIDTH;
    return pos;
}

inline static void setBuffByte(byte *buff, byte x, byte y, byte val) //, byte colour)
{
    // uint pos = x + (y / 8) * FRAME_WIDTH;
    buff[flatXY(x, y)] |= val;
}

inline static byte readPixels(const byte *loc, bool invert)
{
    // byte pixels = ((int)loc & 0x8000 ? eeprom_read_byte((int)loc & ~0x8000) : pgm_read_byte(loc));
    byte pixels = pgm_read_byte(loc); // d读取flash里面的数据到ram

    if (invert)
    {
        pixels = ~pixels;
    }

    return pixels;
}

// Ultra fast bitmap drawing
// Only downside is that height must be a multiple of 8, otherwise it gets rounded down to the nearest multiple of 8
// Drawing bitmaps that are completely on-screen and have a Y co-ordinate that is a multiple of 8 results in best performance
// PS - Sorry about the poorly named variables ;_;
// 超快位图绘制
// 唯一的缺点是高度必须是8的倍数，否则将四舍五入到最接近8的倍数
// 画位图，完全在屏幕上，有一个Y坐标是8的倍数在最佳性能
// PS -不好意思，变量命名不好;
void draw_bitmap(byte x, byte yy, const byte *bitmap, byte w, byte h, bool invert, byte offsetY)
{
    // Apply animation offset
    yy += animation_offsetY();

    //
    byte y = yy - offsetY;

    //
    byte h2 = h / 8;

    //
    byte pixelOffset = (y % 8);

    byte thing3 = (yy + h);

    //
    LOOP(h2, hh)
    {
        //
        byte hhh = (hh * 8) + y; // Current Y pos (every 8 pixels)
        byte hhhh = hhh + 8;     // Y pos at end of pixel column (8 pixels)

        //
        if (offsetY && (hhhh < yy || hhhh > FRAME_HEIGHT || hhh > thing3))
        {
            continue;
        }

        //
        byte offsetMask = 0xFF;

        if (offsetY)
        {
            if (hhh < yy)
            {
                offsetMask = (0xFF << (yy - hhh));
            }
            else if (hhhh > thing3)
            {
                offsetMask = (0xFF >> (hhhh - thing3));
            }
        }

        uint aa = ((hhh / 8) * FRAME_WIDTH);

        const byte *b = bitmap + (hh * w);

        // If() outside of loop makes it faster (doesn't have to keep re-evaluating it)
        // Downside is code duplication
        if (!pixelOffset && hhh < FRAME_HEIGHT)
        {
            //
            LOOP(w, ww)
            {
                // Workout X co-ordinate in frame buffer to place next 8 pixels
                byte xx = ww + x;

                // Stop if X co-ordinate is outside the frame
                if (xx >= FRAME_WIDTH)
                {
                    continue;
                }

                // Read pixels
                byte pixels = readPixels(b + ww, invert) & offsetMask;

                oledBuffer[xx + aa] |= pixels;

                // setBuffByte(buff, xx, hhh, pixels, colour);
            }
        }
        else
        {
            uint aaa = ((hhhh / 8) * FRAME_WIDTH);

            //
            LOOP(w, ww)
            {
                // Workout X co-ordinate in frame buffer to place next 8 pixels
                byte xx = ww + x;

                // Stop if X co-ordinate is outside the frame
                if (xx >= FRAME_WIDTH)
                {
                    continue;
                }

                // Read pixels
                byte pixels = readPixels(b + ww, invert) & offsetMask;

                //
                if (hhh < FRAME_HEIGHT)
                {
                    oledBuffer[xx + aa] |= pixels << pixelOffset;
                }

                // setBuffByte(buff, xx, hhh, pixels << pixelOffset, colour);

                //
                if (hhhh < FRAME_HEIGHT)
                {
                    oledBuffer[xx + aaa] |= pixels >> (8 - pixelOffset);
                }

                // setBuffByte(buff, xx, hhhh, pixels >> (8 - pixelOffset), colour);
            }
        }
    }
}

// 画图
// draw_bitmap2(x0, y0, GliderGun_Map, 36, 9, 1); 高度是9 不能用draw_bitmap
void draw_bitmap2(byte x, byte y, const uint8_t *bitmap, byte w, byte h, byte color)
{
    int yOffset = y % 8;
    int sRow = y / 8;
    //   if (y < 0)
    //   {
    //     sRow--;
    //     yOffset = 8 - yOffset;
    //   }
    int rows = h / 8;
    if (h % 8 != 0)
        rows++;
    for (int a = 0; a < rows; a++)
    {
        int bRow = sRow + a;
        if (bRow > (FRAME_HEIGHT / 8) - 1)
            break;
        if (bRow > -2)
        {
            for (int iCol = 0; iCol < w; iCol++)
            {
                if (iCol + x > (FRAME_WIDTH - 1))
                    break;
                if (iCol + x >= 0)
                {
                    if (bRow >= 0)
                    {
                        if (color == 0) // white
                            oledBuffer[(bRow * FRAME_WIDTH) + x + iCol] |= (pgm_read_byte(bitmap + (a * w) + iCol) << yOffset);
                        else if (color == 1) // black
                            oledBuffer[(bRow * FRAME_WIDTH) + x + iCol] &= (pgm_read_byte(bitmap + (a * w) + iCol) << yOffset);
                        else
                            oledBuffer[(bRow * FRAME_WIDTH) + x + iCol] ^= pgm_read_byte(bitmap + (a * w) + iCol) << yOffset;
                    }
                    if (yOffset && bRow < (FRAME_HEIGHT / 8) - 1 && bRow > -2)
                    {
                        if (color == 0)
                            oledBuffer[((bRow + 1) * FRAME_WIDTH) + x + iCol] |= pgm_read_byte(bitmap + (a * w) + iCol) >> (8 - yOffset);
                        else if (color == 1)
                            oledBuffer[((bRow + 1) * FRAME_WIDTH) + x + iCol] &= ~(pgm_read_byte(bitmap + (a * w) + iCol) >> (8 - yOffset));
                        else
                            oledBuffer[((bRow + 1) * FRAME_WIDTH) + x + iCol] ^= pgm_read_byte(bitmap + (a * w) + iCol) >> (8 - yOffset);
                    }
                }
            }
        }
    }
}

// y must be a multiple of 8
// height is always 8
void draw_clearArea(byte x, byte y, byte w)
{
    uint pos = x + (y / 8) * FRAME_WIDTH;
    memset(&oledBuffer[pos], 0x00, w);
}

void draw_end()
{
    OLED_Flush(); // 刷新屏幕的意思
}

/**
 * @brief  Draws a piont on the screen
 *
 * @param  chXpos: Specifies the X position
 * @param  chYpos: Specifies the Y position
 * @param  chPoint: 0: the point turns off    1: the piont turns on
 *
 * @retval None
 **/

void draw_set_point(byte chXpos, byte chYpos, byte chPoint)
{
    byte bit_position, chTemp = 0;

    // Apply animation offset
    chYpos += animation_offsetY();

    if (chXpos >= FRAME_WIDTH || chYpos >= FRAME_HEIGHT)
    {
        return;
    }

    // 逆向, 低位在前
    // 0x5f=0101 1111 = !
    // 9%8=1
    // 00000001 << 6 = 01000000
    // 00000001 << 1 = 00000010
    bit_position = chYpos % 8;
    chTemp = 1u << bit_position;

    // 设置点
    if (chPoint)
    {
        oledBuffer[flatXY(chXpos, chYpos)] |= chTemp;
        // s_chDispalyBuffer[chXpos][chPos] |= chTemp;

        // 清除点
    }
    else
    {
        oledBuffer[flatXY(chXpos, chYpos)] &= ~chTemp;
        // s_chDispalyBuffer[chXpos][chPos] &= ~chTemp;
    }
}

// 获取像素是否有值
bool draw_get_point(byte chXpos, byte chYpos)
{
    byte bit_position, chTemp = 0;

    if (chXpos >= FRAME_WIDTH || chYpos >= FRAME_HEIGHT)
    {
        return 0;
    }

    bit_position = chYpos % 8;
    chTemp = 1u << bit_position;
    // 000010000
    return ((oledBuffer[flatXY(chXpos, chYpos)] & chTemp) >> bit_position) != 0;
}

/*

// OLED读点
u8 OLED_GetPixel(u8 x, u8 y)
{
  uint8_t row = y / 8;
  uint8_t bit_position = y % 8;
  return (oledBuffer[(row * FRAME_WIDTH) + (u8)x] & _BV(bit_position)) >> bit_position;
}
// OLED画点
void OLED_DrawPixel(u8 x, u8 y, u8 color)
{
  if (x > (FRAME_WIDTH - 1) || y > (FRAME_HEIGHT - 1))
  {
    return;
  }

  u8 row = (u8)y / 8;
  if (color)
  {
    oledBuffer[(row * FRAME_WIDTH) + (u8)x] |= _BV((u8)y % 8);
  }
  else
  {
    oledBuffer[(row * FRAME_WIDTH) + (u8)x] &= ~_BV((u8)y % 8);
  }
}
*/

/**
 * @brief  Fills a rectangle
 *
 * @param  chXpos1: Specifies the X position 1 (X top left position)
 * @param  chYpos1: Specifies the Y position 1 (Y top left position)
 * @param  chXpos2: Specifies the X position 2 (X bottom right position)
 * @param  chYpos3: Specifies the Y position 2 (Y bottom right position)
 *
 * @retval
 **/
void draw_fill_screen(byte chXpos1, byte chYpos1, byte chXpos2, byte chYpos2, byte chDot)
{
    byte chXpos, chYpos;

    for (chXpos = chXpos1; chXpos <= chXpos2; chXpos++)
    {
        for (chYpos = chYpos1; chYpos <= chYpos2; chYpos++)
        {
            draw_set_point(chXpos, chYpos, chDot);
        }
    }
}
