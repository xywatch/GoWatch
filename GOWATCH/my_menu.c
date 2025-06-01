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
    doAction(false); // 执行指令
    // menuData.isOpen = false;  //关闭菜单
}

// 关机
void ShutDown(void)
{
    // display_startCRTAnim(CRTANIM_CLOSE);
    GPIO_ResetBits(POWER_ON_PORT, POWER_ON_PIN);
}

// deep sleep后怎么唤醒呢?
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
    menuData.isOpen = false; // 关闭菜单
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
    menuData.isOpen = true; // 打开菜单

    display_setDrawFunc(menu_draw); // 绑定绘制函数为menu_draw

    buttons_setFuncs(menu_up, menu_select, menu_down); // 绑定按键功能函数

    setMenuInfo(OPTION_COUNT, MENU_TYPE_STR, PSTR(STR_WIFICMDMENU)); // 获取当前菜单信息（选项个数，菜单类型是文字还是图标）
    setMenuFuncs(MENUFUNC_NEXT, mSelect, MENUFUNC_PREV, itemLoader); // 绑定菜单的函数,如前进后退选择确认
    showMeThenRun(NULL);
    return true;
}

// 实现表盘从上往下退出, 然后执行my_menu_open
bool my_menu_open2(void)
{
    if (!animation_active() || animation_movingOn())
    {
        exitMeThenRun(my_menu_open);
    }
    return true;
}
