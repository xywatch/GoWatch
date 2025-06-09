#include "pcf8563.h"

/* Global variables */
static u8 hour;
static u8 minute;
static u8 volt_low;
static u8 sec;
static u8 day;
static u8 weekday;
static u8 month;
static u8 year;
static u8 alarm_hour;
static u8 alarm_minute;
static u8 alarm_weekday;
static u8 alarm_day;
static u8 squareWave;
static u8 timer_control;
static u8 timer_value;
static u8 status1;
static u8 status2;
static u8 century;
static char strOut[9];
static char strDate[11];
static int Rtcc_Addr;

const unsigned int months_days[]={31,59,90,120,151,181,212,243,273,304,334};	// days count for each month

void PCF8563_Init(void)
{
    Rtcc_Addr = RTCC_R>>1; // 0x51
    // I2C_GPIO_Config(); // 已经初始化过, 这里不再重复初始化
}

/* Private internal functions */
static u8 PCF8563_DecToBcd(u8 val)
{
    return ( (val/10*16) + (val%10) );
}

static u8 PCF8563_BcdToDec(u8 val)
{
    return ( (val/16*10) + (val%16) );
}

void PCF8563_ZeroClock(void)
{
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(0x0);        // start address
    I2C_WaitAck();

    I2C_SendByte(0x0);     //control/status1
    I2C_WaitAck();
    I2C_SendByte(0x0);     //control/status2
    I2C_WaitAck();
    I2C_SendByte(0x00);    //set seconds to 0 & VL to 0
    I2C_WaitAck();
    I2C_SendByte(0x00);    //set minutes to 0
    I2C_WaitAck();
    I2C_SendByte(0x00);    //set hour to 0
    I2C_WaitAck();
    I2C_SendByte(0x01);    //set day to 1
    I2C_WaitAck();
    I2C_SendByte(0x00);    //set weekday to 0
    I2C_WaitAck();
    I2C_SendByte(0x81);    //set month to 1, century to 1900
    I2C_WaitAck();
    I2C_SendByte(0x00);    //set year to 0
    I2C_WaitAck();
    I2C_SendByte(0x80);    //minute alarm value reset to 00
    I2C_WaitAck();
    I2C_SendByte(0x80);    //hour alarm value reset to 00
    I2C_WaitAck();
    I2C_SendByte(0x80);    //day alarm value reset to 00
    I2C_WaitAck();
    I2C_SendByte(0x80);    //weekday alarm value reset to 00
    I2C_WaitAck();
    I2C_SendByte(SQW_32KHZ); //set SQW to default, see: setSquareWave
    I2C_WaitAck();
    I2C_SendByte(0x0);     //timer off
    I2C_WaitAck();
    I2C_Stop();
}

void PCF8563_ClearStatus(void)
{
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(0x0);
    I2C_WaitAck();
    I2C_SendByte(0x0);                 //control/status1
    I2C_WaitAck();
    I2C_SendByte(0x0);                 //control/status2
    I2C_WaitAck();
    I2C_Stop();
}

u8 PCF8563_ReadStatus2(void)
{
    PCF8563_GetDateTime();
    return PCF8563_GetStatus2();
}

void PCF8563_ClearVoltLow(void)
{
    PCF8563_GetDateTime();
    // Only clearing is possible on device (I tried)
    PCF8563_SetDateTime(PCF8563_GetDay(), PCF8563_GetWeekday(), PCF8563_GetMonth(),
                PCF8563_GetCentury(), PCF8563_GetYear(), PCF8563_GetHour(),
                PCF8563_GetMinute(), PCF8563_GetSecond());
}

