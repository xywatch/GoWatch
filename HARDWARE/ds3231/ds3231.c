#include "common.h"

Calendar_OBJ calendar;
#define DS3231_WriteAddress 0xD0
#define DS3231_ReadAddress 0xD1

u8 test1, test2, test3;
u8 alarm1_flag;
u8 alarm_hour, alarm_min, alarm_sec, alarm_week;
extern bool DeepSleepFlag;
u8 BCD2HEX(u8 val)
{
    u8 i;
    i = val & 0x0f;
    val >>= 4;
    val &= 0x0f;
    val *= 10;
    i += val;

    return i;
}

// DS3231 оƬ�ļĴ���ʹ�� BCD ��洢ʱ����Ϣ��������д��ʱ��֮ǰ��Ҫ����ͨʮ������ת��Ϊ BCD �롣���磺
// д��Сʱ 23 ʱ����Ҫд�� 0x23 ������ 0x17��23��ʮ�����ƣ�
// д����� 45 ʱ����Ҫд�� 0x45 ������ 0x2D��45��ʮ�����ƣ�
u16 B_BCD(u8 val)
{
    u8 i, j, k;
    i = val / 10;
    j = val % 10;
    k = j + (i << 4);
    return k;
}

u8 DS3231_week_to_system_week(u8 ds3231_week)
{
    return (ds3231_week == 1) ? 6 : (ds3231_week - 2);
}

// ԭ����ֵ��0=��һ��1=�ܶ���2=������3=���ģ�4=���壬5=������6=����
// Ŀ��ֵ��  1=���գ�2=��һ��3=�ܶ���4=������5=���ģ�6=���壬7=����
// ת����ʽ��
// ���������(6)��ת��1
// �������������(0-5)����2�͵õ���Ӧ��ֵ(2-7)
u8 system_week_to_DS3231_week(u8 system_week)
{
    return (system_week == 6) ? 1 : (system_week + 2);
}

void DS3231_WR_Byte(u8 addr, u8 bytedata)
{
    I2C_Start();
    I2C_SendByte(DS3231_WriteAddress);
    I2C_WaitAck();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_SendByte(bytedata);
    I2C_WaitAck();
    I2C_Stop();
}
u8 DS3231_RD_Byte(u8 addr)
{
    u8 Dat = 0;

    I2C_Start();
    I2C_SendByte(DS3231_WriteAddress);
    I2C_WaitAck();
    I2C_SendByte(addr);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(DS3231_ReadAddress);
    I2C_WaitAck();
    Dat = I2C_ReceiveByte();
    I2C_Stop();

    return Dat;
}

void DS3231_Init(void)
{
    I2C_GPIO_Config();
    
    // ������ƼĴ���
    DS3231_WR_Byte(Address_control, 0x00);
    delay_ms(2);
    
    // ���״̬�Ĵ����е����б�־λ
    DS3231_WR_Byte(Address_control_status, Clear_A1IE_Flag);
    delay_ms(2);
    
    // ���ÿ��ƼĴ�����
    // - ���þ��� (OSC_Enable = 0)
    // - ��������1�ж� (A1IE_Enable = 1)
    // - ʹ���ж����ģʽ (OUTPUT_INTSQW = 1)
    // 0x05 = 0000 0101
    // bit 7: OSC_Enable = 0    (���þ���)
    // bit 6: BBSQW = 0        (δʹ��)
    // bit 5: CONV = 0         (δʹ��)
    // bit 4-3: RS2-1 = 00     (δʹ��)
    // bit 2: INTCN = 1        (ʹ���ж����ģʽ)
    // bit 1: A2IE = 0         (��������2�ж�)
    // bit 0: A1IE = 1         (��������1�ж�)
    uint8_t ctrl = OSC_Enable | OUTPUT_INTSQW | A1IE_Enable;
    DS3231_WR_Byte(Address_control, ctrl);
    delay_ms(2);

    // ��ȡ����ӡ����
    // printf("DS3231 Init Config:\n");
    // printf("Control(0x0E): 0x%02X (should be 0x05)\n", DS3231_RD_Byte(Address_control));
    // printf("Status(0x0F): 0x%02X (should be 0x00)\n", DS3231_RD_Byte(Address_control_status));
    // DS3231_Alarm_Config();
}

// void DS3231_Alarm_Config(void)
// {
//     // Enable alarm 1 interrupt and keep INTCN=1 to use INT pin
//     DS3231_WR_Byte(Address_control, OSC_Enable | A1IE_Enable | OUTPUT_INTSQW);
//     // Clear alarm 1 interrupt flag
//     // �������1���жϱ�־λ
//     // �������������־λ��INT���Ż�һֱ���ֵ͵�ƽ
//     // �´����Ӿ��޷����������ж���
//     DS3231_WR_Byte(Address_control_status, Clear_A1IE_Flag);
// }

