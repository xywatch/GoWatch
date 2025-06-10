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

// ����ѡ��
#define COMPILE_GAME1 1      // ��ϷBreakout
#define COMPILE_GAME2 1      // ��ϷCar dodge
#define COMPILE_GAME3 1      // ��ϷFlappy thing (not finished) ��bug  δ��ɣ�ʵ��������
#define COMPILE_GAME_SNAKE 1 // game snake
#define COMPILE_GAME_LIFE 1  // game life

#define COMPILE_ANIMATIONS 1 // ����
#define COMPILE_STOPWATCH 1  // ���
#define COMPILE_TORCH 1      // �ֵ�Ͳ
#define COMPILE_TUNEMAKER 1  // 3D����
#define COMPILE_CALENDAR 1   // ����

#define POWER_ON_PIN GPIO_Pin_1 // PA1
#define POWER_ON_PORT GPIOA

#endif /* CONFIG_H_ */
