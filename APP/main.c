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

extern bool bme_flag;
void bme_update(void)
{
    if (bme_flag)
    {
        bme_flag = 0;
        readTrim();
        bme280CompensateH();
        bme280CompensateP();
        bme280CompensateT();
    }
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

extern short pitch_a, roll_a, yaw_a;
void c_setup()
{
    power_pin_init();
    // SystemInit();
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
    delay_init();                                   // ��ʱ������ʼ?
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // ����ϵͳ�ж����ȼ�����2

    uart_init(115200);
    printf("go1\n");

    OLED_Init(); // ��ʼ��OLED�ӿ�
    Adc_Init();  // ADC��ʼ��
    KEY_INT_INIT(); // �����ж�
    millis_init();
    printf("go2\n");

    delay_ms(50);
    showSpace();
    
    appconfig_init();

    console_log(50, "Init MPU...");
    // console_log(50, "SS %d", SystemCoreClock);

    char i = 0;
    int count = 0;

    i = MPU_Init(); // ��Ҫ����Ϊ����mpu_dmp_init������init

    while (i != 0 && i != 1 && count++ < 3)
    {
        printf("MPU Init %d\n", i);
        console_log(50, "MPU Init %d", i);
        delay_ms(50);
        i = MPU_Init(); // ��Ҫ����Ϊ����mpu_dmp_init������init
    }

    I2C_GPIO_Config();
    i = mpu_dmp_init();
    printf("mpu_dmp_init %d\n", i);
    count = 0;

    while ((++count) < 10 && i)
    {
        // i = MPU_Init(); // ��Ҫ����Ϊ����mpu_dmp_init������init
        // printf("MPU_Init %d\n", i);
        //  console_log(1, "MPU Init:%d", i);
        console_log(50, "MPU DUM Error %d", i);
        delay_ms(50);
        i = mpu_dmp_init();
        printf("mpu_dmp_init error %d\n", i);
    }

    if (!i)
    {
        console_log(50, "Init MPU OK");
    }
    else
    {
        console_log(50, "Init MPU Error");
    }
    if (appConfig.moveCheckFlag) {
        MPU_INT_Init();
    }
    
    milliseconds = 0;

    // Ȼ���ʼ�� DS3231
    DS3231_Init();      // DS3231��ʼ��
    delay_ms(10);       // �ȴ���ʼ�����

    // ��ʼ���ж�����
    RTC_INT_INIT();
    console_log(50, "DS3231 Init OK");

    // time_init();
    // Date_init();
    // ds3231ֻ�õ�ع���, ���Ա���Ҫ���ӵ��
    // ���ds3231������, �ͻ�һֱ����Start! ����5v����ds3231����, ��ʱ��Ҫ����µ�
    DS3231_Get_Time();
    if (timeDate.time.secs == 0 && timeDate.time.mins == 0 && timeDate.time.hour == 0) {
        console_log(50, "Set default date");
        // �ǲ���ÿ�ζ�������rtcʱ��
        time_s time = {0, 39, 10, 'A'};
        date_s date = {DAY_THU, 27, MONTH_FEB, 25};
        timeDate.time = time;
        timeDate.date = date;
        DS3231_Set_Date();
        DS3231_Set_Time();
    }

    led_init();
    buzzer_init();
    buttons_init();

    Bme280_Init();
    console_log(50, "BME Init OK");

    alarm_init();

    pwrmgr_init();
    console_log(50, "START !");

    // memset(&oledBuffer, 0x00, FRAME_BUFFER_SIZE);
    OLED_ClearScreenBuffer();

    // Set watchface
    display_set(watchface_normal);
    display_load(); // ��������
}

bool bme_enable = 1; // ����ʱ��Ҫ

extern bool MPU6050_WakeUpRequested;
extern bool DeepSleepFlag;
extern bool SleepRequested;
void c_loop()
{
    // mpu6050 ����
    if (MPU6050_WakeUpRequested) {
        // 1. �Ȼָ�ʱ��
        RCC_Configuration();
        delay_ms(10);  // �ȴ�ʱ���ȶ�

        printf("MPU6050_WakeUpRequested\n");
        
        // 2. �ټ���˶�
        if (MPU_movecheck()) {
            printf("mpu6050 movecheck\n");
            nvic_wake_up(50);
            SleepRequested = false; // ������mpu6050���Ѻ�����������
        }
        else {
            printf("mpu6050 not move\n");
            // ���mpu6050û���ƶ�, �������ִ������Ľ���STOPģʽ
        }

        MPU6050_WakeUpRequested = false;
    }

    // printf("SleepRequested %d, MPU6050_WakeUpRequested %d, DeepSleepFlag %d\n", SleepRequested, MPU6050_WakeUpRequested, DeepSleepFlag);

    // mpu6050 int����Ȼû��moveCheck�ɹ�, ����ʵ�Ѿ�������, ��ʱDeepSleepFlag=1
    // mpu6050 ���Ѻ�, Ҳ�ᴥ����������, ���Բ�Ҫ��mpu6050���Ѻ�����������
    if (SleepRequested && !MPU6050_WakeUpRequested) {
        SleepRequested = false;
        nvic_sleep(1);
    }

    time_update();

    // �����һֱ����true, ����sleepҲ�Ͳ���ִ��loop()��
    if (pwrmgr_userActive())
    {
        buttons_update();
    }

    buzzer_update();

#if COMPILE_STOPWATCH
    stopwatch_update();
#endif

    if (pwrmgr_userActive())
    {
        alarm_update();

        // ˢ����Ļ
        display_update();
    }

    if (bme_enable)
    {
        bme_update();
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
