#include "common.h"

void power_pin_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(POWER_ON_PORT == GPIOA ? RCC_APB2Periph_GPIOA : RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = POWER_ON_PIN;        //	 PA1 POWER ���ƶ˿�
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // �ٶ�
    GPIO_Init(POWER_ON_PORT, &GPIO_InitStructure);
    GPIO_SetBits(POWER_ON_PORT, POWER_ON_PIN); // ��Ϊ�ߵ�ƽ, ����
}

void showSpace()
{
    byte width = 52;
    byte height = 48;
    byte x = (128 - width) / 2;
    byte y = (64 - height) / 2;
    u16 delayMs = 100;
    byte count = 2; // ѭ������

    while (count--)
    {
        draw_bitmap(x, y, space_image1, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image2, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image3, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image4, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image5, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image6, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image7, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image8, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image9, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);

        draw_bitmap(x, y, space_image10, width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);
    }
}

extern short pitch_a, roll_a, yaw_a;
void c_setup()
{
    power_pin_init();
    // SystemInit();
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
    delay_init();                                   // ��ʱ������ʼ?
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // ����ϵͳ�ж����ȼ�����2

    uart_init(115200);
    // printf("goxxx1");

    OLED_Init(); // ��ʼ��OLED�ӿ�
    Adc_Init();  // ADC��ʼ��
    KEY_INT_INIT(); // �����ж�
    millis_init();

    delay_ms(50);
    showSpace();

    milliseconds = 0;

    I2C_GPIO_Config(); // I2C��ʼ��
    RTC_Init();
    console_log(50, "RTC Init OK");

    appconfig_init();
    // led_init();              // ��ʼ��LED
    buzzer_init();
    buttons_init();

    // global_init();
    alarm_init(); // �޷��������ӣ�ÿ�������Ժ���Ҫ�Զ���

    pwrmgr_init();
    console_log(50, "START !");

    // memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
    OLED_ClearScreenBuffer();

    // Set watchface
    display_set(watchface_normal);
    display_load(); // ��������
}

extern bool DeepSleepFlag;
extern bool SleepRequested;
void c_loop()
{
    // mpu6050 int����Ȼû��moveCheck�ɹ�, ����ʵ�Ѿ�������, ��ʱDeepSleepFlag=1
    // mpu6050 ���Ѻ�, Ҳ�ᴥ����������, ���Բ�Ҫ��mpu6050���Ѻ�����������
    if (SleepRequested) {
        SleepRequested = false;
        nvic_sleep(1);
    }

    time_update();

    // �����һֱ����true, ����sleepҲ�Ͳ���ִ��loop()��
    if (pwrmgr_userActive())
    {
        // battery_update();
        buttons_update();
    }

    // mpu_updata();

    buzzer_update();
    // led_update();

#if COMPILE_STOPWATCH
    stopwatch_update();
#endif
    //  global_update();

    if (pwrmgr_userActive())
    {
        alarm_update();

        // ˢ����Ļ
        display_update();
    }

    pwrmgr_update();

    // ��ʾ��ɺ����������
    // memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
    OLED_ClearScreenBuffer();
}

void c_setup2()
{
    delay_init();
    uart_init(115200);
    printf("goxxx\n");

    while (1)
    {
        printf("goxxx\n");
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); // ��
        GPIO_SetBits(GPIOB, GPIO_Pin_15);   // ��
        delay_ms(500);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15); // ��
        GPIO_SetBits(GPIOB, GPIO_Pin_14);   // ��
        delay_ms(500);
    }
}

int main(void)
{
    c_setup(); // ��ʼ��

    while (1)
    {
        c_loop(); // ѭ��
    }
}
