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

    const u8 *space_images[] = {
        space_image1, space_image2, space_image3, space_image4, space_image5,
        space_image6, space_image7, space_image8, space_image9, space_image10};
  
    while (count--)
    {
      for (int i = 0; i < 10; i++)
      {
        draw_bitmap(x, y, space_images[i], width, height, NOINVERT, 0);
        OLED_Flush();
        OLED_ClearScreenBuffer();
        delay_ms(delayMs);
      }
    }
}

void c_setup()
{
    power_pin_init();
    // SystemInit();
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
    delay_init();                                   // ��ʱ������ʼ?
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // ����ϵͳ�ж����ȼ�����2

    uart_init(115200);
    printf("go1\r\n");

    OLED_Init(); // ��ʼ��OLED�ӿ�
    Adc_Init();  // ADC��ʼ��
    KEY_INT_INIT(); // �����ж�
    millis_init();
    delay_ms(50);
    showSpace();

    milliseconds = 0;
    appconfig_init();

    I2C_GPIO_Config(); // I2C��ʼ��
    RTC_Init();
    printf("RTC_Init done\r\n");
    // RTC_Config("2025:03:12:11:59:00");
    // time_update();
    console_log(50, "RTC Init OK");
    
    RTC_INT_INIT();
    alarm_init();
    console_log(50, "alarm Init OK");
    console_log(50, "BMA423 Init...");
    bmaConfig();
    BMA_INT_INIT();
    console_log(50, "BMA423 Init OK");

    led_init();              // ��ʼ��LED
    buzzer_init();
    buttons_init();
    
    pwrmgr_init();
    console_log(50, "START !");

    OLED_ClearScreenBuffer();

    // Set watchface
    display_set(watchface_normal);
    display_load(); // ��������
}

extern bool DeepSleepFlag;
extern bool SleepRequested;
void c_loop()
{
    if (SleepRequested) {
        printf("SleepRequested\r\n");
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

    buzzer_update();

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
