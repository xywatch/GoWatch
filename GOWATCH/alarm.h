/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#ifndef ALARM_H_
#define ALARM_H_
#include "typedefs.h"

#define ALARM_COUNT 5

#define _BV(bit) (1 << (bit))
#define alarm_dayEnabled(days, day) (days & _BV(day))

void alarm_init(void);
void alarm_reset(void);
void alarm_get(byte, alarm_s *);
bool alarm_getNext(alarm_s *);
byte alarm_getNextDay(void);
void alarm_save(byte, alarm_s *);
void alarm_update(void);
void alarm_updateNextAlarm(void);
void alarm_need_updateAlarm_in_nextLoop(void);

extern bool isAlarmTriggered;

#endif /* ALARM_H_ */
