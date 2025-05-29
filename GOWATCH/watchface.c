/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

#define TIME_POS_X 0
#define TIME_POS_Y 17
#define TICKER_GAP 4

// 8 * 16 = 128

typedef struct
{
    byte x;
    byte y;
    const byte *bitmap;
    byte w;
    byte h;
    byte offsetY;
    byte val;
    byte maxVal;
    bool moving;
} tickerData_t;

static display_t draw(void);
static void drawDate(void);
static display_t ticker(void);
static void drawTickerNum(tickerData_t *);
static void drawBattery(void);

byte seconds = 0;

uint32_t stepCount;

void getBatteryAndOthers () {
    // VBAT = getBatteryVoltage();
    // stepCount = sensor.getCounter();
    // syncWeather();
}

// watch face
void watchface_normal()
{
    display_setDrawFunc(draw);
    // buttons_setFuncs(altitude_open2, menu_select, my_menu_open2);
    showMeThenRun(NULL); // 设置打开过度动画，（不执行函数）
}

static display_t draw()
{
    // console_log(50, "drawDate !");
    // Draw date
    drawDate();

    // Draw time animated
    display_t busy;

    busy = ticker();

    // Draw battery icon
    drawBattery();
    byte x = 35;

#if COMPILE_STOPWATCH
    // Stopwatch icon
    if (stopwatch_active())
    {
        draw_bitmap(x, FRAME_HEIGHT - 8, stopwatch, 8, 8, NOINVERT, 0);
        x += 12;
    }
#endif

    // Draw next alarm
    alarm_s nextAlarm;
    if(alarm_getNext(&nextAlarm))
    {
        time_s alarmTime;
        alarmTime.hour = nextAlarm.hour;
        alarmTime.mins = nextAlarm.min;
        alarmTime.ampm = CHAR_24;
        time_timeMode(&alarmTime, appConfig.timeMode);
        
        char buff[9];
        sprintf_P(buff, PSTR("%02hhu:%02hhu%c"), alarmTime.hour, alarmTime.mins, alarmTime.ampm);
        draw_string(buff, false, x, FRAME_HEIGHT - 8);

        x += (alarmTime.ampm == CHAR_24) ? 35 : 42;
        draw_bitmap(x, FRAME_HEIGHT - 8, dowImg[alarm_getNextDay()], 8, 8, NOINVERT, 0);
        x += 10;
    }

    // drawStep(x);

    return busy;
}

static void drawDate()
{
    // Get day string
    char day[BUFFSIZE_STR_DAYS];
    strcpy(day, days[timeDate.date.day]);

    // Get month string
    char month[BUFFSIZE_STR_MONTHS];
    strcpy(month, months[timeDate.date.month]);

    // Draw date
    char buff[BUFFSIZE_DATE_FORMAT];
    sprintf_P(buff, PSTR(DATE_FORMAT), day, timeDate.date.date, month, timeDate.date.year);
    draw_string(buff, false, 12, 0);
}

#if COMPILE_ANIMATIONS
// static bool animateIcon(bool active, byte* pos)
//{
//	byte y = *pos;
//	if(active || (!active && y < FRAME_HEIGHT))
//	{
//		if(active && y > FRAME_HEIGHT - 8)
//			y -= 1;
//		else if(!active && y < FRAME_HEIGHT)
//			y += 1;
//		*pos = y;
//		return true;
//	}
//	return false;
// }
#endif

