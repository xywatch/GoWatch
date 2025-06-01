/*****
 *  NAME
 *    Pcf8563 Real Time Clock support routines
 *  AUTHOR
 *    Joe Robertson, jmr
 *    orbitalair@bellsouth.net
 *    http://orbitalair.wikispaces.com/Arduino
 *  CREATION DATE
 *    9/24/06,  init - built off of usart demo.  using mikroC
 *  NOTES
 *  HISTORY
 *    10/14/06 ported to CCS compiler, jmr
 *    2/21/09  changed all return values to hex val and not bcd, jmr
 *    1/10/10  ported to arduino, jmr
 *    2/14/10  added 3 world date formats, jmr
 *    28/02/2012 A. Pasotti
 *             fixed a bug in RTCC_ALARM_AF,
 *             added a few (not really useful) methods
 *    22/10/2014 Fix whitespace, tabs, and newlines, cevich
 *    22/10/2014 add voltLow get/set, cevich
 *    22/10/2014 add century get, cevich
 *    22/10/2014 Fix get/set date/time race condition, cevich
 *    22/10/2014 Header/Code rearranging, alarm/timer flag masking,
 *               extern Wire, cevich
 *    26/11/2014 Add zeroClock(), initialize to lowest possible
 *               values, cevich
 *    22/10/2014 add timer support, cevich
 *    24/01/2018 add getTimestamp, gilou
 *
 *  TODO
 *    x Add Euro date format
 *    Add short time (hh:mm) format
 *    Add 24h/12h format
 ******
 *  Robodoc embedded documentation.
 *  http://www.xs4all.nl/~rfsber/Robo/robodoc.html
 */

#ifndef Rtc_Pcf8563_H
#define Rtc_Pcf8563_H

#include "sys.h"
#include "i2c_soft.h"

/* the read and write values for pcf8563 rtcc */
/* these are adjusted for stm32 */
// PCF8563 的 I2C 地址（7-bit）
// 写地址：0xA2（1010001 + 0 写位）
// 读地址：0xA3（1010001 + 1 读位）
#define PCF8563_ADDR 0x51
#define RTCC_R      0xa3 // 读取地址 = 0x51 << 1 | 1
#define RTCC_W      0xa2 // 写入地址 = 0x51 << 1 | 0

#define RTCC_SEC        1
#define RTCC_MIN        2
#define RTCC_HR         3
#define RTCC_DAY        4
#define RTCC_WEEKDAY    5
#define RTCC_MONTH      6
#define RTCC_YEAR       7
#define RTCC_CENTURY    8

/* register addresses in the rtc */
#define RTCC_STAT1_ADDR     0x0
#define RTCC_STAT2_ADDR     0x01
#define RTCC_SEC_ADDR       0x02
#define RTCC_MIN_ADDR       0x03
#define RTCC_HR_ADDR        0x04
#define RTCC_DAY_ADDR       0x05
#define RTCC_WEEKDAY_ADDR   0x06
#define RTCC_MONTH_ADDR     0x07
#define RTCC_YEAR_ADDR      0x08
#define RTCC_ALRM_MIN_ADDR  0x09
#define RTCC_SQW_ADDR       0x0D
#define RTCC_TIMER1_ADDR    0x0E
#define RTCC_TIMER2_ADDR    0x0F

/* setting the alarm flag to 0 enables the alarm.
 * set it to 1 to disable the alarm for that value.
 */
#define RTCC_ALARM          0x80
#define RTCC_ALARM_AIE      0x02
#define RTCC_ALARM_AF       0x08
/* optional val for no alarm setting */
#define RTCC_NO_ALARM       99

#define RTCC_TIMER_TIE      0x01  // Timer Interrupt Enable

#define RTCC_TIMER_TF       0x04  // Timer Flag, read/write active state
                                  // When clearing, be sure to set RTCC_TIMER_AF
                                  // to 1 (see note above).
#define RTCC_TIMER_TI_TP    0x10  // 0: INT is active when TF is active
                                  //    (subject to the status of TIE)
                                  // 1: INT pulses active
                                  //    (subject to the status of TIE);
                                  // Note: TF stays active until cleared
                                  // no matter what RTCC_TIMER_TI_TP is.
#define RTCC_TIMER_TD10     0x03  // Timer source clock, TMR_1MIN saves power
#define RTCC_TIMER_TE       0x80  // Timer 1:enable/0:disable

