#include "sys.h"
#include "rtc.h"
#include "common.h"

_calendar_obj calendar_rtc; // ʱ�ӽṹ��

static void RTC_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;            // RTCȫ���ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�1λ,�����ȼ�3λ
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // ��ռ���ȼ�0λ,�����ȼ�4λ
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // ʹ�ܸ�ͨ���ж�
    NVIC_Init(&NVIC_InitStructure);                           // ����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

/*

1 ʹ��PWR��BKPʱ�ӣ�RCC_APB1PeriphClockCmd();
��  ʹ�ܺ󱸼Ĵ�������:   PWR_BackupAccessCmd();
��  ����RTCʱ��Դ��ʹ��RTCʱ�ӣ�
      RCC_RTCCLKConfig();
      RCC_RTCCLKCmd();
      ���ʹ��LSE��Ҫ��LSE��RCC_LSEConfig(RCC_LSE_ON);
�� ����RTCԤ��Ƶϵ����RTC_SetPrescaler();
�� ����ʱ�䣺RTC_SetCounter();
�޿�������жϣ������Ҫ��:RTC_ITConfig()��
�߱�д�жϷ�������RTC_IRQHandler();
�ಿ�ֲ���Ҫ�ȴ�д������ɺ�ͬ����
   RTC_WaitForLastTask();// �ȴ����һ�ζ�RTC�Ĵ�����д�������
   RTC_WaitForSynchro();	// �ȴ�RTC�Ĵ���ͬ��

*/

// ʵʱʱ������
// ��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
// BKP->DR1���ڱ����Ƿ��һ�����õ�����
// ����0:����
// ����:�������

uint16_t hasConfigured = 0x5054;

// ����
u8 RTC_Config(void)
{
    BKP_DeInit(); // ��λ��������

    // �ⲿ32.768K��Ӵż�Ǹ�
    RCC_LSEConfig(RCC_LSE_ON);
    // RCC_HSEConfig(RCC_HSE_ON);

    u8 temp = 0;

    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp < 250)
    { // ���ָ����RCC��־λ�������,�ȴ����پ������
        temp++;
        delay_ms(10);
    }

    if (temp >= 250)
    {
        return 10; // ��ʼ��ʱ��ʧ��, ����������
    }

    // RTCʱ��Դ���ó�LSE���ⲿ32.768K��
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    // RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);

    RCC_RTCCLKCmd(ENABLE);            // ʹ��RTCʱ��
    RTC_WaitForLastTask();            // �ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_WaitForSynchro();             // �ȴ�RTC�Ĵ���ͬ��
    RTC_ITConfig(RTC_IT_SEC, ENABLE); // ʹ��RTC���ж�
    RTC_WaitForLastTask();            // �ȴ����һ�ζ�RTC�Ĵ�����д�������

    RTC_EnterConfigMode(); // / ��������

    // ����RTC��Ƶ����ʹRTCʱ��Ϊ1Hz
    // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
    // 62.5K/62500 = 1HZ
    // RTC_SetPrescaler(62500-1); // 32768-1
    RTC_SetPrescaler(32768 - 1); // 32768-1

    RTC_WaitForLastTask();                           // �ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_SetDatetime(2022, 3, 16, 23, 52, 30);        // ����ʱ��, -> RTC_CNT
    RTC_ExitConfigMode();                            // �˳�����ģʽ
    BKP_WriteBackupRegister(BKP_DR1, hasConfigured); // ��ָ���ĺ󱸼Ĵ�����д���û���������

    return 0;
}

// ����
u8 RTC_Config_HSE(void)
{
    BKP_DeInit(); // ��λ��������

    // �ⲿ32.768K��Ӵż�Ǹ�
    // RCC_LSEConfig(RCC_LSE_ON);
    RCC_HSEConfig(RCC_HSE_ON);

    // �ȴ��ȶ�
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
        ;

    // RTCʱ��Դ���ó�LSE���ⲿ32.768K��
    // RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);

    RCC_RTCCLKCmd(ENABLE);            // ʹ��RTCʱ��
    RTC_WaitForLastTask();            // �ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_WaitForSynchro();             // �ȴ�RTC�Ĵ���ͬ��
    RTC_ITConfig(RTC_IT_SEC, ENABLE); // ʹ��RTC���ж�
    RTC_WaitForLastTask();            // �ȴ����һ�ζ�RTC�Ĵ�����д�������

    RTC_EnterConfigMode(); // / ��������

    // ����RTC��Ƶ����ʹRTCʱ��Ϊ1Hz
    // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
    // 62.5K/62500 = 1HZ
    RTC_SetPrescaler(62500 - 1); // 32768-1

    RTC_WaitForLastTask();                           // �ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_SetDatetime(2022, 3, 15, 23, 52, 30);        // ����ʱ��, -> RTC_CNT
    RTC_ExitConfigMode();                            // �˳�����ģʽ
    BKP_WriteBackupRegister(BKP_DR1, hasConfigured); // ��ָ���ĺ󱸼Ĵ�����д���û���������

    return 0;
}

