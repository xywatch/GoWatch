/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2014 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

#if COMPILE_GAME3

#define TUBE_COUNT 4

#define BLACK 0
#define WHITE 1

typedef struct
{
    byte x;
    byte y;
} tube_s;

static bool btnExit(void);
static bool btnBounce(void);
static bool btnPause(void);
static display_t draw(void);
void _DrawPoint(u8 x, u8 y, u8 t);
// static void judgeOverlap(void);//判断是否重叠并且绘制人物
// static const uint eepHighscore EEMEM;

int lives = 4;
byte _GRAM[64] = {0}; // 64x1bit  宽8 高度64

// Storage Vars
static uint highscore;
static uint score;
static bool bounce;
static bool newHighscore;
static bool overlap;
static bool gameState = false;

static tube_s tubes[TUBE_COUNT];

static const byte tube_top[] PROGMEM = {
    0xFF,
    0xD5,
    0xAB,
    0xD5,
    0x83,
    0x81,
    0x83,
    0x81,
    0x83,
    0x81,
    0x83,
    0x81,
    0x83,
    0x81,
    0x83,
    0x81,
    0xD5,
    0xFF,
};

static const byte tube[] PROGMEM = {
    0xFF,
    0x55,
    0xAA,
    0x55,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xAA,
    0xFF,
};

static const byte thingImg[] PROGMEM = {
    0x40,
    0xF8,
    0xEC,
    0x2C,
    0x2C,
    0x38,
    0xF0,
    0x10,
    0xD0,
    0x30,
    0xE8,
    0x4C,
    0x4C,
    0x9C,
    0xF0,
    0x02,
    0x1F,
    0x37,
    0x34,
    0x34,
    0x1C,
    0x0F,
    0x08,
    0x0B,
    0x0C,
    0x17,
    0x32,
    0x32,
    0x39,
    0x0F,
};

void draw_bitmap_set(image_s *img)
{
    draw_bitmap(img->x, img->y, img->bitmap, img->width, img->height, img->invert, img->offsetY);
}

void draw_bitmap_s2(image_s *img)
{
    draw_bitmap(img->x, img->y, img->bitmap, img->width, img->height, img->invert, img->offsetY);
}

extern bool keep_on;
void game3_start()
{
    menu_close();

    srand(millis());
    keep_on = 1;
    display_setDrawFunc(draw);
    buttons_setFuncs(btnPause, btnExit, btnBounce);

    byte a = 0;

    LOOP(TUBE_COUNT, i)
    {
        tubes[i].x = a;
        a += 42;
    }

    // 显示信息初始化
    score = 0;
    newHighscore = false;
    overlap = false;
    gameState = true;
    lives = 4;
}

static bool btnExit()
{
    if (lives == 255)
    {
        game3_start();
    }
    else
    {
        exitMeThenRun(display_load);
        keep_on = 0;
    }

    return true;
}

static bool btnBounce()
{
    bounce = true;
    return true;
}

static bool btnPause()
{
    gameState = !gameState;
    return true;
}

