/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

// Deals with sleeping and waking up/��Դ����

#include "common.h"

#define BATTERY_CUTOFF 2800

typedef enum
{
    SYS_AWAKE, // ����״̬(����)
    SYS_CRTANIM, // CRT����״̬
    SYS_SLEEP // ˯��״̬(��û�õ�)
} sys_t;

typedef enum
{
    USER_ACTIVE, // �û���Ծ״̬(����), һֱ�����״̬
    USER_INACTIVE // �û��ǻ�Ծ״̬(��û�õ�)
} user_t;

static sys_t systemState;
static user_t userState;

static void batteryCutoff(void);

/*
systemState ״̬ת�����̣�
1. ����ʱpwrmgr_init()��SYS_AWAKE
2. �޲���ʱ��SYS_AWAKE -> SYS_CRTANIM����ʼϢ��������
3. �а�ť����ʱ��SYS_CRTANIM -> SYS_AWAKE��ȡ��Ϣ��������
4. ���������󣺱����� SYS_CRTANIM ״̬���ȴ�����˯��ģʽ
���״̬����Ҫ���ڿ���ϵͳ����ʾ��˯��״̬����϶���Ч��ʵ��ƽ���Ŀ���������
*/

void pwrmgr_init()
{
    systemState = SYS_AWAKE;
    userState = USER_ACTIVE;
    // set_sleep_mode(SLEEP_MODE_IDLE);
}

bool keep_on = 0;

bool SleepRequested; // �Ƿ���������, ��Ϣ����������Ϊ�Ϳ���������, Ȼ����main.c���ִ������

// c_loop()��ѭ��ִ��
/*
    1. ��ص�ѹ����2.8Vʱ���ر����л���Դ������˯��ģʽ (��û�õ�)
    2. �а�ť�ʱ��������Ļ����
    3. �ް�ť�ʱ����ʼϢ������
    4. ���������󣬽���˯��ģʽ
    5. �а�ť����ʱ������ϵͳ
    6. ���������󣬱����� SYS_CRTANIM ״̬���ȴ�����˯��ģʽ
*/
void pwrmgr_update()
{
    // batteryCutoff();

    bool buttonsActive = buttons_isActive() || keep_on;

    // �����Ϣ��Ļ, ��һֱû�ж���, ������һֱ����
    // û�в���, 2���Ӻ��Ϣ��Ļ, ���������һֱ����Ļ
    if (keep_on) {
        if(millis() - buttons_lastPressedTime() > 2 * 60 * 1000) {
            buttonsActive = false;
        }
    }

    // ��ť���������Ļ����
    if (buttonsActive)
    {
        SleepRequested = false;
        // ���֮ǰ��Ϣ�������������а�ť�����ȡ��Ϣ������, ������Ļ
        if (systemState == SYS_CRTANIM && buttonsActive)
        { 
            // ȡ��Ϣ������
            display_startCRTAnim(CRTANIM_OPEN);
            systemState = SYS_AWAKE;
        }
        else // ����˯��ģʽ
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
        // û�а�ť�����ʼϢ������
        if (systemState == SYS_AWAKE)
        {
            printf("û�а�ť�����ʼϢ������\r\n");
            systemState = SYS_CRTANIM;
            display_startCRTAnim(CRTANIM_CLOSE);
        }
        // ��Ϣ��, ��ȴ�������������˯��ģʽ
        else if (systemState == SYS_CRTANIM)
        {
            // printf("��Ϣ��, ��ȴ�������������˯��ģʽ\r\n");
            // �ȶ�����������sleep mode
            if (!animation_active() && !display_is_ani_active()) {
                printf("��������������˯��ģʽ\r\n");
                SleepRequested = true;
            }

            // time_sleep();
            systemState = SYS_CRTANIM;

            // �����м�, ����û��alrm awake
            // ���м�, ���
            // ����Ӧ���� == �����м����� RTCWAKE_SYSTEM
            if (time_wake() == RTCWAKE_SYSTEM) // Woken by button press, USB plugged in or by RTC user alarm
            {
                printf("�����м�������ϵͳ\r\n");
                userWake();
            }
        }
    }
}

// �ж��û��Ƿ��Ծ
// ��Ϊ�û�һֱ�ǻ�Ծ��, ����һֱ����true
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
