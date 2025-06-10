/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"
#include "my_menu.h"
#include "bme280.h"
#include "altitude_display.h"

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
#if COMPILE_ANIMATIONS
// static bool animateIcon(bool, byte*);
#endif
static display_t ticker(void);
static void drawTickerNum(tickerData_t *);

byte seconds = 0;

// watch face
void watchface_normal()
{
    display_setDrawFunc(draw);
    buttons_setFuncs(altitude_open2, menu_select, my_menu_open2);
    showMeThenRun(NULL); // 设置打开过度动画，（不执行函数）
}

extern float DS3231_Temp;
extern BME280_Data bme280_data;

static display_t draw()
{
#if COMPILE_ANIMATIONS
    //	static byte usbImagePos = FRAME_HEIGHT;
    //	static byte chargeImagePos = FRAME_HEIGHT;
#endif

    // console_log(50, "drawDate !");
    // Draw date
    drawDate();

    // Draw time animated
    display_t busy;

    busy = ticker();

    // Draw battery icon
    drawBattery();
    byte x = 35;

#if COMPILE_ANIMATIONS
    //	if(animateIcon(UDADDR != 0, &usbImagePos))
    //		if(animateIcon(1, &usbImagePos))

    //	{
    //		//draw_bitmap(x, usbImagePos, usbIcon, 16, 8, NOINVERT, 0);
    //		x += 20;
    //	}
    if (alarm_is_enabled())
    {
        draw_bitmap(x, FRAME_HEIGHT - 8, smallFontAlarm, 8, 8, NOINVERT, 0);

        /*
        alarm_s nextAlarm;

        // Make sure we're in 24 hour mode
        time_s time;
        time.hour = timeDate.time.hour;
        time.ampm = timeDate.time.ampm;
        time_timeMode(&time, TIMEMODE_24HR);
        alarm_getNext(&nextAlarm);
        if (alarm_dayEnabled(nextAlarm.days, timeDate.date.day)) {
            draw_bitmap(x + 8, FRAME_HEIGHT - 8, smallFontAlarm, 8, 8, NOINVERT, 0);
        } else {
            draw_bitmap(x + 15, FRAME_HEIGHT - 8, smallFontAlarm, 8, 8, NOINVERT, 0);
        }
        */
        /*
        char temp[8];
        sprintf((char *)temp, "%d", timeDate.date.day); //
        draw_string(temp, NOINVERT, FRAME_WIDTH - x + 8, FRAME_HEIGHT - 8);
        */

        x += 12;
    }

#else
    //	if(UDADDR != 0())
    //	{
    draw_bitmap(x, FRAME_HEIGHT - 9, usbIcon, 16, 8, NOINVERT, 0);
    x += 20;
    //	}
#endif

    // Draw charging icon
#if COMPILE_ANIMATIONS
    //	if(animateIcon(CHARGING(), &chargeImagePos))
    //		if(animateIcon(1, &chargeImagePos))

    //	{
    //		//draw_bitmap(x, chargeImagePos, chargeIcon, 8, 8, NOINVERT, 0);
    //		x += 12;
    //	}
#else
    //	if(CHARGING())
    {
        draw_bitmap(x, FRAME_HEIGHT - 9, chargeIcon, 8, 8, NOINVERT, 0);
        x += 12;
    }
#endif

#if COMPILE_STOPWATCH

    // Stopwatch icon
    if (stopwatch_active())
    {
        draw_bitmap(x, FRAME_HEIGHT - 8, stopwatch, 8, 8, NOINVERT, 0);
        x += 12;
    }

#endif

    // 显示温度
    char temp[8];
    sprintf((char *)temp, "%0.1fC", bme280_data.T); //
    draw_string(temp, NOINVERT, FRAME_WIDTH - 35, FRAME_HEIGHT - 8);

    // BMP280没有湿度
    sprintf((char *)temp, "%0.0f%%", bme280_data.H); // 湿度
    draw_string(temp, NOINVERT, FRAME_WIDTH - 60, FRAME_HEIGHT - 8);

    return busy;
}

static void drawDate()
{
    /*
    // 为了能正常显示
    time_s time = {seconds, 12, 18, 'A'};
    date_s date = {DAY_THU, 12, MONTH_OCT, 22};

    timeDate.time = time;
    timeDate.date = date;
    */

    /*
    char buff2[BUFFSIZE_DATE_FORMAT];
    // sprintf_P(buff2, PSTR(DATE_FORMAT), 1, 12, 2, 22);
    sprintf_P(buff2, PSTR(DATE_FORMAT), timeDate.date.day, timeDate.date.date, month, timeDate.date.year)
    draw_string(buff2, false, 12, 0);
    return;
    */

    // console_log(50, "year=%d\n", timeDate.date.year);
    // console_log(50, "month=%d-day=%d\n", timeDate.date.month, timeDate.date.day);
    // console_log(50, "DS3231 Init %d", BCD2HEX(DS3231_RD_Byte(0x06)));
    // return;
    /*
    if (timeDate.date.month >= 13) {
        console_log(50, "set default date\n");
        time_s time = {seconds, 12, 18, 'A'};
        date_s date = {DAY_THU, 12, MONTH_OCT, 22};

        timeDate_s timeDate2;
        timeDate2.time = time;
        timeDate2.date = date;

        time_set(&timeDate2);
    }
    */

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
/*
static void drawTickerNum(byte x, byte y, byte val, byte maxValue, bool moving, const byte* font, byte w, byte h, byte yPos)
{
    byte arraySize = (w * h) / 8;
    if(yPos == 255)
        yPos = 0;

    s_image img = newImage(x, y, &font[val * arraySize], w, h, WHITE, false, 0);
    draw_bitmap_set(&img);

    if(!moving || yPos == 0)
    {
        draw_bitmap_s2(&img);
        return;
    }

    byte prev = val - 1;
    if(prev == 255)
        prev = maxValue;

    img.offsetY = yPos - h - TICKER_GAP;
    draw_bitmap_s2(&img);

    img.offsetY = yPos;
    img.bitmap = &font[prev * arraySize];
    draw_bitmap_s2(&img);
}*/