u8 RTC_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); // 1.ʹ��PWR��BKP����ʱ��
    PWR_BackupAccessCmd(ENABLE);                                             //  2. ʹ�ܺ󱸼Ĵ�������

    // ��ָ���ĺ󱸼Ĵ����ж�������: ��������д���ָ�����ݲ����
    // ��42��16λ�󱸼Ĵ��� BKP_DR0-BKP_DR041
    // ��ΪRTC�ļĴ���ֻ��Ҫ����һ��, ������һ��ֵ���ж��Ƿ�������

    if (BKP_ReadBackupRegister(BKP_DR1) != hasConfigured)
    { // û����, ��������
        if (RTC_Config())
        {
            return 10;
        }
    }
    else
    {                                     // ������, ϵͳ������ʱ
        RTC_WaitForSynchro();             // �ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_ITConfig(RTC_IT_SEC, ENABLE); // ʹ��RTC���ж�
        RTC_WaitForLastTask();            // �ȴ����һ�ζ�RTC�Ĵ�����д�������
    }

    RTC_NVIC_Config(); // RCT�жϷ�������
    RTC_GetDatetime(); // ����ʱ��

    //
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    // RTC_Alarm_Set(2022, calendar_rtc.w_month, calendar_rtc.w_date, calendar_rtc.hour, calendar_rtc.min, calendar_rtc.sec + 5);

    return 0; // ok
}

// RTCʱ���ж�
// ÿ�봥��һ��
// extern u16 tcnt;
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) == SET)
    {                                      // �����ж�
        RTC_GetDatetime();                 // ����ʱ��
        RTC_ClearITPendingBit(RTC_IT_SEC); // �������ж�
        RTC_WaitForLastTask();
    }

    // ����
    if (RTC_GetITStatus(RTC_IT_ALR) == SET)
    {                                      // �����ж�
        RTC_ClearITPendingBit(RTC_IT_ALR); // �������ж�
        RTC_GetDatetime();                 // ����ʱ��

        RTC_ClearITPendingBit(RTC_IT_ALR); // �������ж�
        RTC_WaitForLastTask();
    }
}

// �ж��Ƿ������꺯��
// �·�   1  2  3  4  5  6  7  8  9  10 11 12
// ����   31 29 31 30 31 30 31 31 30 31 30 31
// ������ 31 28 31 30 31 30 31 31 30 31 30 31
// ����:���
// ���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{
    if (year % 4 == 0)
    { // �����ܱ�4����
        if (year % 100 == 0)
        {
            if (year % 400 == 0)
            {
                return 1; // �����00��β,��Ҫ�ܱ�400����
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}
// ����ʱ��
// �������ʱ��ת��Ϊ����
// ��1970��1��1��Ϊ��׼
// 1970~2099��Ϊ�Ϸ����
// ����ֵ:0,�ɹ�;����:�������.
// �·����ݱ�
// ƽ����·����ڱ�
const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
u8 RTC_SetDatetime(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
    u16 t;
    u32 seccount = 0;

    if (syear < 1970 || syear > 2099)
    {
        return 1;
    }

    for (t = 1970; t < syear; t++)
    { // ��������ݵ��������
        if (Is_Leap_Year(t))
        {
            seccount += 31622400; // �����������
        }
        else
        {
            seccount += 31536000; // ƽ���������
        }
    }

    smon -= 1;

    for (t = 0; t < smon; t++)
    {                                          // ��ǰ���·ݵ����������
        seccount += (u32)mon_table[t] * 86400; // �·����������

        if (Is_Leap_Year(syear) && t == 1)
        {
            seccount += 86400; // ����2�·�����һ���������
        }
    }

    seccount += (u32)(sday - 1) * 86400; // ��ǰ�����ڵ����������
    seccount += (u32)hour * 3600;        // Сʱ������
    seccount += (u32)min * 60;           // ����������
    seccount += sec;                     // �������Ӽ���ȥ

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); // ʹ��PWR��BKP����ʱ��
    PWR_BackupAccessCmd(ENABLE);                                             // ʹ��RTC�ͺ󱸼Ĵ�������
    RTC_SetCounter(seccount);                                                // ����RTC��������ֵ

    RTC_WaitForLastTask(); // �ȴ����һ�ζ�RTC�Ĵ�����д�������
    return 0;
}

// ��ʼ������
// ��1970��1��1��Ϊ��׼
// 1970~2099��Ϊ�Ϸ����
// syear,smon,sday,hour,min,sec�����ӵ�������ʱ����
// ����ֵ:0,�ɹ�;����:�������.
u8 RTC_Alarm_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
    u16 t;
    u32 seccount = 0;

    if (syear < 1970 || syear > 2099)
    {
        return 1;
    }

    for (t = 1970; t < syear; t++)
    { // ��������ݵ��������
        if (Is_Leap_Year(t))
        {
            seccount += 31622400; // �����������
        }
        else
        {
            seccount += 31536000; // ƽ���������
        }
    }

    smon -= 1;

    for (t = 0; t < smon; t++)
    {                                          // ��ǰ���·ݵ����������
        seccount += (u32)mon_table[t] * 86400; // �·����������

        if (Is_Leap_Year(syear) && t == 1)
        {
            seccount += 86400; // ����2�·�����һ���������
        }
    }

    seccount += (u32)(sday - 1) * 86400; // ��ǰ�����ڵ����������
    seccount += (u32)hour * 3600;        // Сʱ������
    seccount += (u32)min * 60;           // ����������
    seccount += sec;                     // �������Ӽ���ȥ

    // ����ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); // ʹ��PWR��BKP����ʱ��
    PWR_BackupAccessCmd(ENABLE);                                             // ʹ�ܺ󱸼Ĵ�������
    // ���������Ǳ����!

    RTC_SetAlarm(seccount);

    RTC_WaitForLastTask(); // �ȴ����һ�ζ�RTC�Ĵ�����д�������

    return 0;
}