void DS3231_Set_Time(void)
{
    u8 temp = 0;
    temp = B_BCD(timeDate.time.hour);
    DS3231_WR_Byte(0x02, temp);

    temp = B_BCD(timeDate.time.mins);
    DS3231_WR_Byte(0x01, temp);

    temp = B_BCD(timeDate.time.secs);
    DS3231_WR_Byte(0x00, temp);
}

void DS3231_Set_Date(void)
{
    u8 temp = 0;

    temp = B_BCD(timeDate.date.year);
    DS3231_WR_Byte(0x06, temp);

    temp = B_BCD(timeDate.date.month);
    DS3231_WR_Byte(0x05, temp);

    temp = B_BCD(timeDate.date.date);
    DS3231_WR_Byte(0x04, temp);

    // ת�����ڸ�ʽ��0-6����һ�����գ�ת��Ϊ1-7�����յ�������
    u8 ds3231_day = system_week_to_DS3231_week(timeDate.date.day);
    temp = B_BCD(ds3231_day);
    DS3231_WR_Byte(0x03, temp);  // д��ת���������ֵ

    // ��ӡ������Ϣ
    // printf("Setting current date:\n");
    // printf("System week: %d (0=Mon, 6=Sun)\n", timeDate.date.day);
    // printf("DS3231 week: %d (1=Sun, 7=Sat)\n", ds3231_day);
    // printf("Week register after set: 0x%02X\n", DS3231_RD_Byte(0x03));
}

void Date_init(void)
{
    timeDate.date.date = 11;
    timeDate.date.day = 5;
    timeDate.date.month = 2;
    timeDate.date.year = 22;
}

float DS3231_Temp;


void DS3231_Get_Time(void)
{
    calendar.w_year = DS3231_RD_Byte(0x06);
    timeDate.date.year = BCD2HEX(calendar.w_year);
    calendar.w_month = DS3231_RD_Byte(0x05);
    timeDate.date.month = BCD2HEX(calendar.w_month);
    calendar.w_date = DS3231_RD_Byte(0x04);
    timeDate.date.date = BCD2HEX(calendar.w_date);

    calendar.hour = DS3231_RD_Byte(0x02);
    //  calendar.hour&=0x3f;
    timeDate.time.hour = BCD2HEX(calendar.hour);
    calendar.min = DS3231_RD_Byte(0x01);
    timeDate.time.mins = BCD2HEX(calendar.min);
    calendar.sec = DS3231_RD_Byte(0x00);
    timeDate.time.secs = BCD2HEX(calendar.sec);

    calendar.week = DS3231_week_to_system_week(DS3231_RD_Byte(0x03));
    timeDate.date.day = BCD2HEX(calendar.week);

    /*************��ȡ3231оƬ�¶�***************************************/

    calendar.temper_H = DS3231_RD_Byte(0x11);
    calendar.temper_L = (DS3231_RD_Byte(0x12) >> 6) * 25;
    DS3231_Temp = (float)calendar.temper_H + (float)calendar.temper_L / 100;
}

