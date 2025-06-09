#include "beep.h"
#include "common.h"

static uint buzzLen;
static millis8_t startTime;
static buzzFinish_f onFinish;
static tonePrio_t prio;

static void stop(void);

// time2 -> 部分重映射到了 PA15
// time2 == PA15, PA15连接到了BEEP

// TIM2 PWM initialization for buzzer
void TIM_PWM_Init_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // Enable clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // Configure PA15 for TIM2_CH1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置PA15为TIM2_CH1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_TIM2);

    // Configure TIM2
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // Configure PWM
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = arr / 4;  // 25%占空比，减少发热
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    // 使能TIM2的OC1输出
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    TIM_Cmd(TIM2, DISABLE);
}

// 蜂鸣器初始化
void buzzer_init()
{
    // 对于32MHz系统时钟：
    // 分频到2KHz (32MHz/16000)
    // 2KHz/4 = 500Hz PWM频率
    // 这个频率应该接近蜂鸣器的谐振频率
    // TIM_PWM_Init_Init(4, 16000);
    buzzer_init2();
}

// 设置不同的音色
void buzzer_set_timbre(u8 timbre)
{
    switch(timbre) {
        case 0:  // 纯音
            TIM_PWM_Init_Init(10000, 256);  // 12.5Hz
            break;
        case 1:  // 柔和音
            TIM_PWM_Init_Init(8000, 256);   // 15.6Hz
            break;
        case 2:  // 明亮音
            TIM_PWM_Init_Init(12000, 256);  // 10.4Hz
            break;
        case 3:  // 低沉音
            TIM_PWM_Init_Init(6000, 256);   // 20.8Hz
            break;
        default:
            TIM_PWM_Init_Init(10000, 256);
            break;
    }
}

// Non-blocking buzz
void buzzer_buzz(uint16_t len, uint16_t tone, vol_t volType, tonePrio_t _prio, buzzFinish_f _onFinish)
{
    if (_prio < prio)
    {
        return;
    }
    else if (tone == TONE_STOP)
    {
        stop();
        return;
    }

    // Tell power manager that we are busy buzzing
    //	pwrmgr_setState(PWR_ACTIVE_BUZZER, PWR_STATE_IDLE);

    prio = _prio;
    onFinish = _onFinish;
    buzzLen = len;
    startTime = millis();

    // Silent pause tone   //无声暂停音
    if (tone == TONE_PAUSE)
    {
        //		CLEAR_BITS(TCCR1A, COM1A1, COM1A0);
        //		power_timer1_disable();
        TIM_SetCompare1(TIM2, 5000);

        TIM_Cmd(TIM2, DISABLE); // 停止TIM3

        GPIO_ResetBits(GPIOA, GPIO_Pin_15); // 蜂鸣器对应引脚GPIOF8拉低
        return;
    }

    // Workout volume
    uint ocr;
    byte vol;

    switch (volType)
    {
    case VOL_UI:
        vol = appConfig.volUI;
        break;

    case VOL_ALARM:
        vol = appConfig.volAlarm;
        break;

    case VOL_HOUR:
        vol = appConfig.volHour;
        break;

    default:
        vol = 2;
        break;
    }

    // Pulse width goes down as freq goes up
    // This keeps power consumption the same for all frequencies, but volume goes down as freq goes up

    // vol--;
    //	if(vol > 2)
    //		return;

    //  uint icr = tone * (8 << 1);

    //	ocr = icr - (icr / (32>>vol));

    ocr = 50000 / tone;

    // BEEP=1; //PB.5 输出低

    switch (vol)
    {
    case 0:
        TIM_SetCompare1(TIM2, 5000);
        break;

    case 1:
        TIM_SetCompare1(TIM2, ocr / 8); // 1/4音量
        break;

    case 2:
        TIM_SetCompare1(TIM2, ocr / 4); // 1/2音量
        break;

    case 3:
        TIM_SetCompare1(TIM2, ocr / 2); // 1/1音量
        break;
    }

    // TIM_SetCompare1(TIM1,(ocr>>(2-vol))/2);

    TIM_SetAutoreload(TIM2, ocr); // 改变频率
    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
}
/*
#include "led.h"

void buzzer_buzzb(byte len, tone_t tone, vol_t volType)
{
    (void)(volType);

    led_flash(LED_GREEN, 50, 255);
    led_flash(LED_RED, 50, 255);

    power_timer1_enable();
    TCCR1A |= _BV(COM1A1)|_BV(COM1A0);

//	static uint vol = 0;
//	vol++;
//	if(vol > 790)
//		vol = 1;
//	OCR1A = vol;

//	if(vol > 3)
//		vol = 0;
//	if(vol == 0)
//		OCR1A = tone; // normal
//	else if(vol == 1)
//		OCR1A = (tone * 2) - 50; // quiet
//	else if(vol == 2)
//		OCR1A = (tone / 2); // loud
//	else if(vol == 3)
//		OCR1A = (tone / 4); // loader (acually quiter)

    OCR1A = (tone * 2) - 100;
    ICR1 = tone * 2;
    while(len--)
    {
        delay(1);
        led_update();
    }
//	delay(20);
    TCCR1A &= ~(_BV(COM1A1)|_BV(COM1A0));
    power_timer1_disable();
}
*/
// Are we buzzing?
bool buzzer_buzzing()
{
    return buzzLen;
}
#include "led.h"
// See if its time to stop buzzing
void buzzer_update()
{

    if (buzzLen && (millis() - startTime) >= buzzLen)
    {
        stop();
        // led_flash(LED_GREEN, 50, 255);

        if (onFinish != NULL)
        {
            onFinish(); // 播放下一个音节
        }
    }
}

static void stop()
{
    //	CLEAR_BITS(TCCR1A, COM1A1, COM1A0);
    //	power_timer1_disable();`
    TIM_SetCompare1(TIM2, 5000);
    TIM_Cmd(TIM2, DISABLE); // 停止TIM3
    // TIM_CtrlPWMOutputs(TIM2, DISABLE);
    GPIO_ResetBits(GPIOA, GPIO_Pin_15); // 蜂鸣器对应引脚GPIOF8拉低
    // BEEP=0; //PB.5 输出低
    buzzLen = 0;
    prio = PRIO_MIN;
    //	pwrmgr_setState(PWR_ACTIVE_BUZZER, PWR_STATE_NONE);
}

// 简单方案（用于对比）
void buzzer_init2()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 使能GPIOA时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    
    // 配置PA15为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // 初始状态设为低电平
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

// 蜂鸣器开关控制
void buzzer_on()
{
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
}

void buzzer_off()
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

// 蜂鸣器状态切换
void buzzer_toggle()
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_15, (BitAction)!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15));
}

// 蜂鸣器鸣叫指定时间
void buzzer_beep(u16 time)
{
    buzzer_on();
    delay_ms(time);
    buzzer_off();
}

// 蜂鸣器鸣叫指定次数
void buzzer_beep_times(u8 times, u16 on_time, u16 off_time)
{
    while(times--)
    {
        buzzer_on();
        delay_ms(on_time);
        buzzer_off();
        delay_ms(off_time);
    }
}

/*
优化 PWM 参数：
频率保持在 500Hz（蜂鸣器的谐振频率）
占空比降到 25%（减少发热）
使用更合理的预分频值
硬件建议：
在蜂鸣器两端并联一个 0.1μF 电容
在蜂鸣器串联一个 100Ω 电阻
这些可以：
限制电流
改善波形
减少发热
保留了两个方案：
PWM 方案（buzzer_init）：音质好，但需要硬件改进
简单方案（buzzer_init2）：发热小，但音质一般
*/