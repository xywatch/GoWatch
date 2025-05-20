/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

// #include "stmflash.h"

#define NOALARM UCHAR_MAX

static byte nextAlarm;
static byte nextAlarmDay; // 下一个闹钟的星期, 0-6, 0:周一, 6:周日
bool isAlarmTriggered; // 移除static，使其成为全局变量
static draw_f oldDrawFunc;
static button_f oldBtn1Func;
static button_f oldBtn2Func;
static button_f oldBtn3Func;

// 所有闹钟
alarm_s eepAlarms[ALARM_COUNT] EEMEM = {
    {10, 42, 255}, // 22:45:00, 127 = 1111111, 表示星期1,2,3,4,5,6,7, 255=1(开启) 111111(周二)1(周一), 表示所有星期且开启
    {10, 43, 255}, 
    {7, 45, 63},  // 63 = 111111, 表示星期1,2,3,4,5,6
    {9, 4, 0}, 
    {3, 1, 7} // 7 = 111, 表示星期1,2,3
};

static bool isAlarmTimeReached(void);
static void getNextAlarm(void);
static uint toMinutes(byte, byte, byte);
static bool stopAlarm(void);
static display_t draw(void);

bool AlarmEnalbe()
{
    u8 i;

    for (i = 0; i < ALARM_COUNT; i++)
    {
        if (eepAlarms[i].enabled == 1)
        {
            return 1;
        }
    }

    return 0;
}

void alarm_init()
{
    getNextAlarm();
}

void alarm_reset()
{
    // Set bytes individually, uses less flash space
    //??????
    memset(&eepAlarms, 0x00, ALARM_COUNT * sizeof(alarm_s));

    // alarm_s alarm;
    // memset(&alarm, 0, sizeof(alarm_s));
    // LOOPR(ALARM_COUNT, i)
    //	eeprom_update_block(&alarm, &eepAlarms[i], sizeof(alarm_s));
}

void alarm_get(byte num, alarm_s *alarm)
{
    //	alarm=&eepAlarms[num];
    memcpy(alarm, &eepAlarms[num], sizeof(alarm_s));

    //  	STMFLASH_Read((const u8)&eepAlarms[num],(u32*)alarm,sizeof(alarm_s));

    //	eeprom_read_block(alarm, &eepAlarms[num], sizeof(alarm_s));
    //	if(alarm->hour > 23)
    //		memset(alarm, 0, sizeof(alarm_s));
}

bool alarm_getNext(alarm_s *alarm)
{
    if (nextAlarm == NOALARM)
    {
        return false;
    }

    alarm_get(nextAlarm, alarm);
    return true;
}

byte alarm_getNextDay()
{
    return nextAlarmDay;
}

void alarm_save(byte num, alarm_s *alarm)
{
    //	eeprom_update_block(alarm, &eepAlarms[num], sizeof(alarm_s));
    memcpy(&eepAlarms[num], alarm, sizeof(alarm_s));
    
    // eepAlarms[num]=*alarm;
    getNextAlarm();
}

extern const uint32_t STAY[];

// 响应闹钟
// main.c 循环调用
void alarm_update()
{
    bool wasTriggered = isAlarmTriggered; // 之前为false
    bool alarmNow = isAlarmTimeReached(); // 调用这个方法后, isAlarmTriggered 会变为true

    if (isAlarmTriggered) // 0秒闹钟触发
    {
        if (alarmNow) // 到达闹钟时间 1分钟内
        {
            if (!wasTriggered && isAlarmTriggered) // 闹钟第一次触发
            {
                oldDrawFunc = display_setDrawFunc(draw);
                oldBtn1Func = buttons_setFunc(BTN_1, NULL);
                oldBtn2Func = buttons_setFunc(BTN_2, stopAlarm);
                oldBtn3Func = buttons_setFunc(BTN_3, NULL);
                tune_play(STAY, VOL_ALARM, PRIO_ALARM);
            }
        }
        else  // 未到达闹钟时间 或 超过1分钟 就停止闹钟
        {
            stopAlarm();
        }
    }
}

