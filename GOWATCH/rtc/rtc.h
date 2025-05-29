#ifndef RTC_H
#define RTC_H

#include "sys.h"
#include "pcf8563.h"
#include "typedefs.h"

#define RTC_PCF_ADDR    0x51
#define YEAR_OFFSET_PCF 2000

// Function declarations
void RTC_Init(void);
void RTC_Config(const char* datetime);  // Format: YYYY:MM:DD:HH:MM:SS
void RTC_ClearAlarm(void);
void RTC_SetNextMinuteAlarm(void);
void RTC_Read_Datetime(timeDate_s* timeDate);
void RTC_Set_Datetime(timeDate_s* timeDate);
void RTC_Set_Alarm(u8 minute, u8 hour, u8 wday);
u8 RTC_GetTemperature(void);
void RTC_PrintAlarm(void);
// Helper functions
static void RTC_PCFConfig(const char* datetime);
static const char* RTC_GetValue(const char* data, char separator, int index);

#endif
