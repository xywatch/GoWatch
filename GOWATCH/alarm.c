/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

#define NOALARM UCHAR_MAX

static draw_f oldDrawFunc;
static button_f oldBtn1Func;
static button_f oldBtn2Func;
static button_f oldBtn3Func;

byte nextAlarmIndex;
byte nextAlarmDay; // ��һ�����ӵ�����, 0-6, 0:��һ, 6:����
bool isAlarmTriggered; // 

alarm_s eepAlarms[ALARM_COUNT] EEMEM = {
    {8, 0, 63}, // 22:45:00, 127 = 1111111, ��ʾ����1,2,3,4,5,6,7, 255=1(����) 111111(�ܶ�)1(��һ), ��ʾ���������ҿ���
    {9, 0, 63}, 
    {7, 45, 63},  // 63 = 111111, ��ʾ����1,2,3,4,5,6
    {9, 4, 0}, 
    {3, 1, 7} // 7 = 111, ��ʾ����1,2,3
};
bool isAlarmInited = false;
static bool need_updateAlarm_in_nextLoop = false;

static bool isAlarmTimeReached(void);
static void getNextAlarm(void);
static uint toMinutes(byte, byte, byte);
static bool stopAlarm(void);
static display_t draw(void);

extern bool keep_on;

bool alarm_is_enabled()
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

// ��ʼ������, ִֻ��һ��, ��setup�е���
// ��ʹ���Ѻ�, Ҳ�������³�ʼ��
// ��Ϊ��������ΪRTC����, �����ִ������, ���Ӿͻ�����һ������
void alarm_init()
{
    if (!isAlarmInited) {
        isAlarmInited = true;
        getNextAlarm();
    }
}

void alarm_reset()
{
    memset(&eepAlarms, 0x00, ALARM_COUNT * sizeof(alarm_s));
}

void alarm_get(byte num, alarm_s *alarm)
{
    memcpy(alarm, &eepAlarms[num], sizeof(alarm_s));
}

bool alarm_getNext(alarm_s *alarm)
{
    if (nextAlarmIndex == NOALARM)
    {
        return false;
    }

    alarm_get(nextAlarmIndex, alarm);
    return true;
}

byte alarm_getNextDay()
{
    return nextAlarmDay;
}

void alarm_save(byte num, alarm_s *alarm)
{
    memcpy(&eepAlarms[num], alarm, sizeof(alarm_s));
    getNextAlarm();
}

extern const uint32_t STAY[];

// ��Ӧ����
// main.c ѭ������
void alarm_update()
{
    bool wasTriggered = isAlarmTriggered; // ֮ǰΪfalse
    bool alarmNow = isAlarmTimeReached(); // �������������, isAlarmTriggered ���Ϊtrue

    // Serial.printf("alarm_update isAlarmTriggered: %d, alarmNow: %d\n", isAlarmTriggered, alarmNow);

    if (isAlarmTriggered) // 0�����Ӵ���
    {
        if (alarmNow) // ��������ʱ�� 1������
        {
            if (!wasTriggered && isAlarmTriggered) // ���ӵ�һ�δ���
            {
                keep_on = true;
                need_updateAlarm_in_nextLoop = false; // ��һ��loopʱ��ҪgetNextAlarm�� Ӧ����stopʱ��getNextAlarm
                // Serial.printf("alarm_update isAlarmTriggered: %d, alarmNow: %d\n", isAlarmTriggered, alarmNow);
                oldDrawFunc = display_setDrawFunc(draw);
                oldBtn1Func = buttons_setFunc(BTN_1, NULL);
                oldBtn2Func = buttons_setFunc(BTN_2, stopAlarm);
                oldBtn3Func = buttons_setFunc(BTN_3, NULL);
                tune_play(STAY, VOL_ALARM, PRIO_ALARM);
            }
        }
        else  // δ��������ʱ�� �� ����1���� ��ֹͣ����
        {
            stopAlarm();
        }
    }

    // ���»�ȡ��һ��alarm
    // ���������alarm����һ�¾�stopAlarm��
    if (need_updateAlarm_in_nextLoop) {
        need_updateAlarm_in_nextLoop = false;
        getNextAlarm();
    }
}

// RTC�ж�ʱ����, ����һ��loop��alarm_update���ٸ���
// ��ΪRTC�жϿ������������㱨ʱ, ��������жϺ���Ҫ����alarm
// ��Ҳ�п��������alarm, ������Ҫ��alarm_update���ٸ���
void alarm_need_updateAlarm_in_nextLoop () {
    need_updateAlarm_in_nextLoop = true;
}

