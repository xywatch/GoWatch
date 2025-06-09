#include "common.h"

#define BTN_IS_PRESSED 4
#define BTN_NOT_PRESSED 4

typedef struct
{
    millis_t pressedTime; // Time of press
    bool processed;       // Time of press has been stored (don't store again until next press)
    byte counter;         // Debounce counter
    bool funcDone;        // Function has been ran (don't run again until next press)
    button_f onPress;     // Function to run when pressed
    const ulong *tune;    // Tune to play when pressed
} s_button;

static s_button buttons[BTN_COUNT];
static millis_t lastPressedTime;
byte lastCounter;

static void processButtons(void);
static void processButton(s_button *, BOOL);
static byte bitCount(byte);

extern bool DeepSleepFlag;
// EMPTY_INTERRUPT(PCINT0_vect);

void buttons_init()
{
    buttons_startup();

    // 蜂鸣器的播放曲调
    //  Assign tunes
    buttons[BTN_1].tune = tuneBtn1;
    buttons[BTN_2].tune = tuneBtn2;
    buttons[BTN_3].tune = tuneBtn3;

    // Set up interrupts
    // #ifdef __AVR_ATmega32U4__
    //  SET_BITS(PCMSK0, PCINT4, PCINT6, PCINT7);
    // #else
    //  SET_BITS(PCMSK1, PCINT9, PCINT10, PCINT11);
    // #endif
    //  BTN_INT_ON();
}

void buttons_update()
{
    static millis8_t lastUpdate;

    // Update every 10ms
    millis8_t now = millis();

    if ((millis8_t)(now - lastUpdate) >= 10)
    {
        // printf("1");
        lastUpdate = now;
        processButtons();
    }
}

// 按键初始化函数
//  Sets button pins to INPUT with PULLUP
void buttons_startup()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Enable GPIO clocks
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

    // Configure PB1 (Button 1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Configure PA4 and PA5 (Buttons 2 and 3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// Sets button pins to OUTPUT LOW
// This stops them wasting current through the pull-up resistor when pressed
void buttons_shutdown()
{
    // 配合休眠sleep函数开启后关闭按键，不会做

    //  pinPullup(BTN_1_P,	PU_DIS);
    //  pinPullup(BTN_2_P,	PU_DIS);
    //  pinPullup(BTN_3_P,	PU_DIS);

    //  pinMode(BTN_1_P,	OUTPUT);
    //  pinMode(BTN_2_P,	OUTPUT);
    //  pinMode(BTN_3_P,	OUTPUT);
}

// 按下后接地
static void processButtons()
{
    // Get button pressed states
    BOOL isPressed[BTN_COUNT];
    isPressed[BTN_1] = !UP_BTN_KEY; // right一个按下即可
    isPressed[BTN_2] = !CONFIRM_BTN_KEY;
    isPressed[BTN_3] = !DOWN_BTN_KEY; // left
    
    // printf("UP_BTN_KEY: %d, CONFIRM_BTN_KEY: %d, DOWN_BTN_KEY: %d\n", isPressed[BTN_1], isPressed[BTN_2], isPressed[BTN_3]);
    // Process each button
    LOOPR(BTN_COUNT, i)
    processButton(&buttons[i], isPressed[i]);
}

static void processButton(s_button *button, BOOL isPressed)
{
    button->counter <<= 1;

    if (isPressed)
    {
        // Set debounce counter bit
        button->counter |= 1;

        // Are enough bits set to count as pressed?//相当于按键消抖
        if (bitCount(button->counter) >= BTN_IS_PRESSED)
        {
            // Store time of press
            if (!button->processed)
            {
                button->pressedTime = millis();
                button->processed = true;

                // 如果连续按3下, 1秒内, 则重启屏幕
                // millis8_t now = button->pressedTime;
                // lastCounter++;
                // // 如果和上次按下<1000, 则认为是1秒内
                // if (now - lastPressedTime <= 1000 && lastCounter >= 4) {
                //     // GPIO_ResetBits(POWER_ON_PORT, POWER_ON_PIN);
                //     // OLED_Init(); // 重新oled初始化不行啊
                //     // OLED_InitIt();
                //     lastPressedTime = 0;
                //     lastCounter = 0;
                // }
                // // 重新计数
                // else if (now - lastPressedTime > 1000) {
                //     lastPressedTime = now;
                //     lastCounter = 1;
                // }
            }

            // Run function
            if (!button->funcDone && button->onPress != NULL && button->onPress())
            {
                button->funcDone = true;
                // 按键声音, 如果是按下休眠, 再响会导致声音很怪
                if (!DeepSleepFlag) {
                    tune_play(button->tune, VOL_UI, PRIO_UI);
                    buzzer_beep(100);
                }
            }
        }
    }
    else
    { // Not pressed
        // Has button been not pressed for long enough?
        if (bitCount(button->counter) <= BTN_NOT_PRESSED)
        {
            button->processed = false;
            button->funcDone = false;
        }
    }
}

// Count set bits in value
static byte bitCount(byte val)
{
    byte count = 0;

    for (; val; val >>= 1)
    {
        count += val & 1;
    }

    return count;
}

// 定义一个返回函数指针的函数
//  Set new function to run when button is pressed and return the old function
button_f buttons_setFunc(btn_t btn, button_f func)
{
    button_f old = buttons[btn].onPress;
    buttons[btn].onPress = func;
    return old;
}

// Set functions to run for each button
void buttons_setFuncs(button_f btn1, button_f btn2, button_f btn3)
{
    buttons[BTN_1].onPress = btn1;
    buttons[BTN_2].onPress = btn2;
    buttons[BTN_3].onPress = btn3;
}

/*
  // Get how long a button has been pressed for
  millis_t buttons_pressTime(btn_t btn) // set max press time to 1 min!!!
  {
    s_button* button = &buttons[btn];
    if(button->pressed == BTN_NOT_PRESSED)
        return 0;
    return (millis() - button->pressedTime);
  }
*/

// See if a button has been pressed in the past x milliseconds
bool buttons_isActive()
{
    //  // If sleep has been disabled then just say that the buttons are always active
    if (!appConfig.sleepTimeout)
    {
        return true;
    }

    //  // Get timeout val in ms
    uint timeout = (appConfig.sleepTimeout * 5) * 1000;
    //  uint timeout =  1000;

    // See if a button has been pressed within that timeout
    LOOPR(BTN_COUNT, i)
    {
        if (millis() - buttons[i].pressedTime < timeout)
        {
            return true;
        }
    }

    return false;
}

// Set button status to pressed, processed etc but don't run their functions
void buttons_wake()
{
    LOOPR(BTN_COUNT, i)
    {
        buttons[i].funcDone = true;
        buttons[i].processed = true;
        buttons[i].counter = BTN_IS_PRESSED;
        buttons[i].pressedTime = millis();
    }
}
