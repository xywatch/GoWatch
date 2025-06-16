/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

// Deals with sleeping and waking up/电源管理

#include "common.h"

#define BATTERY_CUTOFF 2800

typedef enum
{
    SYS_AWAKE, // 唤醒状态(开机)
    SYS_CRTANIM, // CRT动画状态
    SYS_SLEEP // 睡眠状态(暂没用到)
} sys_t;

typedef enum
{
    USER_ACTIVE, // 用户活跃状态(开机), 一直是这个状态
    USER_INACTIVE // 用户非活跃状态(暂没用到)
} user_t;

static sys_t systemState;
static user_t userState;

static void batteryCutoff(void);

/*
systemState 状态转换流程：
1. 开机时pwrmgr_init()：SYS_AWAKE
2. 无操作时：SYS_AWAKE -> SYS_CRTANIM（开始息屏动画）
3. 有按钮操作时：SYS_CRTANIM -> SYS_AWAKE（取消息屏动画）
4. 动画结束后：保持在 SYS_CRTANIM 状态，等待进入睡眠模式
这个状态机主要用于控制系统的显示和睡眠状态，配合动画效果实现平滑的开关屏过渡
*/

void pwrmgr_init()
{
    systemState = SYS_AWAKE;
    userState = USER_ACTIVE;
    // set_sleep_mode(SLEEP_MODE_IDLE);
}

bool keep_on = 0;

bool SleepRequested; // 是否请求休眠, 在息屏结束会认为就可以休眠了, 然后在main.c里会执行休眠

// c_loop()里循环执行
/*
    1. 电池电压低于2.8V时，关闭所有唤醒源，进入睡眠模式 (暂没用到)
    2. 有按钮活动时，保持屏幕开启
    3. 无按钮活动时，开始息屏动画
    4. 动画结束后，进入睡眠模式
    5. 有按钮操作时，唤醒系统
    6. 动画结束后，保持在 SYS_CRTANIM 状态，等待进入睡眠模式
*/
void pwrmgr_update()
{
    // batteryCutoff();

    bool buttonsActive = buttons_isActive() || keep_on;

    // 如果不息屏幕, 但一直没有动作, 不可能一直亮着
    // 没有操作, 2分钟后就息屏幕, 避免误操作一直亮屏幕
    if (keep_on) {
        if(millis() - buttons_lastPressedTime() > 2 * 60 * 1000) {
            buttonsActive = false;
        }
    }

    // 按钮活动，保持屏幕开启
    if (buttonsActive)
    {
        SleepRequested = false;
        // 如果之前是息屏动画，现在有按钮活动，则取消息屏动画, 即打开屏幕
        if (systemState == SYS_CRTANIM && buttonsActive)
        { 
            // 取消息屏动画
            display_startCRTAnim(CRTANIM_OPEN);
            systemState = SYS_AWAKE;
        }
        else // 空闲睡眠模式
        {
            //			if(PRR == (_BV(PRTWI)|_BV(PRTIM0)|_BV(PRTIM1)|_BV(PRSPI)|_BV(PRUSART0)|_BV(PRADC))) // No peripherals are in use other than Timer2
            //				set_sleep_mode(SLEEP_MODE_PWR_SAVE); // Also disable BOD?
            //			else
            //				set_sleep_mode(SLEEP_MODE_IDLE);

            //			debugPin_sleepIdle(HIGH);
            //			sleep_mode();
            //			debugPin_sleepIdle(LOW);
        }
    }
    else
    {
        // printf("buttonsActive false\r\n");
        // 没有按钮活动，开始息屏动画
        if (systemState == SYS_AWAKE)
        {
            printf("没有按钮活动，开始息屏动画\r\n");
            systemState = SYS_CRTANIM;
            display_startCRTAnim(CRTANIM_CLOSE);
        }
        // 已息屏, 则等待动画结束进入睡眠模式
        else if (systemState == SYS_CRTANIM)
        {
            // printf("已息屏, 则等待动画结束进入睡眠模式\r\n");
            // 等动画结束进入sleep mode
            if (!animation_active() && !display_is_ani_active()) {
                printf("动画结束，进入睡眠模式\r\n");
                SleepRequested = true;
            }

            // time_sleep();
            systemState = SYS_CRTANIM;

            // 按下中键, 现在没有alrm awake
            // 按中键, 则打开
            // 这里应该是 == 按下中键就是 RTCWAKE_SYSTEM
            if (time_wake() == RTCWAKE_SYSTEM) // Woken by button press, USB plugged in or by RTC user alarm
            {
                printf("按下中键，唤醒系统\r\n");
                userWake();
            }
        }
    }
}

// 判断用户是否活跃
// 因为用户一直是活跃的, 所以一直返回true
bool pwrmgr_userActive()
{
    return userState == USER_ACTIVE;
}

void userWake(void)
{
    userState = USER_ACTIVE;
    buttons_wake();
    display_startCRTAnim(CRTANIM_OPEN);
    // oled_power(OLED_PWR_ON);
    // battery_setUpdate(3);
}

// static void userSleep()
//{
//	userState = USER_INACTIVE;
//	//oled_power(OLED_PWR_OFF);
// }

static void batteryCutoff()
{
    //	// If the battery voltage goes below a threshold then disable
    //	// all wakeup sources apart from USB plug-in and go to power down sleep.
    //	// This helps protect the battery from undervoltage and since the battery's own PCM hasn't kicked in yet the RTC will continue working.

    //	uint voltage = battery_voltage();
    //	if(voltage < BATTERY_CUTOFF && !USB_CONNECTED() && voltage)
    //	{
    //		// Turn everything off
    //		buttons_shutdown();
    //		tune_stop(PRIO_MAX);
    //		led_stop();
    //		oled_power(OLED_PWR_OFF);
    //		time_shutdown();
    //
    //		// Stay in this loop until USB is plugged in or the battery voltage is above the threshold
    //		do
    //		{
    //			set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    //			cli();
    //			sleep_enable();
    //			sleep_bod_disable();
    //			sei();
    //			sleep_cpu();
    //			sleep_disable();
    //
    //			// Get battery voltage
    //			battery_updateNow();

    //		} while(!USB_CONNECTED() && battery_voltage() < BATTERY_CUTOFF);

    //		// Wake up
    //		time_wake();
    //		buttons_startup();
    //		buttons_wake();
    //		oled_power(OLED_PWR_ON);
    //	}
}