void PCF8563_GetDateTime(void)
{
    /* Start at beginning, read entire memory in one go */
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT1_ADDR);
    I2C_WaitAck();

    I2C_Start();
    I2C_SendByte(RTCC_R);
    I2C_WaitAck();

    /* As per data sheet, have to read everything all in one operation */
    u8 readBuffer[16] = {0};
    for (u8 i=0; i < 15; i++) {
        readBuffer[i] = I2C_ReceiveByte();
        I2C_Ack();
    }
    readBuffer[15] = I2C_ReceiveByte();
    I2C_NoAck();
    I2C_Stop();

    // status bytes
    status1 = readBuffer[0];
    status2 = readBuffer[1];

    // time bytes
    //0x7f = 0b01111111
    volt_low = readBuffer[2] & RTCC_VLSEC_MASK;  //VL_Seconds
    sec = PCF8563_BcdToDec(readBuffer[2] & ~RTCC_VLSEC_MASK);
    minute = PCF8563_BcdToDec(readBuffer[3] & 0x7f);
    //0x3f = 0b00111111
    hour = PCF8563_BcdToDec(readBuffer[4] & 0x3f);

    // date bytes
    //0x3f = 0b00111111
    day = PCF8563_BcdToDec(readBuffer[5] & 0x3f);
    //0x07 = 0b00000111
    weekday = PCF8563_BcdToDec(readBuffer[6] & 0x07);
    //get raw month data byte and set month and century with it.
    month = readBuffer[7];
    if (month & RTCC_CENTURY_MASK)
        century = 1;
    else
        century = 0;
    //0x1f = 0b00011111
    month = month & 0x1f;
    month = PCF8563_BcdToDec(month);
    year = PCF8563_BcdToDec(readBuffer[8]);

    // printf("PCF8563_GetDateTime: %d-%d-%d\r\n", year, month, day);

    // alarm bytes
    alarm_minute = readBuffer[9];
    if(0x80 & alarm_minute)      // 0b10000000 - check alarm enable bit
        alarm_minute = RTCC_NO_ALARM;
    else
        alarm_minute = PCF8563_BcdToDec(alarm_minute & 0x7F);  // 0b01111111 - mask for minutes
    alarm_hour = readBuffer[10];
    if(0x80 & alarm_hour)        // 0b10000000 - check alarm enable bit
        alarm_hour = RTCC_NO_ALARM;
    else
        alarm_hour = PCF8563_BcdToDec(alarm_hour & 0x3F);      // 0b00111111 - mask for hours
    alarm_day = readBuffer[11];
    if(0x80 & alarm_day)         // 0b10000000 - check alarm enable bit
        alarm_day = RTCC_NO_ALARM;
    else
        alarm_day = PCF8563_BcdToDec(alarm_day & 0x3F);        // 0b00111111 - mask for days
    alarm_weekday = readBuffer[12];
    if(0x80 & alarm_weekday)     // 0b10000000 - check alarm enable bit
        alarm_weekday = RTCC_NO_ALARM;
    else
        alarm_weekday = PCF8563_BcdToDec(alarm_weekday & 0x07); // 0b00000111 - mask for weekdays

    // CLKOUT_control 0x03 = 0b00000011
    squareWave = readBuffer[13] & 0x03;

    // timer bytes
    timer_control = readBuffer[14] & 0x03;
    timer_value = readBuffer[15];  // current value != set value when running
}

void PCF8563_SetDateTime(u8 day, u8 weekday, u8 month, u8 century, u8 year,
                     u8 hour, u8 minute, u8 sec)
{
    /* year val is 00 to 99, xx
        with the highest bit of month = century
        0=20xx
        1=19xx
        */
    month = PCF8563_DecToBcd(month);
    if (century)
        month |= RTCC_CENTURY_MASK;
    else
        month &= ~RTCC_CENTURY_MASK;

    /* As per data sheet, have to set everything all in one operation */
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_SEC_ADDR);       // send addr low byte, req'd
    I2C_WaitAck();
    I2C_SendByte(PCF8563_DecToBcd(sec) &~RTCC_VLSEC_MASK); //set sec, clear VL bit
    I2C_WaitAck();
    I2C_SendByte(PCF8563_DecToBcd(minute));    //set minutes
    I2C_WaitAck();
    I2C_SendByte(PCF8563_DecToBcd(hour));        //set hour
    I2C_WaitAck();
    I2C_SendByte(PCF8563_DecToBcd(day));            //set day
    I2C_WaitAck();
    I2C_SendByte(PCF8563_DecToBcd(weekday));    //set weekday
    I2C_WaitAck();
    I2C_SendByte(month);                 //set month, century to 1
    I2C_WaitAck();
    I2C_SendByte(PCF8563_DecToBcd(year));        //set year to 99
    I2C_WaitAck();
    I2C_Stop();
    // Keep values in-sync with device
    PCF8563_GetDateTime();
}

