/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

float BatteryVol;

// 电池电量百分比计算
static int calcBatteryPercentage(float voltage) {
    // 锂电池电压范围
    const float VBAT_MAX = 4.2f;  // 满电电压
    const float VBAT_MIN = 3.2f;  // 最低电压
    const float VBAT_FULL = 4.15f; // 认为完全充满的电压
    
    // 电压限幅
    if(voltage > VBAT_MAX) voltage = VBAT_MAX;
    if(voltage < VBAT_MIN) voltage = VBAT_MIN;
    
    // 非线性映射计算电量百分比
    float percentage;
    if(voltage >= VBAT_FULL) {
        percentage = 100.0f;
    }
    else if(voltage >= 3.9f) { // 4.15V-3.9V 映射到 100%-80%
        percentage = 80.0f + (voltage - 3.9f) * 20.0f / (VBAT_FULL - 3.9f);
    }
    else if(voltage >= 3.7f) { // 3.9V-3.7V 映射到 80%-55%
        percentage = 55.0f + (voltage - 3.7f) * 25.0f / 0.2f;
    }
    else if(voltage >= 3.5f) { // 3.7V-3.5V 映射到 55%-30%
        percentage = 30.0f + (voltage - 3.5f) * 25.0f / 0.2f;
    }
    else { // 3.5V-3.2V 映射到 30%-0%
        percentage = (voltage - VBAT_MIN) * 30.0f / 0.3f;
    }
    
    return (int)(percentage + 0.5f); // 四舍五入
}

void drawBattery()
{
    int bat;
    char ad[5];
    const byte *battIcon;

    // 根据电量百分比选择电池图标
    bat = calcBatteryPercentage(BatteryVol);
    
    if (bat < 10) {
        battIcon = battIconEmpty;
    }
    else if (bat < 30) {
        battIcon = battIconLow;
    }
    else if (bat < 60) {
        battIcon = battIconHigh;
    }
    else if (bat < 90) {
        battIcon = battIconHigh;  // 可以添加更多图标
    }
    else {
        battIcon = battIconFull;
    }

    // 绘制电池图标
    draw_bitmap(0, FRAME_HEIGHT - 8, battIcon, 16, 8, NOINVERT, 0);

    if (bat >= 100) {
        bat = 99;
    }

    // 显示电量百分比
    sprintf((char *)ad, "%d", bat);
    draw_string(ad, NOINVERT, 18, FRAME_HEIGHT - 8);
}