void alarm_updateNextAlarm()
{
    getNextAlarm();
}

// �ж��Ƿ񵽴�����ʱ��
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
            // ֻ����Ϊ0ʱ����
            isAlarmTriggered = true;
        }

        // ��������ʱ��, 1�����ڶ���
        return true;
    }

    return false;
}

void rtc_set_pcf8563_alarm (byte hour, byte min, day_t day) {
    byte pcf8563_day = dayToPcf8563Day(day);
    // Serial.printf("set alarm for %02d:%02d, nextAlarmDay: %d -> pcf8563_day: %d\n", hour, min, day, pcf8563_day);
    // delay(100);
    RTC_Set_Alarm(min, hour, pcf8563_day);
    RTC_PrintAlarm();
}

void rtc_set_alarm(void)
{
    // PCF8563ֻ������һ������
    // Ҫʵ�����㱨ʱ, ��Ҫ����һ�����㱨ʱ��ʱ�����õ�RTC��
    // ����������12:01, ��һ�����㱨ʱ��13:00, ��13:00���õ�RTC��
    // �õ���һ�����㱨ʱ��ʱ��
    timeDate_s nextZhengdian;
    time_getNextZhengdiao(&nextZhengdian);
    // Serial.printf("nextZhengdian %02d:%02d, day:%d\n", nextZhengdian.time.hour, nextZhengdian.time.mins, nextZhengdian.date.day);

    alarm_s alarm;
    if (!alarm_getNext(&alarm))
    {
        // �������
        // RTC.clearAlarm();

        // û������, ��ֱ������һ������
        // Serial.printf("û������, ��ֱ������һ������\n");
        rtc_set_pcf8563_alarm(nextZhengdian.time.hour, nextZhengdian.time.mins, nextZhengdian.date.day);
        return;
    }
    // Serial.printf("nextAlarm %02d:%02d, day:%d\n", alarm.hour, alarm.min, nextAlarmDay);

    // ����������, ���жϵ�ǰ�����Ƿ��������, �����, �������ӵ�, �����������
    time_s timeNow;
    timeNow.hour = timeDate.time.hour;
    timeNow.ampm = timeDate.time.ampm;
    time_timeMode(&timeNow, TIMEMODE_24HR);

    // Now in minutes from start of week
    uint now = toMinutes(timeNow.hour, timeDate.time.mins + 1, timeDate.date.day);
    uint nextZhengdianMins = toMinutes(nextZhengdian.time.hour, nextZhengdian.time.mins + 1, nextZhengdian.date.day);
    uint nextAlarmMins = toMinutes(alarm.hour, alarm.min + 1, nextAlarmDay);

    // �������һ��������, �ұȵ�ǰʱ����
    // now zd alarm
    if (now < nextZhengdianMins && nextZhengdianMins < nextAlarmMins)
    {
        // Serial.println("now zd alarm");
        rtc_set_pcf8563_alarm(nextZhengdian.time.hour, nextZhengdian.time.mins, nextZhengdian.date.day);
        return;
    }

    // alarm now zd
    if (nextAlarmMins < now && now < nextZhengdianMins)
    {
        // Serial.println("alarm now zd");
        rtc_set_pcf8563_alarm(nextZhengdian.time.hour, nextZhengdian.time.mins, nextZhengdian.date.day);
        return;
    }

    // zd alarm now
    if (nextZhengdianMins < nextAlarmMins && nextAlarmMins < now)
    {
        // Serial.println("zd alarm now");
        rtc_set_pcf8563_alarm(nextZhengdian.time.hour, nextZhengdian.time.mins, nextZhengdian.date.day);
        return;
    }

    // Serial.println("use alarm");
    rtc_set_pcf8563_alarm(alarm.hour, alarm.min, (day_t)nextAlarmDay);
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
    nextAlarmIndex = next;

    rtc_set_alarm();
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

    keep_on = false;
    return true;
}

static display_t draw()
{
    draw_bitmap(16, 16, menu_alarm, 32, 32, NOINVERT, 0);

    // Draw time
    draw_string(time_timeStr(), NOINVERT, 79, 20);

    // Draw day
    char buff[BUFFSIZE_STR_DAYS];
    strcpy(buff, days[timeDate.date.day]);
    draw_string(buff, false, 86, 36);

    return DISPLAY_DONE;
}