static display_t ticker()
{
    static byte yPos;
    static byte yPos_secs;
    static bool moving = false;
    static bool moving2[5];

    /*
    if(milliseconds % 3600 > 1800) {
        seconds++;
        seconds = seconds % 60;
        timeDate.time.secs = seconds;
    }
    */

#if COMPILE_ANIMATIONS
    static byte hour2;
    static byte mins;
    static byte secs;

    if (appConfig.animations)
    {
        if (timeDate.time.secs != secs)
        {
            yPos = 0;
            yPos_secs = 0;
            moving = true;

            moving2[0] = div10(timeDate.time.hour) != div10(hour2);
            moving2[1] = mod10(timeDate.time.hour) != mod10(hour2);
            moving2[2] = div10(timeDate.time.mins) != div10(mins);
            moving2[3] = mod10(timeDate.time.mins) != mod10(mins);
            moving2[4] = div10(timeDate.time.secs) != div10(secs);

            // moving2[3] = 1;

            // memcpy(&timeDateLast, &timeDate, sizeof(timeDate_s));
            hour2 = timeDate.time.hour;
            mins = timeDate.time.mins;
            secs = timeDate.time.secs;
        }

        // 之前height是 16 24
        // 改成了 16 32
        // TICKER_GAP 有什么用? 秒和时针的结束动画错开, 时晚4
        if (moving)
        {
            if (yPos <= 3)
            {
                yPos++;
            }
            else if (yPos <= 6)
            {
                yPos += 3;
            }
            else if (yPos <= 16)
            {
                yPos += 5;
            }
            else if (yPos <= MIDFONT_NUM_HEIGHT - 3)
            { // 22 前快后慢
                yPos += 3;
            }
            else if (yPos <= MIDFONT_NUM_HEIGHT + TICKER_GAP)
            { // 24 + TICKER_GAP, 如果还用24会卡顿
                yPos++;
            }

            if (yPos >= MIDFONT_NUM_HEIGHT + TICKER_GAP)
            {
                yPos = 255;
            }

            if (yPos_secs <= 1)
            {
                yPos_secs++;
            }
            else if (yPos_secs <= SMALLFONT_NUM_HEIGHT - 3)
            {
                yPos_secs += 3;
            }
            else if (yPos_secs <= SMALLFONT_NUM_HEIGHT + TICKER_GAP)
            {
                yPos_secs++;
            }

            if (yPos_secs >= SMALLFONT_NUM_HEIGHT + TICKER_GAP)
            {
                yPos_secs = 255;
            }

            if (yPos_secs > SMALLFONT_NUM_HEIGHT + TICKER_GAP && yPos > MIDFONT_NUM_HEIGHT + TICKER_GAP)
            {
                yPos = 0;
                yPos_secs = 0;
                moving = false;
                memset(moving2, false, sizeof(moving2));
            }
        }
    }
    else
#endif
    {
        yPos = 0;
        yPos_secs = 0;
        moving = false;
        memset(moving2, false, sizeof(moving2));
    }

    tickerData_t data;

    // Set new font data for hours and minutes
    data.y = TIME_POS_Y;
    // data.w = MIDFONT_WIDTH;
    // data.h = MIDFONT_HEIGHT;
    data.w = MIDFONT_NUM_WIDTH;
    data.h = MIDFONT_NUM_HEIGHT;
    // data.bitmap = (const byte*)&midFont;
    data.bitmap = (const byte *)&numFont16x32;
    data.offsetY = yPos;

    // Hours
    data.x = TIME_POS_X;
    data.val = div10(timeDate.time.hour);
    data.maxVal = 2;
    data.moving = moving2[0];
    drawTickerNum(&data);

    data.x += 16;
    data.val = mod10(timeDate.time.hour);
    data.maxVal = 9;
    data.moving = moving2[1];
    drawTickerNum(&data);

    data.x += 16;

    // Draw colon for half a second   画半秒的冒号
    // if(milliseconds % 3600 > 1800) { // 假装是半秒钟  30ms
    // draw_bitmap(TIME_POS_X + 46 + 2, TIME_POS_Y, colon, FONT_COLON_WIDTH, FONT_COLON_HEIGHT, NOINVERT, 0);
    draw_bitmap(data.x, TIME_POS_Y, numFont16x32[10], MIDFONT_NUM_WIDTH, MIDFONT_NUM_HEIGHT, NOINVERT, 0);
    //}

    // Minutes
    data.x += 16;
    data.val = div10(timeDate.time.mins);
    data.maxVal = 5;
    data.moving = moving2[2];
    drawTickerNum(&data);

    data.x += 16;
    data.val = mod10(timeDate.time.mins);
    data.maxVal = 9;
    data.moving = moving2[3];
    drawTickerNum(&data);
    data.x += 16;

    // Seconds
    data.y += 16;

    // if(milliseconds % 3600 > 1800) { // 假装是半秒钟  30ms
    if (milliseconds % 1000 >= 500)
    { // 0.5s 1秒分成两断, 后半秒显示, 前半秒隐藏
        // draw_bitmap(TIME_POS_X + 46 + 2, TIME_POS_Y, colon, FONT_COLON_WIDTH, FONT_COLON_HEIGHT, NOINVERT, 0);
        draw_bitmap(data.x, data.y, numFont16x16[10], SMALLFONT_NUM_WIDTH, SMALLFONT_NUM_HEIGHT, NOINVERT, 0);
    }

    data.x += 16;

    // data.bitmap = (const byte*)&small2Font;
    data.bitmap = (const byte *)&numFont16x16;
    // data.w = FONT_SMALL2_WIDTH;
    // data.h = FONT_SMALL2_HEIGHT;
    data.w = SMALLFONT_NUM_WIDTH;
    data.h = SMALLFONT_NUM_HEIGHT;
    data.offsetY = yPos_secs;
    data.val = div10(timeDate.time.secs);
    data.maxVal = 5;
    data.moving = moving2[4];
    drawTickerNum(&data);

    data.x += 16;
    data.val = mod10(timeDate.time.secs);
    data.maxVal = 9;
    data.moving = moving;
    drawTickerNum(&data);

    // Draw AM/PM character
    char tmp[2];
    tmp[0] = timeDate.time.ampm;
    tmp[1] = 0x00;
    draw_string(tmp, false, 104, 20);

    //	char buff[12];
    //	sprintf_P(buff, PSTR("%lu"), time_getTimestamp());
    //	draw_string(buff, false, 30, 50);

    return (moving ? DISPLAY_BUSY : DISPLAY_DONE);
}