void DS3231_Set_alarm1(byte hour, byte min, day_t day)
{
		u8 temp_a = 0;
    printf("Setting alarm for %02d:%02d\n", hour, min);

    // 1. ����Ϊ0��A1M1=0����Ҫƥ�䣩
    DS3231_WR_Byte(Alarm1_Address_Second, 0x00);  // ����Ϊ0��A1M1=0����Ҫƥ�䣩
    // DS3231_WR_Byte(Alarm1_Address_Second, 0x80);  // ����Ҫƥ�䣬A1M1=1������ƥ�䣩���������, ����Ҫ����Ϊ0, ��Ȼ�������Ӳ�����
    
    // 2. ���÷��ӣ�A1M2=0����Ҫƥ�䣩
    temp_a = B_BCD(min);
    DS3231_WR_Byte(Alarm1_Address_Minute, temp_a);  // ����Ҫ&0x7F����ΪBCDֵ�����0x59
    
    // 3. ����Сʱ��A1M3=0����Ҫƥ�䣩��24Сʱ��
    temp_a = B_BCD(hour);
    DS3231_WR_Byte(Alarm1_Address_Hour, temp_a);  // ����Ҫ&0x3F����ΪBCDֵ�����0x23
    
    // 4. ��������/���ڣ�A1M4=1������ƥ�䣩
    /*
    DS3231_WR_Byte(Alarm1_Address_Week, 0x80);
    // ��ȡ����ӡ������ؼĴ�����ֵ���е���
    printf("\nDS3231 Registers after alarm set:\n");
    printf("Control(0x0E): 0x%02X (should be 0x05)\n", DS3231_RD_Byte(Address_control));
    printf("Status(0x0F): 0x%02X (should be 0x00)\n", DS3231_RD_Byte(Address_control_status));
    printf("Alarm1 Second: 0x%02X (should be 0x00)\n", DS3231_RD_Byte(Alarm1_Address_Second));
    printf("Alarm1 Minute: 0x%02X (should match %02d)\n", DS3231_RD_Byte(Alarm1_Address_Minute), min);
    printf("Alarm1 Hour: 0x%02X (should match %02d)\n", DS3231_RD_Byte(Alarm1_Address_Hour), hour);
    printf("Alarm1 Week/Date: 0x%02X\n", DS3231_RD_Byte(Alarm1_Address_Week));
    */

    // ����1������/���ڼĴ�����0x0A���ĸ�ʽ���£�
    // bit 7: A1M4 (ƥ��λ)
    // bit 6: DY/DT (1=������ƥ�䣬0=������ƥ��)
    // bit 5-0: ����(1-7)������(1-31)��BCDֵ
    // �������ڣ�A1M4=0����Ҫƥ�䣩��DY/DT=1��������ƥ�䣩
    
    byte nextAlarmDay = (byte)day;  // ��һ�����ӵ�����, 0-6, 0:��һ, 6:����
    uint8_t ds3231_day = system_week_to_DS3231_week(nextAlarmDay); // (nextAlarmDay == 6) ? 1 : (nextAlarmDay + 2);
    temp_a = B_BCD(ds3231_day);  // ת��ΪDS3231�����ڸ�ʽ, ����Ϊ DS3231 �����ڴ� 1 ��ʼ��1=���գ�7=������
    DS3231_WR_Byte(Alarm1_Address_Week, (0x40 | temp_a));  // 0x40=������ƥ�䣬A1M4=0 0b 0100 0000

    // ��ӡ������Ϣ
    printf("\nAlarm Configuration Verification:\n");
    printf("Day conversion: %d -> %d (DS3231 format)\n", nextAlarmDay, ds3231_day);
    printf("Time: %02d:%02d:00\n", hour, min);
    printf("Week register: 0x%02X\n", DS3231_RD_Byte(Alarm1_Address_Week));
    printf("Control: 0x%02X, Status: 0x%02X\n", 
           DS3231_RD_Byte(Address_control),
           DS3231_RD_Byte(Address_control_status));
    // ��ӡ��ǰweek
    printf("Current week: %d\n", nextAlarmDay); // 0-6, 0:��һ, 6:����
}

// û����, ��Ϊ���������������
void DS3231_Clear_alarm1(void)
{
    // �������
    DS3231_WR_Byte(Alarm1_Address_Second, 0x00);  // ����Ϊ0��A1M1=0
    DS3231_WR_Byte(Alarm1_Address_Minute, 0x00);  // ������Ϊ0��A1M2=0
    DS3231_WR_Byte(Alarm1_Address_Hour, 0x00);    // Сʱ��Ϊ0��A1M3=0
    DS3231_WR_Byte(Alarm1_Address_Week, 0x00);    // ������Ϊ0��A1M4=0
    
    // ��������жϱ�־λ
    // uint8_t status = DS3231_RD_Byte(Address_control_status);
    // DS3231_WR_Byte(Address_control_status, status & ~0x01);  // ���A1F��־λ

    // // ���������ж�
    // uint8_t ctrl = DS3231_RD_Byte(Address_control);
    // DS3231_WR_Byte(Address_control, ctrl & ~0x01);  // ���A1IEλ
}

