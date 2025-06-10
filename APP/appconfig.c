#include "common.h"
#include "stmflash.h"

#define EEPROM_CHECK_NUM 0x68 // Any 8 bit number that isn't 0 or 255

/*
Flash总大小：128KB (0x20000)
页大小：2KB (0x800)
总页数：64页
最后一页的地址计算：
起始地址：0x08000000
最后一页起始地址 = 起始地址 + (总页数-1) * 页大小
= 0x08000000 + (64-1) * 0x800
= 0x08000000 + 63 * 0x800
= 0x08000000 + 0x1F800
= 0x0801F800
所以：
最后一页起始地址：0x0801F800
最后一页结束地址：0x0801FFFF
最后一页大小：2KB (0x800)
*/

// 这是额外64k的地址了
#define appConfig_SAVE_ADDR 0x0801F800
appconfig_s appConfig;

void appconfig_init()
{
    appConfig = *((appconfig_s *)malloc(sizeof(appconfig_s)));
    memset(&appConfig, 0x00, sizeof(appconfig_s));

    // appconfig_reset();

    STMFLASH_Read(appConfig_SAVE_ADDR, (uint32_t *)(&appConfig), sizeof(appconfig_s));

    // 如果之前设置过appconfig, 则使用之
    if (appConfig.flashCheck == EEPROM_CHECK_NUM)
    {
        printf("之前有 appconfig_init: appConfig: %d, flashCheck=%d\n", appConfig.sleepTimeout, appConfig.flashCheck);
    }
    else
    {
        printf("之前没有 appconfig_init: appConfig: %d, flashCheck=%d\n", appConfig.sleepTimeout, appConfig.flashCheck);
        appconfig_reset();
    }
}

void appconfig_save()
{
    STMFLASH_Write(appConfig_SAVE_ADDR, (uint32_t*)(&appConfig), sizeof(appconfig_s));
    printf("appconfig_save保存成功\n");
}

void appconfig_reset()
{
    appConfig.flashCheck = EEPROM_CHECK_NUM;
    appConfig.brightness = 1;
    appConfig.sleepTimeout = 2; // 4: 20s; 3: 15s; 2: 10s
    appConfig.invert = false;
#if COMPILE_ANIMATIONS
    appConfig.animations = true;
#endif
    appConfig.display180 = false;
    appConfig.CTRL_LEDs = false;
    appConfig.showFPS = false;
    
    appConfig.tiltWrist = true;
    appConfig.doubleTap = true;

    appConfig.timeMode = TIMEMODE_24HR;

    appConfig.volUI = 1;
    appConfig.volAlarm = 2;
    appConfig.volHour = 1;

    // alarms
    // 22:45:00, 127 = 1111111, 表示星期1,2,3,4,5,6,7, 255=1(开启) 111111(周二)1(周一), 表示所有星期且开启
    // 63 = 111111, 表示星期1,2,3,4,5,6   7 = 111, 表示星期1,2,3
    appConfig.alarms[0].hour = 9;
    appConfig.alarms[0].min = 0;
    appConfig.alarms[0].days = 127;

    appConfig.alarms[1].hour = 10;
    appConfig.alarms[1].min = 0;
    appConfig.alarms[1].days = 127;

    appConfig.alarms[2].hour = 7;
    appConfig.alarms[2].min = 0;
    appConfig.alarms[2].days = 63;

    appConfig.alarms[3].hour = 9;
    appConfig.alarms[3].min = 0;
    appConfig.alarms[3].days = 0;

    appConfig.alarms[4].hour = 3;
    appConfig.alarms[4].min = 0;
    appConfig.alarms[4].days = 7;

    appconfig_save();
}
