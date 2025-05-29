#include "rtc.h"
#include <string.h>
#include <stdlib.h>

void RTC_Init(void)
{
    PCF8563_Init();
    PCF8563_InitClock();
}

void RTC_Config(const char* datetime)
{
    RTC_PCFConfig(datetime);
}

void RTC_ClearAlarm(void)
{
    PCF8563_ClearAlarm();
}

void RTC_SetNextMinuteAlarm(void)
{
    PCF8563_ClearAlarm();
    
    // Set alarm for next minute
    u8 nextAlarmMinute = PCF8563_GetMinute();
    nextAlarmMinute = (nextAlarmMinute == 59) ? 0 : (nextAlarmMinute + 1);
    PCF8563_SetAlarm(nextAlarmMinute, 99, 99, 99);
}

void RTC_Set_Alarm(u8 minute, u8 hour, u8 wday) // RTC_SetAlarm 方法和 stm32f10x_rtc.c 中的 RTC_SetAlarm 方法冲突
{
    PCF8563_ClearAlarm();
    PCF8563_SetAlarm(minute, hour, 99, wday);
}

/*
根据 PCF8563 的数据手册和 typedefs.h 的定义，我发现了一些不一致的地方：
星期（day）：
    PCF8563: 0-6，0=星期日，6=星期六
    typedefs.h: 0-6，0=星期一，6=星期日
    不一致：起始日不同
月份（month）：
    PCF8563: 1-12
    typedefs.h: 0-11（使用枚举，0=一月，11=十二月）
    不一致：范围不同
年份（year）：
    PCF8563: 0-99（2000-2099）
    typedefs.h: 0-99
    一致：范围相同
*/

void RTC_Read_Datetime(timeDate_s* timeDate)
{
    PCF8563_GetDateTime();
    
    timeDate->time.secs = PCF8563_GetSecond();
    timeDate->time.mins = PCF8563_GetMinute();
    timeDate->time.hour = PCF8563_GetHour();
    
    // Convert PCF8563 weekday (0=Sunday) to typedefs.h weekday (0=Monday)
    u8 pcf_weekday = PCF8563_GetWeekday();
    timeDate->date.day = (pcf_weekday == 0) ? 6 : (pcf_weekday - 1);
    
    timeDate->date.date = PCF8563_GetDay();
    
    // Convert PCF8563 month (1-12) to typedefs.h month (0-11)
    timeDate->date.month = PCF8563_GetMonth() - 1;
    
    timeDate->date.year = PCF8563_GetYear();
}

void RTC_Set_Datetime(timeDate_s* timeDate)
{
    // Convert typedefs.h weekday (0=Monday) to PCF8563 weekday (0=Sunday)
    u8 pcf_weekday = (timeDate->date.day == 6) ? 0 : (timeDate->date.day + 1);
    
    // Set date
    PCF8563_SetDate(timeDate->date.date, 
                    pcf_weekday,
                    timeDate->date.month + 1,  // Convert typedefs.h month (0-11) to PCF8563 month (1-12)
                    0,  // century (0=2000)
                    timeDate->date.year);
    
    // Set time
    PCF8563_SetTime(timeDate->time.hour,
                    timeDate->time.mins,
                    timeDate->time.secs);
    
    // Clear alarm after setting time
    RTC_ClearAlarm();
}

u8 RTC_GetTemperature(void)
{
    return 255;  // PCF8563 doesn't have temperature sensor
}

void RTC_PrintAlarm(void)
{
    PCF8563_GetAlarm();
    printf("RTC alarm: %d:%d, weekDay: %d, date: %d\r\n", 
           PCF8563_GetAlarmHour(),
           PCF8563_GetAlarmMinute(),
           PCF8563_GetAlarmWeekday(),
           PCF8563_GetAlarmDay());
}

static void RTC_PCFConfig(const char* datetime)
{
    if (datetime != NULL && strlen(datetime) > 0)
    {
        timeDate_s timeDate;
        
        // Parse datetime string (YYYY:MM:DD:HH:MM:SS)
        timeDate.date.year = atoi(RTC_GetValue(datetime, ':', 0)) - 2000;
        timeDate.date.month = atoi(RTC_GetValue(datetime, ':', 1));
        timeDate.date.date = atoi(RTC_GetValue(datetime, ':', 2));
        timeDate.time.hour = atoi(RTC_GetValue(datetime, ':', 3));
        timeDate.time.mins = atoi(RTC_GetValue(datetime, ':', 4));
        timeDate.time.secs = atoi(RTC_GetValue(datetime, ':', 5));
        
        // Calculate weekday
        timeDate.date.day = PCF8563_WhatWeekday(timeDate.date.date,
                                              timeDate.date.month,
                                              0,  // century (0=2000)
                                              timeDate.date.year);
        
        // Set the date and time
        RTC_Set_Datetime(&timeDate);
    }
    
    // Clear alarm
    RTC_ClearAlarm();
}

static const char* RTC_GetValue(const char* data, char separator, int index)
{
    static char buffer[10];  // Buffer for the result
    int found = 0;
    int start = 0;
    int i;
    
    for (i = 0; data[i] != '\0' && found <= index; i++)
    {
        if (data[i] == separator || data[i + 1] == '\0')
        {
            if (found == index)
            {
                int len = (data[i] == separator) ? (i - start) : (i - start + 1);
                if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
                strncpy(buffer, &data[start], len);
                buffer[len] = '\0';
                return buffer;
            }
            found++;
            start = i + 1;
        }
    }
    
    return "";
} 