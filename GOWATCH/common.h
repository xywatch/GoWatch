/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "config.h"
#include "util.h"
#include "typedefs.h"
#include "debug.h"

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "oled.h"
#include "buttons.h"
#include "millis.h"
#include "functions.h"
#include "alarms.h"
#include "m_display.h"
#include "games.h"
#include "timedate.h"
#include "settings.h"
#include "sleep.h"
#include "sound.h"
#include "m_main.h"
#include "game1.h"
#include "game2.h"
#include "game3.h"
#include "game_snake.h"
#include "gamelife.h"
#include "stopwatch.h"
#include "torch.h"
#include "watchface.h"
#include "tunemaker.h"
#include "calendar.h"

#include "i2c_soft.h"
#include "adc.h"
#include "nvic.h"
#include "oled_driver.h"

// #include "system.h"
#include "global.h"
#include "display.h"
#include "rtc.h"
#include "time.h"
#include "alarm.h"
#include "pwrmgr.h"
#include "appconfig.h"
#include "tune.h"
#include "animation.h"
#include "draw.h"
#include "menu.h"

#include "english.h"
#include "lang.h"
#include "tunes.h"
#include "resources.h"

#include "wait_data_clock.h"
#include "gui_log_console.h"

#endif /* COMMON_H_ */