/*********************************************************************************************************************************************/
// �����趨�������ڲ����趨Ϊ����ʱ���֣���ƥ�����ӡ�
// ��ѡ���Ƿ�ƥ������
void DS3231_Set_alarm1_old(void)
{
    alarm_s alarm;
    u8 temp_a = 0;
    
    if (!alarm_getNext(&alarm))
    {
        printf("DS3231 alarm is not set, clear alarm\n");
        // �������, �������������
        // DS3231_WR_Byte(Alarm1_Address_Second, 0x80);  // A1M1=1��������ƥ��
        // DS3231_WR_Byte(Alarm1_Address_Minute, 0x80);  // A1M2=1�����Է���ƥ��
        // DS3231_WR_Byte(Alarm1_Address_Hour, 0x80);    // A1M3=1������Сʱƥ��
        // DS3231_WR_Byte(Alarm1_Address_Week, 0x80);    // A1M4=1, ��������/����ƥ��

        // �������
        DS3231_WR_Byte(Alarm1_Address_Second, 0x00);  // ����Ϊ0��A1M1=0
        DS3231_WR_Byte(Alarm1_Address_Minute, 0x00);  // ������Ϊ0��A1M2=0
        DS3231_WR_Byte(Alarm1_Address_Hour, 0x00);    // Сʱ��Ϊ0��A1M3=0
        DS3231_WR_Byte(Alarm1_Address_Week, 0x00);    // ������Ϊ0��A1M4=0
        
        // ��������жϱ�־λ
        // uint8_t status = DS3231_RD_Byte(Address_control_status);
        // DS3231_WR_Byte(Address_control_status, status & ~0x01);  // ���A1F��־λ

        // // ���������ж�
        // uint8_t ctrl = DS3231_RD_Byte(Address_control);
        // DS3231_WR_Byte(Address_control, ctrl & ~0x01);  // ���A1IEλ

        return;
    }

    /*
    // �����ã���������ʱ��Ϊ��ǰʱ���1����
    DS3231_Get_Time();  // ��ȡ��ǰʱ��
    alarm.hour = timeDate.time.hour;
    alarm.min = timeDate.time.mins + 1;
    if(alarm.min >= 60) {
        alarm.min = 0;
        alarm.hour++;
        if(alarm.hour >= 24) {
            alarm.hour = 0;
        }
    }
    */

}

void DS3231_Get_alarm1(void) // ��ȡ����1������
{
    /**********���ӵ�����*************/
    // test1 = DS3231_RD_Byte(Alarm1_Address_Second); // test����������ʾ�Ĵ����е�ֵ��ת�����ƺ����ں˶�ʹ�ã��ɹ۲����޸ļĴ���������
    // test2 = DS3231_RD_Byte(Address_control);
    // test3 = DS3231_RD_Byte(Address_control_status);
    /********************************/

    // calendar.alarm_week = DS3231_RD_Byte(Alarm1_Address_Week) & 0x0f; // ��ȡ0X0A�е��������ݣ�����λΪ�������ݣ�ͨ��&�������θ���λ
    // calendar.alarm_week = BCD2HEX(calendar.alarm_week);

    // calendar.alarm_hour = DS3231_RD_Byte(Alarm1_Address_Hour);
    // calendar.alarm_hour &= 0x3f; // ����0X09�����λ������A1M3��־λ��
    // calendar.alarm_hour = BCD2HEX(calendar.alarm_hour);

    // calendar.alarm_min = DS3231_RD_Byte(Alarm1_Address_Minute);
    // calendar.alarm_min = BCD2HEX(calendar.alarm_min);
    // calendar.alarm_sec = DS3231_RD_Byte(Alarm1_Address_Second);
    // calendar.alarm_sec = BCD2HEX(calendar.alarm_sec);
}

// void get_alarm1_status(void)//���0X0F���ӱ�־λA1F�Ƿ񴥷�����������ʾ1������Ĭ��Ϊ0
// {
//	 u8 a;
//	 a=DS3231_RD_Byte(0x0f);
//	 if(a&0x01)
//		 alarm1_flag=1;
//	 else
//		 alarm1_flag=0;
//
// }
// void enable_alarm1(void)
//{
//
//	DS3231_WR_Byte(0x0E,0x1f);
//}
// void reset_alarm1(void)
//{
//	u8 a;
//	a=DS3231_RD_Byte(0x0E);
//	DS3231_WR_Byte(0x0E,a&0xfe);
//	 a=DS3231_RD_Byte(0x0f);
//	DS3231_WR_Byte(0x0f,a&0xfe);
//}

void DS3231_Alarm_Handler(void)
{
    // Read control/status register to check which alarm triggered
    uint8_t status = DS3231_RD_Byte(Address_control_status);
    uint8_t ctrl = DS3231_RD_Byte(Address_control);
    
    printf("DS3231 interrupt triggered!\n");
    printf("Status Register (0x0F): 0x%02X\n", status); // 0x01��ʾ����1����
    printf("Control Register (0x0E): 0x%02X\n", ctrl); // 0x05��ʾ����1�ж�ʹ��
    
    // Check if Alarm 1 triggered (A1F bit)
    if(status & 0x01)
    {
        printf("Alarm 1 flag is set!\n");
        // Clear the alarm flag
        DS3231_WR_Byte(Address_control_status, status & ~0x01);
        
        // Verify flag was cleared
        status = DS3231_RD_Byte(Address_control_status);
        printf("Status after clearing (0x0F): 0x%02X\n", status);

        if (DeepSleepFlag) {
            printf("Waking up from deep sleep\n");
            nvic_wake_up(9);
        }
        userWake();
        alarm_update();
    }
    else
    {
        printf("Alarm 1 flag is NOT set!\n");
    }
}