static display_t draw() // 还差一部是无法判断人物和管道是否重叠
{
    static double y;
    static double yVel;
    static byte tubeOffset;
    static u8 bird_y, tube1_y, tube2_y;

    if (gameState)
    {
        // if (MPU_Pitch_Detect())
        // {
        //     bounce = true;
        // }

        if (bounce)
        {
            yVel = -1.2;
            bounce = false;
        }
        else if ((byte)y > 200)
        {
            yVel = 0;
            y = 0;
        }
        else if ((byte)y < FRAME_HEIGHT - 8)
        {
            if (yVel < 0)
            {
                yVel += 0.1;
            }
            else
            {
                yVel += 0.06;
            }
        }
        else
        {
            yVel = 0;
            y = FRAME_HEIGHT - 8;
        }

        y += yVel;
    }

    bird_y = y;

    image_s img = {16, y, thingImg, 7, 8, NOINVERT, 0};

    draw_bitmap_set(&img); // 绘制人物坐标

    char buff[6];

    if (lives != 255)
    {

        img.width = 16;
        img.height = 8;
        LOOP(TUBE_COUNT, i)
        {
            img.bitmap = tube;
            img.x = tubes[i].x;

            LOOP(2, h)
            { // 画上管道
                img.y = (tubes[i].y + (8 * h)) - 16;
                draw_bitmap_s2(&img);
            }

            LOOP(2, h)
            { // 画下管道
                img.y = (tubes[i].y + (8 * h) + 48 + 16) - 16;
                draw_bitmap_s2(&img);
            }

            if (gameState)
            {
                img.x -= 1;
            }

            img.width = 18;
            img.bitmap = tube_top; // 管道顶部方框
            img.y = tubes[i].y + (8 * 2) - 16;
            tube1_y = img.y + 5;
            draw_bitmap_s2(&img);
            img.y = (tubes[i].y + (8 * 2) + 48 + 16) - 16 - (8 * 3);
            tube2_y = img.y - 5;
            draw_bitmap_s2(&img);

            if (gameState)
            {
                tubes[i].x--;
            }

            if (tubes[i].x > 200 && tubes[i].x < 240)
            {
                tubes[i].y = rand() % 16;
                tubes[i].x = FRAME_WIDTH + 16;
            }

            if (gameState)
            {
                if (tubes[i].x == 16)
                {
                    if (bird_y <= tube1_y || bird_y >= tube2_y)
                    {
                        overlap = 1;
                        lives = 255;
                    }
                    else
                    {
                        score++;
                    }
                }
            }
        }

        tubeOffset++;

        if (tubeOffset > 25)
        {
            tubeOffset = 0;
        }

        if (gameState && overlap)
        {
            if (score > highscore)
            {
                highscore = score;
                newHighscore = true;
            }
            else
            {
                newHighscore = false;
            }

            gameState = 0;
        }

        // Draw score
        sprintf_P(buff, PSTR("%u"), score);
        // sprintf_P(buff, PSTR("%u"), tube2_y);
        draw_string(buff, false, FRAME_WIDTH - 25, 1);

        // Draw lives
        //		LOOP(lives, i)
        //			draw_bitmap(32 + (8*i), 1, livesImg, 7, 8, NOINVERT, 0);
    }
    else
    {
        // Draw end game stuff

        draw_string_P(PSTR(STR_GAMEOVER), false, 20, 0);
        draw_string_P(PSTR(STR_SCORE), false, 20, 16);
        draw_string_P(PSTR(STR_HIGHSCORE), false, 20, 32);

        if (newHighscore)
        {
            draw_string_P(PSTR(STR_NEWHIGHSCORE), false, 20, 48);
        }

        sprintf_P(buff, PSTR("%u"), score);
        draw_string(buff, false, 96, 16);

        sprintf_P(buff, PSTR("%u"), highscore);
        draw_string(buff, false, 96, 32);
    }

    return DISPLAY_BUSY;
}

// 画点
// x:0~63
// y:0~63
// t:1 填充 0,清空
void _DrawPoint(u8 x, u8 y, u8 t)
{
    // 对应 byte OLED_GRAM[512];	  // x= 64 ,y=8
    // 变换一哈形式
    u8 pos, bx, temp = 0;
    //	if(x>7||y>63)return;//超出范围了.
    pos = y / 8;
    bx = y % 8;
    temp = 1 << (bx);

    if (t)
    {
        _GRAM[8 * pos + x] |= temp; // 做到的效果是竖着存放 竖着为y ，横着为x
    }
    else
    {
        _GRAM[8 * pos + x] &= ~temp;
    }
}

// 判断是否重叠并且绘制人物
// static void judgeOverlap(void)
//{
//	int t,i;
////	for(t=0;t<8;t++)
////	{
////		for(i=0;i<8;i++)
////		{
////			 //头痛啊这个bug，改不动了
////		  if(((oledBuffer[20+t*128]>>i)&0x01) == ((_GRAM[t*8+6]>>i)&0x01))
////			{
////				overlap=true;
////			}
////		}
////	}
//
//	if(overlap)
//	{
//		overlap=false;
//				lives=255;
//
////				if(lives == 255)
//						{
//							// Check for new highscore
//							if(score > highscore)
//							{
//								highscore = score;
//								newHighscore = true;

//							}
//							else
//								newHighscore = false;

//
//							//led_flash(LED_RED, 250, 255);
////							buzzer_buzz(250, TONE_2KHZ, VOL_UI, PRIO_UI, NULL);
//						}
////						else
////						{
////							led_flash(LED_GREEN, 30, 255);
//////							buzzer_buzz(100, TONE_2KHZ, VOL_UI, PRIO_UI, NULL);
////						}
//	}

//}

#endif
