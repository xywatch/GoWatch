#include "common.h"
#include "stmflash.h"

#define EEPROM_CHECK_NUM 0x68 // Any 8 bit number that isn't 0 or 255

/*
Flash�ܴ�С��128KB (0x20000)
ҳ��С��2KB (0x800)
��ҳ����64ҳ
���һҳ�ĵ�ַ���㣺
��ʼ��ַ��0x08000000
���һҳ��ʼ��ַ = ��ʼ��ַ + (��ҳ��-1) * ҳ��С
= 0x08000000 + (64-1) * 0x800
= 0x08000000 + 63 * 0x800
= 0x08000000 + 0x1F800
= 0x0801F800
���ԣ�
���һҳ��ʼ��ַ��0x0801F800
���һҳ������ַ��0x0801FFFF
���һҳ��С��2KB (0x800)
*/

// ���Ƕ���64k�ĵ�ַ��
#define appConfig_SAVE_ADDR 0x0801F800
appconfig_s appConfig;

void appconfig_init()
{
    appConfig = *((appconfig_s *)malloc(sizeof(appconfig_s)));
    memset(&appConfig, 0x00, sizeof(appconfig_s));

    // appconfig_reset();

    STMFLASH_Read(appConfig_SAVE_ADDR, (uint32_t *)(&appConfig), sizeof(appconfig_s));

    // ���֮ǰ���ù�appconfig, ��ʹ��֮
    if (appConfig.flashCheck == EEPROM_CHECK_NUM)
    {
        printf("֮ǰ�� appconfig_init: appConfig: %d, flashCheck=%d\n", appConfig.sleepTimeout, appConfig.flashCheck);
    }
    else
    {
        printf("֮ǰû�� appconfig_init: appConfig: %d, flashCheck=%d\n", appConfig.sleepTimeout, appConfig.flashCheck);
        appconfig_reset();
    }
}

void appconfig_save()
{
    STMFLASH_Write(appConfig_SAVE_ADDR, (uint32_t*)(&appConfig), sizeof(appconfig_s));
    printf("appconfig_save����ɹ�\n");
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
    // 22:45:00, 127 = 1111111, ��ʾ����1,2,3,4,5,6,7, 255=1(����) 111111(�ܶ�)1(��һ), ��ʾ���������ҿ���
    // 63 = 111111, ��ʾ����1,2,3,4,5,6   7 = 111, ��ʾ����1,2,3
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