// �õ���ǰ��ʱ��
// ����ֵ:0,�ɹ�;����:�������.
u8 RTC_GetDatetime(void)
{
    static u16 daycnt = 0;
    u32 timecount = 0;
    u32 temp = 0;
    u16 temp1 = 0;
    timecount = RTC_GetCounter();
    temp = timecount / 86400; // �õ�����(��������Ӧ��)

    if (daycnt != temp)
    { // ����һ����
        daycnt = temp;
        temp1 = 1970; // ��1970�꿪ʼ

        while (temp >= 365)
        {
            if (Is_Leap_Year(temp1))
            { // ������
                if (temp >= 366)
                {
                    temp -= 366; // �����������
                }
                else
                {
                    temp1++;
                    break;
                }
            }
            else
            {
                temp -= 365; // ƽ��
            }

            temp1++;
        }

        calendar_rtc.w_year = temp1; // �õ����
        temp1 = 0;

        while (temp >= 28)
        { // ������һ����
            if (Is_Leap_Year(calendar_rtc.w_year) && temp1 == 1)
            { // �����ǲ�������/2�·�
                if (temp >= 29)
                {
                    temp -= 29; // �����������
                }
                else
                {
                    break;
                }
            }
            else
            {
                if (temp >= mon_table[temp1])
                {
                    temp -= mon_table[temp1]; // ƽ��
                }
                else
                {
                    break;
                }
            }

            temp1++;
        }

        calendar_rtc.w_month = temp1 + 1; // �õ��·�
        calendar_rtc.w_date = temp + 1;   // �õ�����
    }

    temp = timecount % 86400;                                                                         // �õ�������
    calendar_rtc.hour = temp / 3600;                                                                  // Сʱ
    calendar_rtc.min = (temp % 3600) / 60;                                                            // ����
    calendar_rtc.sec = (temp % 3600) % 60;                                                            // ����
    calendar_rtc.week = RTC_Get_Week(calendar_rtc.w_year, calendar_rtc.w_month, calendar_rtc.w_date); // ��ȡ����

    // ���õ�timeDate��
    timeDate.date.year = calendar_rtc.w_year - 2000; // 0-255
    timeDate.date.month = calendar_rtc.w_month - 1;  // 1-12 -> 0-11
    timeDate.date.date = calendar_rtc.w_date;        // 1-31

    timeDate.time.hour = calendar_rtc.hour;
    timeDate.time.mins = calendar_rtc.min;
    timeDate.time.secs = calendar_rtc.sec;

    timeDate.date.day = calendar_rtc.week; // 0-6
    return 0;
}

// �����ʽ�Ǵ��!!
// ������������ڼ�
// ��������:���빫�����ڵõ�����(ֻ����1901-2099��)
// �������������������
// ����ֵ�����ں�
/*
u8 const table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; // ���������ݱ�
u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{
    u16 temp2;
    u8 yearH, yearL;

    yearH = year / 100;
    yearL = year % 100;

    //  ���Ϊ21����,�������100
    if (yearH > 19) {
        yearL += 100;
    }

    //  ����������ֻ��1900��֮���
    temp2 = yearL + yearL / 4;
    temp2 = temp2 % 7;
    temp2 = temp2 + day + table_week[month - 1];

    if (yearL % 4 == 0 && month < 3) {
        temp2--;
    }

    return(temp2 % 7);
}
*/

// ��ķ����ɭ�������ڹ�ʽ
// m = �� 1-12 d 1-31, y = 2022
// ���� 0-6 0��һ, 6����
u8 RTC_Get_Week(u16 y, u8 m, u8 d)
{
    if (m == 1 || m == 2)
    {
        m += 12;
        y--;
    }

    return (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
}
