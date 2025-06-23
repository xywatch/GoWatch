#include "common.h"
#include "stmflash.h"

#define EEPROM_CHECK_NUM 0x10 // Any 8 bit number that isn't 0 or 255

// ���洢������ʼ��ַ���� 0X08000000, stm32f103c8t6 flash��С64K����ַ��Χ 0x08000000 - 0x0800FFFF 
// 1 STM32F103C8T6��STM32F103CBT6 ������ͬ��Ψһ��������ǰ��Ϊ64kflash��0x8000000~0x800FFFF�� 
// ����Ϊ128kflash��0x8000000~0x801FFFF)��
// 2 �Ѿ�����STM32Ff103C8T6 �� 00x8010000~0x801FFFF�ǿɶ�д�ģ�
// ��ѡ Flash���������Σ�	0x0801 0000 - 0x0801 FFFF	+64 KB������ԣ�

/*
1. 64 KB Flash����׼���ã�
ҳ��Sector��	��ַ��Χ	��С	��;
Page 0	0x0800 0000 - 0x0800 3FFF	16 KB	�洢�������롢������
Page 1	0x0800 4000 - 0x0800 7FFF	16 KB	���������ݴ洢
Page 2	0x0800 8000 - 0x0800 BFFF	16 KB	���������ݴ洢
Page 3	0x0800 C000 - 0x0800 FFFF	16 KB	���������ݴ洢

2. 128 KB Flash���������Σ�
ĳЩ STM32F103C8T6 оƬ ʵ���� 128 KB Flash�����ٷ�����֤ 64 KB��������� 64 KB ��ҳ���£�

ҳ��Sector��	��ַ��Χ	��С	��;
Page 4	0x0801 0000 - 0x0801 3FFF	16 KB	����洢�ռ䣨�ǹٷ���֤��
Page 5	0x0801 4000 - 0x0801 7FFF	16 KB	����洢�ռ䣨�ǹٷ���֤��
Page 6	0x0801 8000 - 0x0801 BFFF	16 KB	����洢�ռ䣨�ǹٷ���֤��
Page 7	0x0801 C000 - 0x0801 FFFF	16 KB	����洢�ռ䣨�ǹٷ���֤��

*/

// ���Ƕ���64k�ĵ�ַ��
#define appConfig_SAVE_ADDR 0x08018000  // 0X080E0000 ����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
// ����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.
appconfig_s appConfig;

void appconfig_init()
{
    appConfig = *((appconfig_s *)malloc(sizeof(appconfig_s)));
    memset(&appConfig, 0x00, sizeof(appconfig_s));
    STMFLASH_Read(appConfig_SAVE_ADDR, (u16 *)(&appConfig), sizeof(appconfig_s));

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
    STMFLASH_Write(appConfig_SAVE_ADDR, (u16*)(&appConfig), sizeof(appconfig_s));
    printf("����ɹ�\n");
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
    appConfig.timeMode = TIMEMODE_24HR;
    appConfig.watchface = 0;

    appConfig.volUI = 1;
    appConfig.volAlarm = 2;
    appConfig.volHour = 1;

    // alarms
    // 22:45:00, 127 = 1111111, ��ʾ����1,2,3,4,5,6,7, 255=1(����) 111111(�ܶ�)1(��һ), ��ʾ���������ҿ���
    // 63 = 111111, ��ʾ����1,2,3,4,5,6   7 = 111, ��ʾ����1,2,3
    appConfig.alarms[0].hour = 9;
    appConfig.alarms[0].min = 0;
    appConfig.alarms[0].days = 225;

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
