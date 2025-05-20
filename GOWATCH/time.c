/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"
#include "ds3231.h"
#include "rtc.h"

#define SECONDS_IN_MIN 60
#define SECONDS_IN_HOUR (60 * SECONDS_IN_MIN)
#define SECONDS_IN_DAY (((uint32_t)24) * SECONDS_IN_HOUR)

#define FEB_LEAP_YEAR 29

// 月份天数
static const byte monthDayCount[] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

timeDate_s timeDate;
// static timestamp_t timestamp;
bool update;

static void getRtcTime(void);

void time_init()
{
    time_wake();
}

void time_sleep()
{
    ////	TCCR2B = _BV(CS22)|_BV(CS20);
    ////	while(ASSR & (_BV(OCR2BUB)|_BV(TCR2AUB)|_BV(TCR2BUB)));

    // #if RTC_SRC != RTC_SRC_INTERNAL
    //	// Turn off square wave
    //	rtc_sqw(RTC_SQW_OFF);

    //	alarm_s alarm;

    //	// Set next alarm
    //	if(alarm_getNext(&alarm))
    //	{
    //		alarm.days = alarm_getNextDay() + 1;
    //		rtc_setUserAlarmWake(&alarm);
    //	}
    //	else // No next alarm
    //		rtc_setUserAlarmWake(NULL);

    //	// Set up hour beeps
    //	if(appConfig.volHour)
    //	{
    //		alarm.min = 0;
    //		alarm.hour = 0;
    //		alarm.days = 0;
    //		rtc_setSystemAlarmWake(&alarm);
    //	}
    //	else // Hour beep volume set to minimum, so don't bother with the system alarm
    //		rtc_setSystemAlarmWake(NULL);
    // #endif

    update = false;
}

void time_shutdown()
{
    // #if RTC_SRC != RTC_SRC_INTERNAL
    //	rtc_sqw(RTC_SQW_OFF);
    //	rtc_setUserAlarmWake(NULL);
    //	rtc_setSystemAlarmWake(NULL);
    // #endif

    update = false;
}

rtcwake_t time_wake()
{
    // #if RTC_SRC != RTC_SRC_INTERNAL
    // getRtcTime();

    // Turn on square wave
    //	rtc_sqw(RTC_SQW_ON);

    //	update = false;

    // Check alarms
    if (KEY1 == 1)
    {
        return RTCWAKE_SYSTEM;
    }
    else
    {

        return RTCWAKE_NONE;
    }
}

void time_set(timeDate_s *newTimeDate)
{

    memcpy(&timeDate, newTimeDate, sizeof(timeDate_s));

    timeDate.time.secs = 0;
    time_timeMode(&timeDate.time, appConfig.timeMode);

#ifdef RTC_SRC
    RTC_SetDatetime(timeDate.date.year + 2000, timeDate.date.month + 1, timeDate.date.date, timeDate.time.hour, timeDate.time.mins, timeDate.time.secs);
    getRtcTime();
#else
    DS3231_Set_Date();
    DS3231_Set_Time();
#endif

    alarm_updateNextAlarm();
}

bool time_isLeapYear(byte year)
{
    // Watch only supports years 2000 - 2099, so no need to do the full calculation

    return (year % 4 == 0);

    // uint fullYear = year + 2000;
    // return ((fullYear & 3) == 0 && ((fullYear % 25) != 0 || (fullYear & 15) == 0));
}

// Calculate day of week from year, month and date
// Using Zeller's congruence formula
// Parameters:
//   yy: year (0-99, representing 2000-2099)
//   m: month (0-11, where 0=January, 1=February, ..., 11=December)
//   d: day of month (1-31)
// Returns:
//   day of week (0=Monday, 1=Tuesday, ..., 6=Sunday)
day_t time_calcWeekDay(byte yy, month_t m, byte d)
{
    // 输入范围是0-11（对应1-12月）
    // 蔡勒公式使用的是1-14月
    // 调整月份和年份
    // 如果是1月或2月，按上一年的13月和14月计算
    m += 1;
    if (m < 3) {
        m = (month_t)(m + 12);
        yy--;
    }
    
    int y = yy + 2000;  // 完整年份
    int k = y % 100;    // 年份后两位
    int j = y / 100;    // 年份前两位
    
    // 蔡勒公式
    int h = (d + ((13 * (m + 1)) / 5) + k + (k / 4) + (j / 4) - (2 * j)) % 7;
    
    // 确保结果为正数
    h = (h + 7) % 7;
    
    // 转换为我们需要的格式：0 = 周一，1 = 周二，...，6 = 周日
    // 蔡勒公式结果：0 = 周六，1 = 周日，2 = 周一，...，6 = 周五
    int dow = (h + 5) % 7;
    
    return (day_t)dow;
}