void alarm_updateNextAlarm()
{
    getNextAlarm();
}

// 判断是否到达闹钟时间
static bool isAlarmTimeReached()
{

    alarm_s nextAlarm;

    // Make sure we're in 24 hour mode
    time_s time;
    time.hour = timeDate.time.hour;
    time.ampm = timeDate.time.ampm;
    time_timeMode(&time, TIMEMODE_24HR);

    if (alarm_getNext(&nextAlarm) && alarm_dayEnabled(nextAlarm.days, timeDate.date.day) && nextAlarm.hour == time.hour && nextAlarm.min == timeDate.time.mins)
    {
        if (timeDate.time.secs == 0)
        {
            // 只在秒为0时触发
            isAlarmTriggered = true;
        }

        // 到达闹钟时间, 1分钟内都算
        return true;
    }

    return false;
}

// This func needs to be ran when an alarm has changed, time has changed or an active alarm has been turned off
static void getNextAlarm()
{
    byte next = NOALARM;
    uint nextTime = (uint)UINT_MAX;

    // Make sure time is in 24 hour mode
    time_s timeNow;
    timeNow.hour = timeDate.time.hour;
    timeNow.ampm = timeDate.time.ampm;
    time_timeMode(&timeNow, TIMEMODE_24HR);

    // Now in minutes from start of week
    uint now = toMinutes(timeNow.hour, timeDate.time.mins + 1, timeDate.date.day);

    // Loop through alarms
    LOOPR(ALARM_COUNT, i)
    {
        // Get alarm data
        alarm_s alarm;
        alarm_get(i, &alarm);

        // Not enabled
        if (!alarm.enabled)
        {
            continue;
        }

        // Loop through days
        LOOPR(7, d)
        {
            // Day not enabled
            if (!alarm_dayEnabled(alarm.days, d))
            {
                continue;
            }

            // Alarm time in minutes from start of week
            uint alarmTime = toMinutes(alarm.hour, alarm.min, d);

            // Minutes to alarm
            int timeTo = alarmTime - now;

            // Negative result, must mean alarm time is earlier in the week than now, add a weeks time
            if (timeTo < 0)
            {
                timeTo += ((6 * 1440) + (23 * 60) + 59); // 10079
            }

            // Is minutes to alarm less than the last minutes to alarm?
            if ((uint)timeTo < nextTime)
            {
                // This is our next alarm
                nextTime = timeTo;
                next = i;
                nextAlarmDay = d;
            }
        }
    }

    // Set next alarm
    nextAlarm = next;

    DS3231_Set_alarm1(); // 存入ds3231闹钟1中
}

static uint toMinutes(byte hours, byte mins, byte dow)
{
    uint total = mins;
    total += hours * 60;
    total += dow * 1440;
    return total;
}

static bool stopAlarm()
{
    getNextAlarm();
    display_setDrawFunc(oldDrawFunc);
    buttons_setFuncs(oldBtn1Func, oldBtn2Func, oldBtn3Func);
    //	oled_setInvert(appConfig.invert);
    //	pwrmgr_setState(PWR_ACTIVE_ALARM, PWR_STATE_NONE);
    tune_stop(PRIO_ALARM);
    isAlarmTriggered = false;
    return true;
}

static display_t draw()
{
    if ((millis8_t)millis() < 128)
    {
        draw_bitmap(16, 16, menu_alarm, 32, 32, NOINVERT, 0);
    }

    // Draw time
    draw_string(time_timeStr(), NOINVERT, 79, 20);

    // Draw day
    char buff[BUFFSIZE_STR_DAYS];
    strcpy(buff, days[timeDate.date.day]);
    draw_string(buff, false, 86, 36);

    return DISPLAY_DONE;
}