/* Getter functions */
u8 PCF8563_GetVoltLow(void) { return volt_low; }
u8 PCF8563_GetSecond(void) { return sec; }
u8 PCF8563_GetMinute(void) { return minute; }
u8 PCF8563_GetHour(void) { return hour; }
u8 PCF8563_GetDay(void) { return day; }
u8 PCF8563_GetMonth(void) { return month; }
u8 PCF8563_GetYear(void) { return year; }
u8 PCF8563_GetCentury(void) { return century; }
u8 PCF8563_GetWeekday(void) { return weekday; }
u8 PCF8563_GetStatus1(void) { return status1; }
u8 PCF8563_GetStatus2(void) { return status2; }
u8 PCF8563_GetAlarmMinute(void) { return alarm_minute; }
u8 PCF8563_GetAlarmHour(void) { return alarm_hour; }
u8 PCF8563_GetAlarmDay(void) { return alarm_day; }
u8 PCF8563_GetAlarmWeekday(void) { return alarm_weekday; }
u8 PCF8563_GetTimerControl(void) { return timer_control; }
u8 PCF8563_GetTimerValue(void) { return timer_value; }

/* Helper functions */
int PCF8563_LeapDaysBetween(u8 century_start, u8 year_start,
                        u8 century_end, u8 year_end)
{
    // Credit: Victor Haydin via stackoverflow.com
    int span_start = 2000 - (century_start * 100) + year_start;
    int span_end = 2000 - (century_end * 100) + year_end - 1;  // less year_end
    // Subtract leap-years before span_start, from leap-years before span_end
    return ((span_end / 4) - (span_end / 100) + (span_end / 400)) -
           ((span_start / 4) - (span_start / 100) + (span_start / 400));
}

u8 PCF8563_IsLeapYear(u8 century, int year)
{
    year = 2000 - (century * 100) + year;
    if ((year % 4) != 0)
        return 0;
    else if ((year % 100) != 0)
        return 1;
    else if ((year % 400) != 0)
        return 0;
    else
        return 1;
}

u8 PCF8563_DaysInMonth(u8 century, u8 year, u8 month)
{
    const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    u8 dim = days[month];
    if (month == 2 && PCF8563_IsLeapYear(century, year))
        dim += 1;
    return dim;
}

