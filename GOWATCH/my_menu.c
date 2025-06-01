#include "common.h"

#define STR_WIFICMDMENU "< Quickstart >"
#define CMD1_NAME "Shut Down"
#define CMD2_NAME "Deep Sleep"
#define Show_Accelerometer "Show Accelerometer"
// #define CMD3_NAME "MPU Display"
// #define CMD4_NAME "History Data"
#define CMD5_NAME "Back"

#define OPTION_COUNT 3

static void mSelect()
{
    doAction(false); // ִ��ָ��
    // menuData.isOpen = false;  //�رղ˵�
}

// �ػ�
void ShutDown(void)
{
    // display_startCRTAnim(CRTANIM_CLOSE);
    GPIO_ResetBits(POWER_ON_PORT, POWER_ON_PIN);
}

// deep sleep����ô������?
// deep sleep
extern bool DeepSleepFlag;
void cmd2(void)
{
    nvic_sleep(2);
}

// mpu display
void cmd3(void)
{
    exitMeThenRun(display_load);
    menuData.isOpen = false; // �رղ˵�
}

// log
extern u8 log_time;
static void LogTimeUpdate()
{
    //	battery_updateNow();
    log_time += 2;

    if (log_time > 15)
    {
        log_time = 1;
    }
}

static void itemLoader(byte num)
{
    char buff[20];
    num = 0;

    setMenuOption_P(num++, PSTR(CMD1_NAME), NULL, ShutDown);

    setMenuOption_P(num++, PSTR(CMD2_NAME), NULL, cmd2);

    // setMenuOption_P(num++, PSTR(CMD3_NAME), NULL, mpu_open);

    // setMenuOption_P(num++, PSTR(CMD4_NAME), NULL, history_display);

    // sprintf_P((char *)buff, PSTR("Log Time  %d min"), log_time);
    // setMenuOption_P(num++, buff, NULL, LogTimeUpdate);

    setMenuOption_P(num++, PSTR(Show_Accelerometer), NULL, showAccelerometer);

    setMenuOption_P(num++, PSTR(CMD5_NAME), NULL, cmd3);
}

bool my_menu_open(void)
{
    menuData.isOpen = true; // �򿪲˵�

    display_setDrawFunc(menu_draw); // �󶨻��ƺ���Ϊmenu_draw

    buttons_setFuncs(menu_up, menu_select, menu_down); // �󶨰������ܺ���

    setMenuInfo(OPTION_COUNT, MENU_TYPE_STR, PSTR(STR_WIFICMDMENU)); // ��ȡ��ǰ�˵���Ϣ��ѡ��������˵����������ֻ���ͼ�꣩
    setMenuFuncs(MENUFUNC_NEXT, mSelect, MENUFUNC_PREV, itemLoader); // �󶨲˵��ĺ���,��ǰ������ѡ��ȷ��
    showMeThenRun(NULL);
    return true;
}

// ʵ�ֱ��̴��������˳�, Ȼ��ִ��my_menu_open
bool my_menu_open2(void)
{
    if (!animation_active() || animation_movingOn())
    {
        exitMeThenRun(my_menu_open);
    }
    return true;
}