static void drawTickerNum(tickerData_t *data)
{
    byte arraySize = (data->w * data->h) / 8;
    byte yPos = data->offsetY;
    const byte *bitmap = &data->bitmap[data->val * arraySize];
    byte x = data->x;
    byte y = data->y;

    if (!data->moving || yPos == 0 || yPos == 255)
    {
        draw_bitmap(x, y, bitmap, data->w, data->h, NOINVERT, 0);
    }
    else
    {
        byte prev = data->val - 1;

        if (prev == 255)
        {
            prev = data->maxVal;
        }

        draw_bitmap(x, y, bitmap, data->w, data->h, NOINVERT, yPos - data->h - TICKER_GAP);
        draw_bitmap(x, y, &data->bitmap[prev * arraySize], data->w, data->h, NOINVERT, yPos);
    }
}

float BatteryVol;

// 电池电量百分比计算
static int calcBatteryPercentage(float voltage) {
    // 锂电池电压范围
    const float VBAT_MAX = 4.2f;  // 满电电压
    const float VBAT_MIN = 3.2f;  // 最低电压
    const float VBAT_FULL = 4.15f; // 认为完全充满的电压
    
    // 电压限幅
    if(voltage > VBAT_MAX) voltage = VBAT_MAX;
    if(voltage < VBAT_MIN) voltage = VBAT_MIN;
    
    // 非线性映射计算电量百分比
    float percentage;
    if(voltage >= VBAT_FULL) {
        percentage = 100.0f;
    }
    else if(voltage >= 3.9f) { // 4.15V-3.9V 映射到 100%-80%
        percentage = 80.0f + (voltage - 3.9f) * 20.0f / (VBAT_FULL - 3.9f);
    }
    else if(voltage >= 3.7f) { // 3.9V-3.7V 映射到 80%-55%
        percentage = 55.0f + (voltage - 3.7f) * 25.0f / 0.2f;
    }
    else if(voltage >= 3.5f) { // 3.7V-3.5V 映射到 55%-30%
        percentage = 30.0f + (voltage - 3.5f) * 25.0f / 0.2f;
    }
    else { // 3.5V-3.2V 映射到 30%-0%
        percentage = (voltage - VBAT_MIN) * 30.0f / 0.3f;
    }
    
    return (int)(percentage + 0.5f); // 四舍五入
}

static void drawBattery()
{
    int bat;
    char ad[5];
    const byte *battIcon;

    // 根据电量百分比选择电池图标
    bat = calcBatteryPercentage(BatteryVol);
    
    if (bat < 10) {
        battIcon = battIconEmpty;
    }
    else if (bat < 30) {
        battIcon = battIconLow;
    }
    else if (bat < 60) {
        battIcon = battIconHigh;
    }
    else if (bat < 90) {
        battIcon = battIconHigh;  // 可以添加更多图标
    }
    else {
        battIcon = battIconFull;
    }

    // 绘制电池图标
    draw_bitmap(0, FRAME_HEIGHT - 8, battIcon, 16, 8, NOINVERT, 0);

    if (bat >= 100) {
        bat = 99;
    }

    // 显示电量百分比
    sprintf((char *)ad, "%d", bat);
    draw_string(ad, NOINVERT, 18, FRAME_HEIGHT - 8);
}

static void drawStep(byte x) {
    char ad[5];
    // uint32_t stepCount = sensor.getCounter();
    sprintf((char *)ad, "%d", (int)stepCount);
    // draw_string(ad, NOINVERT, x, FRAME_HEIGHT - 8);

    // 靠右显示
    u8 len = 10;
    if (stepCount < 10) {
        len = 1;
    } else if (stepCount < 100) {
        len = 2;
    } else if (stepCount < 1000) {
        len = 3;
    } else if (stepCount < 10000) {
        len = 4;
    }
     else if (stepCount < 100000) {
        len = 5;
    }

    x = 127 - len * 7 - 10; // 每一个字5像素宽
    draw_bitmap(x, FRAME_HEIGHT - 8, step, 8, 8, NOINVERT, 0);
    x += 10;
    draw_string(ad, NOINVERT, x, FRAME_HEIGHT - 8);
}