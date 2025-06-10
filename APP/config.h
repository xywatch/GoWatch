#ifndef CONFIG_H_
#define CONFIG_H_

// Hardware version
#define HW_VERSION 2
#define USER_NAME "Life"

// Firmware version
#define FW_VERSION "2"

// Language
// 0 = English
// 1 = German (not done)
// 2 = Russian
#define LANGUAGE 0

// 编译选项
#define COMPILE_GAME1 1      // 游戏Breakout
#define COMPILE_GAME2 1      // 游戏Car dodge
#define COMPILE_GAME3 1      // 游戏Flappy thing (not finished) 有bug  未完成，实在做不来
#define COMPILE_GAME_SNAKE 1 // game snake
#define COMPILE_GAME_LIFE 1  // game life

#define COMPILE_ANIMATIONS 1 // 动画
#define COMPILE_STOPWATCH 1  // 秒表
#define COMPILE_TORCH 1      // 手电筒
#define COMPILE_TUNEMAKER 1  // 3D滚动
#define COMPILE_CALENDAR 1   // 日历

#define POWER_ON_PIN GPIO_Pin_1 // PA1
#define POWER_ON_PORT GPIOA

#endif /* CONFIG_H_ */
