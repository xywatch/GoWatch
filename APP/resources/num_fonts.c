#include "common.h"

// 多个字体合并
const byte (*numFont16x16s[WATCHFACE_COUNT])[11][32] = {
    &numFont16x16_weixue,  // 微雪字体
    &numFont16x16_micro5,  // Micro5 字体
    &numFont16x16_tiny5,   // Tiny5 字体
    &numFont16x16_jaro,    // Jaro 字体
    &numFont16x16_revoluzia // Revoluzia 字体
};

const byte (*numFont16x32s[WATCHFACE_COUNT])[11][64] = {
    &numFont16x32_weixue,  // 微雪字体
    &numFont16x32_micro5,  // Micro5 字体
    &numFont16x32_tiny5,   // Tiny5 字体
    &numFont16x32_jaro,    // Jaro 字体
    &numFont16x32_revoluzia // Revoluzia 字体
};