/* Timer source-clock frequency constants */
#define TMR_4096HZ      0x00    // 0b00000000
#define TMR_64Hz        0x01    // 0b00000001
#define TMR_1Hz         0x02    // 0b00000010
#define TMR_1MIN        0x03    // 0b00000011

#define RTCC_CENTURY_MASK   0x80
#define RTCC_VLSEC_MASK     0x80

/* date format flags */
#define RTCC_DATE_WORLD     0x01
#define RTCC_DATE_ASIA      0x02
#define RTCC_DATE_US        0x04
/* time format flags */
#define RTCC_TIME_HMS       0x01
#define RTCC_TIME_HM        0x02

/* square wave constants */
#define SQW_DISABLE     0x00    // 0b00000000
#define SQW_32KHZ       0x80    // 0b10000000
#define SQW_1024HZ      0x81    // 0b10000001
#define SQW_32HZ        0x82    // 0b10000010
#define SQW_1HZ         0x83    // 0b10000011

/* epoch timestamp constants : 01/01/2016 脿 00:00:00 : 1451599200 */
#define epoch_day	1
#define epoch_month	1
#define epoch_year	16
#define EPOCH_TIMESTAMP 1451606400
extern const unsigned int months_days[];	// days count for each month

/* Function declarations */
void PCF8563_Init(void);
void PCF8563_ZeroClock(void);
void PCF8563_ClearStatus(void);
u8 PCF8563_ReadStatus2(void);
void PCF8563_ClearVoltLow(void);
void PCF8563_GetDateTime(void);
void PCF8563_SetDateTime(u8 day, u8 weekday, u8 month, u8 century, u8 year,
                     u8 hour, u8 minute, u8 sec);
void PCF8563_GetAlarm(void);
u8 PCF8563_AlarmEnabled(void);
u8 PCF8563_AlarmActive(void);
void PCF8563_EnableAlarm(void);
void PCF8563_SetAlarm(u8 min, u8 hour, u8 day, u8 weekday);
void PCF8563_ClearAlarm(void);
void PCF8563_ResetAlarm(void);
u8 PCF8563_TimerEnabled(void);
u8 PCF8563_TimerActive(void);
void PCF8563_EnableTimer(void);
void PCF8563_SetTimer(u8 value, u8 frequency, u8 is_pulsed);
void PCF8563_ClearTimer(void);
void PCF8563_ResetTimer(void);
void PCF8563_SetSquareWave(u8 frequency);
void PCF8563_ClearSquareWave(void);
const char *PCF8563_FormatTime(u8 style);
const char *PCF8563_FormatDate(u8 style);
u8 PCF8563_GetVoltLow(void);
u8 PCF8563_GetSecond(void);
u8 PCF8563_GetMinute(void);
u8 PCF8563_GetHour(void);
u8 PCF8563_GetDay(void);
u8 PCF8563_GetMonth(void);
u8 PCF8563_GetYear(void);
u8 PCF8563_GetCentury(void);
u8 PCF8563_GetWeekday(void);
u8 PCF8563_GetStatus1(void);
u8 PCF8563_GetStatus2(void);
u8 PCF8563_GetAlarmMinute(void);
u8 PCF8563_GetAlarmHour(void);
u8 PCF8563_GetAlarmDay(void);
u8 PCF8563_GetAlarmWeekday(void);
u8 PCF8563_GetTimerControl(void);
u8 PCF8563_GetTimerValue(void);
u32 PCF8563_GetTimestamp(void);
void PCF8563_InitClock(void);
void PCF8563_SetTime(u8 hour, u8 minute, u8 sec);
void PCF8563_GetTime(void);
void PCF8563_SetDate(u8 day, u8 weekday, u8 month, u8 century, u8 year);
void PCF8563_GetDate(void);

/* Helper functions */
u8 PCF8563_DecToBcd(u8 value);
u8 PCF8563_BcdToDec(u8 value);
int PCF8563_LeapDaysBetween(u8 century_start, u8 year_start,
                        u8 century_end, u8 year_end);
u8 PCF8563_IsLeapYear(u8 century, int year);
u8 PCF8563_DaysInMonth(u8 century, u8 year, u8 month);
u8 PCF8563_DaysInYear(u8 century, u8 year, u8 month, u8 day);
u8 PCF8563_WhatWeekday(u8 day, u8 month, u8 century, int year);

#endif
