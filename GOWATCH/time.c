/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

#define SECONDS_IN_MIN 60
#define SECONDS_IN_HOUR (60 * SECONDS_IN_MIN)
#define SECONDS_IN_DAY (((uint32_t)24) * SECONDS_IN_HOUR)

#define FEB_LEAP_YEAR 29

// �·�����
static const byte monthDayCount[] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

timeDate_s timeDate;
// static timestamp_t timestamp;
bool update;
static uint32_t lastUpdateTime = 0;  // ��¼�ϴθ���ʱ��

void time_init()
{
    time_wake();
}

void time_sleep()
{
}

void time_shutdown()
{
}

rtcwake_t time_wake()
{
    if (CONFIRM_BTN_KEY == 1) // ����
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
    // ����ʱ������
    memcpy(&timeDate, newTimeDate, sizeof(timeDate_s));

    // ������Ϊ0
    timeDate.time.secs = 0;

    // ת��24Сʱģʽ���浽RTC
    time_timeMode(&timeDate.time, TIMEMODE_24HR);

    RTC_Set_Datetime(&timeDate);

    // ���ú�ʱ����������������
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
    // ���뷶Χ��0-11����Ӧ1-12�£�
    // ���չ�ʽʹ�õ���1-14��
    // �����·ݺ����
    // �����1�»�2�£�����һ���13�º�14�¼���
    u8 m2 = (u8)m;
    m2 += 1;
    if (m2 < 3) {
        m2 = (month_t)(m2 + 12);
        yy--;
    }
    
    int y = yy + 2000;  // �������
    int k = y % 100;    // ��ݺ���λ
    int j = y / 100;    // ���ǰ��λ
    
    // ���չ�ʽ
    int h = (d + ((13 * (m2 + 1)) / 5) + k + (k / 4) + (j / 4) - (2 * j)) % 7;
    
    // ȷ�����Ϊ����
    h = (h + 7) % 7;
    
    // ת��Ϊ������Ҫ�ĸ�ʽ��0 = ��һ��1 = �ܶ���...��6 = ����
    // ���չ�ʽ�����0 = ������1 = ���գ�2 = ��һ��...��6 = ����
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

// תΪʱ��mode, 12��24Сʱ
/*
12Сʱ�ƣ���һ���Ϊ����12Сʱ���ڣ������磨AM�������磨PM����
    AM��Ante Meridiem������ҹ12:00������11:59��
    PM��Post Meridiem��������12:00����ҹ11:59��

12Сʱ�ƵĹ���
    һ�챻��Ϊ����12Сʱ���ڣ�AM����ҹ�����磩��PM�����絽��ҹ����
    Сʱ����12��ʼ��������1��11��û��0��
    AM���ڣ�12:00 AM����ҹ���� 1:00 AM �� �� �� 11:59 AM�����磩��
    PM���ڣ�12:00 PM�����磩�� 1:00 PM �� �� �� 11:59 PM����ҹǰ����

24Сʱ�ƣ�ֱ����0-24��ʾȫ��ʱ�䣬��������AM/PM�����磬13:00������1�㡣
*/
void time_timeMode(time_s *time, timemode_t mode)
{
    byte hour = time->hour;

    // תΪ12Сʱ��
    if (mode == TIMEMODE_12HR)
    {
        if (time->ampm != CHAR_24) // Already 12hr
        {
            return;
        }
        // ֮ǰ��24Сʱ, ����תΪ12Сʱ, ��13��������1��
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
            // 0����12��AM (12Сʱ��û��0��)
            if (hour == 0)
            {
                hour = 12;
            }

            time->ampm = CHAR_AM;
        }
    }
    // תΪ24Сʱ��
    else
    {
        if (time->ampm == CHAR_AM && hour == 12) // Midnight 12AM = 00:00
        {
            hour = 0;
        }
        else if (time->ampm == CHAR_PM && hour < 12) // No change for 12PM (midday)
        {
            hour += 12;
        }

        time->ampm = CHAR_24;
    }

    time->hour = hour;
}

ulong lastMils = 0;

