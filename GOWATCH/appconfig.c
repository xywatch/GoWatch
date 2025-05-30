#include "common.h"
// #include "stmflash.h"
#include "stm32f10x_flash.h"

#define EEPROM_CHECK_NUM 0x68 // Any 8 bit number that isn't 0 or 255

// 主存储器的起始地址就是 0X08000000, stm32f103c8t6 flash大小64K，地址范围 0x08000000 - 0x0800FFFF 
// 1 STM32F103C8T6和STM32F103CBT6 引脚相同，唯一的区别是前者为64kflash（0x8000000~0x800FFFF） 
// 后者为128kflash（0x8000000~0x801FFFF)；
// 2 已经发现STM32Ff103C8T6 在 00x8010000~0x801FFFF是可读写的；
// 可选 Flash（部分批次）	0x0801 0000 - 0x0801 FFFF	+64 KB（需测试）

/*
1. 64 KB Flash（标准配置）
页（Sector）	地址范围	大小	用途
Page 0	0x0800 0000 - 0x0800 3FFF	16 KB	存储启动代码、主程序
Page 1	0x0800 4000 - 0x0800 7FFF	16 KB	主程序、数据存储
Page 2	0x0800 8000 - 0x0800 BFFF	16 KB	主程序、数据存储
Page 3	0x0800 C000 - 0x0800 FFFF	16 KB	主程序、数据存储

2. 128 KB Flash（部分批次）
某些 STM32F103C8T6 芯片 实际有 128 KB Flash（但官方仅保证 64 KB），额外的 64 KB 分页如下：

页（Sector）	地址范围	大小	用途
Page 4	0x0801 0000 - 0x0801 3FFF	16 KB	额外存储空间（非官方保证）
Page 5	0x0801 4000 - 0x0801 7FFF	16 KB	额外存储空间（非官方保证）
Page 6	0x0801 8000 - 0x0801 BFFF	16 KB	额外存储空间（非官方保证）
Page 7	0x0801 C000 - 0x0801 FFFF	16 KB	额外存储空间（非官方保证）

*/

// 这是额外64k的地址了
#define eepCheck_SAVE_ADDR 0x08018000  // 0X080E0000 设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
// 否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.
#define appConfig_SAVE_ADDR eepCheck_SAVE_ADDR + 16 // 设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
// 否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.

appconfig_s appConfig; // appconfig_s的长度为8
static byte eepCheck;//= EEPROM_CHECK_NUM;

void appconfig_initOld()
{
    STMFLASH_Read(eepCheck_SAVE_ADDR, (u32 *)(&eepCheck), sizeof(byte));
    appConfig = *((appconfig_s *)malloc(sizeof(appconfig_s)));
    memset(&appConfig, 0x00, sizeof(appconfig_s));

    // 如果之前设置过appconfig, 则读取appconfig
    if (eepCheck == EEPROM_CHECK_NUM)
    {
        STMFLASH_Read(appConfig_SAVE_ADDR, (u32 *)(&appConfig), sizeof(appconfig_s));
        printf("之前有 appconfig_init: appConfig: %d\n", appConfig.sleepTimeout);
    }
    else
    {
        printf("之前没有 appconfig_init: appConfig: %d, eepCheck=%d\n", appConfig.sleepTimeout, eepCheck);
        eepCheck = EEPROM_CHECK_NUM;
        STMFLASH_Write(eepCheck_SAVE_ADDR, (u32 *)(&eepCheck), sizeof(byte));

        appconfig_reset();
    }

    if (appConfig.sleepTimeout > 12)
    {
        appConfig.sleepTimeout = 0;
    }
}

void appconfig_init()
{
    appConfig = *((appconfig_s *)malloc(sizeof(appconfig_s)));
    memset(&appConfig, 0x00, sizeof(appconfig_s));
    STMFLASH_Read(appConfig_SAVE_ADDR, (u32 *)(&appConfig), sizeof(appconfig_s));

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
    STMFLASH_Write(appConfig_SAVE_ADDR, (u32*)(&appConfig), sizeof(appconfig_s));
    printf("保存成功\n");
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

    appConfig.volUI = 1;
    appConfig.volAlarm = 2;
    appConfig.volHour = 1;

    appconfig_save();
}