byte time_monthDayCount(month_t month, byte year)
{
    //	byte numDays = pgm_read_byte(&monthDayCount[month]);
    byte numDays = monthDayCount[month];

    if (month == MONTH_FEB && time_isLeapYear(year))
    {
        numDays = FEB_LEAP_YEAR;
    }

    return numDays;
}

char *time_timeStr()
{
    static char buff[BUFFSIZE_TIME_FORMAT_SMALL];
    sprintf_P(buff, PSTR(TIME_FORMAT_SMALL), timeDate.time.hour, timeDate.time.mins, timeDate.time.ampm);
    return buff;
}

void time_timeMode(time_s *time, timemode_t mode)
{
    byte hour = time->hour;

    if (mode == TIMEMODE_12HR)
    {
        if (time->ampm != CHAR_24)
        { // Already 12hr
            return;
        }
        else if (hour >= 12)
        {
            if (hour > 12)
            {
                hour -= 12;
            }

            time->ampm = CHAR_PM;
        }
        else
        {
            if (hour == 0)
            {
                hour = 12;
            }

            time->ampm = CHAR_AM;
        }
    }
    else
    { // 24 hour
        if (time->ampm == CHAR_AM && hour == 12)
        { // Midnight 12AM = 00:00
            hour = 0;
        }
        else if (time->ampm == CHAR_PM && hour < 12)
        { // No change for 12PM (midday)
            hour += 12;
        }

        time->ampm = CHAR_24;
    }

    time->hour = hour;
}

void time_update()
{
    static bool tuneHourFlag;

    if (!update)
    {
        return;
    }

    update = false;

#ifdef RTC_SRC
    getRtcTime();

    if (timeDate.time.secs == 0 && timeDate.time.mins == 0)
    {
        tune_play(tuneHour, VOL_HOUR, PRIO_HOUR);
    }

#else
    // Slightly modified code from AVR134
    // console_log(50, "DS3231_Get_Time !");

    DS3231_Get_Time();

    // console_log(50, "DS3231_Get_Time2 !");
    // console_log(50, "year=%d", timeDate.date.year);

    if (timeDate.time.mins == 0 && tuneHourFlag == 1)
    {
        tune_play(tuneHour, VOL_HOUR, PRIO_HOUR);
        tuneHourFlag = 0;
    }

    if (timeDate.time.mins > 0)
    {
        tuneHourFlag = 1;
    }

    //	if(timeDate.time.secs == 60)
    //	{
    //		timeDate.time.secs = 0;
    //		if(++timeDate.time.mins == 60)
    //		{
    //			timeDate.time.mins = 0;
    //			if(++timeDate.time.hour == 24) // What about 12 hr?
    //			{
    //				byte numDays = time_monthDayCount(timeDate.date.month, timeDate.date.year);

    //				timeDate.time.hour = 0;
    //				if (++timeDate.date.date == numDays + 1)
    //				{
    //					timeDate.date.month++;
    //					timeDate.date.date = 1;
    //				}

    //				if (timeDate.date.month == 13)
    //				{
    //					timeDate.date.month = (month_t)1;
    //					timeDate.date.year++;
    //					if(timeDate.date.year == 100)
    //						timeDate.date.year = 0;
    //				}

    //				if(++timeDate.date.day == 7)
    //					timeDate.date.day = (day_t)0;
    //			}

    //			tune_play(tuneHour, VOL_HOUR, PRIO_HOUR);
    //		}
    //	}
#endif

    // debug_printf("%02hhu:%02hhu:%02hhu\n", timeDate.time.hour, timeDate.time.mins, timeDate.time.secs);
    // debug_printf("T: %hhuC\n",rtc_temp());
}

// #if RTC_SRC == RTC_SRC_INTERNAL
// ISR(TIMER2_OVF_vect)
// #else
// #ifdef __AVR_ATmega32U4__
// ISR(INT3_vect)
// #else
// ISR(INT0_vect)
// #endif
// #endif
//{
//	update = true;
// }

// life
static void getRtcTime()
{
    RTC_GetDatetime();
    timeDate.time.ampm = CHAR_24;

    // Convert to correct time mode
    time_timeMode(&timeDate.time, appConfig.timeMode);

    // rtc_get(&timeDate);
    // timeDate.time.ampm = CHAR_24;

    // Convert to correct time mode
    // time_timeMode(&timeDate.time, appConfig.timeMode);
}