// ����ʱ��
void time_update()
{
    uint32_t currentTime = millis();
    
    // ÿ1000ms��1�룩����һ��
    if (lastUpdateTime != 0 && currentTime - lastUpdateTime < 1000) {
        return;
    }
    lastUpdateTime = currentTime;

    RTC_Read_Datetime(&timeDate);

    // RTC����������24Сʱʱ��, תΪ���õ�12��24Сʱʱ��
    byte hour = timeDate.time.hour; // ����ת��12Сʱ��, ����resetStepCounter��Ҫ�ж�
    timeDate.time.ampm = CHAR_24;
		time_timeMode(&timeDate.time, appConfig.timeMode);

    // alarm_updateNextAlarm();

    // Serial.printf("time_update\n");
    // Serial.printf("%d-%d-%d %d:%d:%d\n", timeDate.date.year, timeDate.date.month, timeDate.date.date, timeDate.time.hour, timeDate.time.mins, timeDate.time.secs);

    // u8 rtcPin = digitalRead(RTC_INT_PIN);
    // Serial.printf("rtc pin: %d\n", rtcPin);

    // ���㱨ʱ
    if (timeDate.time.mins == 0 && timeDate.time.secs == 0)
    {
        tune_play(tuneHour, VOL_HOUR, PRIO_HOUR);

        // �������0��0��, ����Ҫreset step count
        if (hour == 0) {
            // addStepLog();
            // resetStepCounter();
        }
    }

    // ʱ��оƬ����, ��Ҫ��������
    if (timeDate.date.year == 0 || timeDate.date.date == 0)
    {
        RTC_Config("2025:03:12:12:00:00");
    }
}

// �õ���һ�����㱨ʱ��ʱ��
void time_getNextZhengdiao(timeDate_s *timeDate2) {
    // �õ���ǰʱ��
    RTC_Read_Datetime(&timeDate);

    // ��3��ֵ���Բ���, ���ùܽ�λ
    timeDate2->date.year = timeDate.date.year;
    timeDate2->date.month = timeDate.date.month;
    timeDate2->date.date = timeDate.date.date;

    timeDate2->time.hour = (timeDate.time.hour + 1) % 24; // 0-23
    timeDate2->time.mins = 0;
    timeDate2->time.secs = 0;

    // �ڶ���
    if (timeDate.time.hour == 23)
    {
        timeDate2->date.day = (day_t)((timeDate.date.day + 1) % 7); // 0-6
    }
    else
    {
        timeDate2->date.day = timeDate.date.day;
    }
}

/*
typedef struct
{
	day_t day; // 0-6, ����, 0:��һ, 6:����
	byte date; // 1-31, ����
	month_t month; // 0-11, �·�, 0:һ��, 11:ʮ����
	byte year; // 0-99, ���
} date_s;

typedef struct  { 
  uint8_t Second; 
  uint8_t Minute; 
  uint8_t Hour; 
  uint8_t Wday;   // ���ڼ� (1 = ������, 7 = ������)
  uint8_t Day; // һ�����еĵڼ��� (1 - 31)
  uint8_t Month; // �·� 1-12
  uint8_t Year;   // offset from 1970; 
} 	tmElements_t, TimeElements, *tmElementsPtr_t;
*/

// tmWdayתΪday_t
// tmWday 1-7, 1:������, 7:������
// day_t 0-6, 0:��һ, 6:����
day_t tmWdayToDay(uint8_t tmWday)
{
  if (tmWday == 1)
  {
    return DAY_SUN;
  }
  else if (tmWday == 7)
  {
    return DAY_SAT;
  }
  else
  {
    return (day_t)(tmWday - 2);
  }
}

// day_tתΪtmWday
// day_t 0-6, 0:��һ, 6:����, 4:����
// tmWday 1-7, 1:������, 7:������, 6:����
uint8_t dayToTmWday(day_t day)
{
  if (day == DAY_SUN)
  {
    return 1;
  }
  else if (day == DAY_SAT)
  {
    return 7;
  }
  else
  {
    return (uint8_t)(day + 2);
  }
}

// PCF8563 �����ڱ�ʾ�У�0��ʾ����, 6��ʾ����
uint8_t dayToPcf8563Day(day_t day)
{
    return dayToTmWday(day)-1;
}

// timeDate monthתΪtmMonth
// timeDate month 0-11, 0:һ��, 11:ʮ����
// tmMonth 1-12
uint8_t timeDateMonthToTmMonth(month_t month)
{
  return (uint8_t)(month + 1);
}

// tmMonthתΪtimeDate month
// tmMonth 1-12
// timeDate month 0-11, 0:һ��, 11:ʮ����
month_t tmMonthToTimeDateMonth(uint8_t tmMonth)
{
  return (month_t)(tmMonth - 1);
}