u8 PCF8563_DaysInYear(u8 century, u8 year, u8 month, u8 day)
{
    const int days[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    u8 total = days[month - 1] + day;
    if ((month > 2) && PCF8563_IsLeapYear(century, year))
        total += 1;
    return total;
}

u8 PCF8563_WhatWeekday(u8 day, u8 month, u8 century, int year)
{
    year = 2000 - (century * 100) + year;
    // Credit: Tomohiko Sakamoto
    // http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
    year -= month < 3;
    static int trans[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    return (year + year / 4 - year / 100 + year / 400 +
            trans[month - 1] + day) % 7;
}

/* Alarm functions */
void PCF8563_GetAlarm(void)
{
    PCF8563_GetDateTime();
}

u8 PCF8563_AlarmEnabled(void)
{
    return PCF8563_GetStatus2() & RTCC_ALARM_AIE;
}

u8 PCF8563_AlarmActive(void)
{
    return PCF8563_GetStatus2() & RTCC_ALARM_AF;
}

void PCF8563_EnableAlarm(void)
{
    PCF8563_GetDateTime();  // operate on current values
    //set status2 AF val to zero
    status2 &= ~RTCC_ALARM_AF;
    //set TF to 1 masks it from changing, as per data-sheet
    status2 |= RTCC_TIMER_TF;
    //enable the interrupt
    status2 |= RTCC_ALARM_AIE;

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();
}

void PCF8563_SetAlarm(u8 min, u8 hour, u8 day, u8 weekday)
{
    PCF8563_GetDateTime();  // operate on current values
    
    if (min < 99) {
        if (min > 59) min = 59;
        min = PCF8563_DecToBcd(min);
        min &= ~RTCC_ALARM;
    } else {
        min = 0x0; min |= RTCC_ALARM;
    }

    if (hour < 99) {
        if (hour > 23) hour = 23;
        hour = PCF8563_DecToBcd(hour);
        hour &= ~RTCC_ALARM;
    } else {
        hour = 0x0; hour |= RTCC_ALARM;
    }

    if (day < 99) {
        if (day < 1) day = 1;
        if (day > 31) day = 31;
        day = PCF8563_DecToBcd(day);
        day &= ~RTCC_ALARM;
    } else {
        day = 0x0; day |= RTCC_ALARM;
    }

    if (weekday < 99) {
        if (weekday > 6) weekday = 6;
        weekday = PCF8563_DecToBcd(weekday);
        weekday &= ~RTCC_ALARM;
    } else {
        weekday = 0x0; weekday |= RTCC_ALARM;
    }

    alarm_hour = hour;
    alarm_minute = min;
    alarm_weekday = weekday;
    alarm_day = day;

    // First set alarm values, then enable
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_ALRM_MIN_ADDR);
    I2C_WaitAck();
    I2C_SendByte(alarm_minute);
    I2C_WaitAck();
    I2C_SendByte(alarm_hour);
    I2C_WaitAck();
    I2C_SendByte(alarm_day);
    I2C_WaitAck();
    I2C_SendByte(alarm_weekday);
    I2C_WaitAck();
    I2C_Stop();

    PCF8563_EnableAlarm();
}

void PCF8563_ClearAlarm(void)
{
    //set status2 AF val to zero to reset alarm
    status2 &= ~RTCC_ALARM_AF;
    //set TF to 1 masks it from changing, as per data-sheet
    status2 |= RTCC_TIMER_TF;
    //turn off the interrupt
    status2 &= ~RTCC_ALARM_AIE;

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();
}

void PCF8563_ResetAlarm(void)
{
    //set status2 AF val to zero to reset alarm
    status2 &= ~RTCC_ALARM_AF;
    //set TF to 1 masks it from changing, as per data-sheet
    status2 |= RTCC_TIMER_TF;

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();
}

/* Timer functions */
u8 PCF8563_TimerEnabled(void)
{
    if (PCF8563_GetStatus2() & RTCC_TIMER_TIE)
        if (timer_control & RTCC_TIMER_TE)
            return 1;
    return 0;
}

u8 PCF8563_TimerActive(void)
{
    return PCF8563_GetStatus2() & RTCC_TIMER_TF;
}

void PCF8563_EnableTimer(void)
{
    PCF8563_GetDateTime();
    //set TE to 1
    timer_control |= RTCC_TIMER_TE;
    //set status2 TF val to zero
    status2 &= ~RTCC_TIMER_TF;
    //set AF to 1 masks it from changing, as per data-sheet
    status2 |= RTCC_ALARM_AF;
    //enable the interrupt
    status2 |= RTCC_TIMER_TIE;

    // Enable interrupt first, then enable timer
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_TIMER1_ADDR);
    I2C_WaitAck();
    I2C_SendByte(timer_control);  // Timer starts ticking now!
    I2C_WaitAck();
    I2C_Stop();
}

void PCF8563_SetTimer(u8 value, u8 frequency, u8 is_pulsed)
{
    PCF8563_GetDateTime();
    if (is_pulsed)
        status2 |= is_pulsed << 4;
    else
        status2 &= ~(is_pulsed << 4);
    timer_value = value;
    // TE set to 1 in enableTimer(), leave 0 for now
    timer_control |= (frequency & RTCC_TIMER_TD10); // use only last 2 bits

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_TIMER1_ADDR);
    I2C_WaitAck();
    I2C_SendByte(timer_control);
    I2C_WaitAck();
    I2C_SendByte(timer_value);
    I2C_WaitAck();
    I2C_Stop();

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();

    PCF8563_EnableTimer();
}

void PCF8563_ClearTimer(void)
{
    PCF8563_GetDateTime();
    //set status2 TF val to zero
    status2 &= ~RTCC_TIMER_TF;
    //set AF to 1 masks it from changing, as per data-sheet
    status2 |= RTCC_ALARM_AF;
    //turn off the interrupt
    status2 &= ~RTCC_TIMER_TIE;
    //turn off the timer
    timer_control = 0;

    // Stop timer first
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_TIMER1_ADDR);
    I2C_WaitAck();
    I2C_SendByte(timer_control);
    I2C_WaitAck();
    I2C_Stop();

    // clear flag and interrupt
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();
}

void PCF8563_ResetTimer(void)
{
    PCF8563_GetDateTime();
    //set status2 TF val to zero to reset timer
    status2 &= ~RTCC_TIMER_TF;
    //set AF to 1 masks it from changing, as per data-sheet
    status2 |= RTCC_ALARM_AF;

    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_STAT2_ADDR);
    I2C_WaitAck();
    I2C_SendByte(status2);
    I2C_WaitAck();
    I2C_Stop();
}

/* Square wave functions */
void PCF8563_SetSquareWave(u8 frequency)
{
    I2C_Start();
    I2C_SendByte(RTCC_W);
    I2C_WaitAck();
    I2C_SendByte(RTCC_SQW_ADDR);
    I2C_WaitAck();
    I2C_SendByte(frequency);
    I2C_WaitAck();
    I2C_Stop();
}

void PCF8563_ClearSquareWave(void)
{
    PCF8563_SetSquareWave(SQW_DISABLE);
}

/* Time/Date formatting functions */
const char *PCF8563_FormatTime(u8 style)
{
    if (style == RTCC_TIME_HMS)
        sprintf(strOut, "%02d:%02d:%02d", hour, minute, sec);
    else
        sprintf(strOut, "%02d:%02d", hour, minute);
    return strOut;
}

const char *PCF8563_FormatDate(u8 style)
{
    if (style == RTCC_DATE_WORLD)
        sprintf(strDate, "%02d-%02d-%02d", year, month, day);
    else if (style == RTCC_DATE_ASIA)
        sprintf(strDate, "%02d-%02d-%02d", month, day, year);
    else
        sprintf(strDate, "%02d-%02d-%02d", day, month, year);
    return strDate;
}

/* Timestamp function */
u32 PCF8563_GetTimestamp(void)
{
    u32 timestamp = EPOCH_TIMESTAMP;
    u8 days = 0;
    
    // Add days from epoch to current year
    for (u8 y = epoch_year; y < year; y++) {
        if (PCF8563_IsLeapYear(century, y))
            days += 366;
        else
            days += 365;
    }
    
    // Add days from current year
    days += PCF8563_DaysInYear(century, year, month, day) - 1;
    
    // Convert to seconds and add time
    timestamp += (days * 86400) + (hour * 3600) + (minute * 60) + sec;
    
    return timestamp;
}

/* Time/Date get/set functions */
void PCF8563_InitClock(void)
{
    PCF8563_ZeroClock();
    PCF8563_ClearStatus();
}

void PCF8563_SetTime(u8 hour, u8 minute, u8 sec)
{
    PCF8563_SetDateTime(PCF8563_GetDay(), PCF8563_GetWeekday(), PCF8563_GetMonth(),
                    PCF8563_GetCentury(), PCF8563_GetYear(), hour, minute, sec);
}

void PCF8563_GetTime(void)
{
    PCF8563_GetDateTime();
}

void PCF8563_SetDate(u8 day, u8 weekday, u8 month, u8 century, u8 year)
{
    PCF8563_SetDateTime(day, weekday, month, century, year,
                    PCF8563_GetHour(), PCF8563_GetMinute(), PCF8563_GetSecond());
}

void PCF8563_GetDate(void)
{
    PCF8563_GetDateTime();
} 