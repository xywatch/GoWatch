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

// DS3231 芯片的寄存器使用 BCD 码存储时间信息，所以在写入时间之前需要将普通十进制数转换为 BCD 码。比如：
// 写入小时 23 时，需要写入 0x23 而不是 0x17（23的十六进制）
// 写入分钟 45 时，需要写入 0x45 而不是 0x2D（45的十六进制）
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

// 原来的值：0=周一，1=周二，2=周三，3=周四，4=周五，5=周六，6=周日
// 目标值：  1=周日，2=周一，3=周二，4=周三，5=周四，6=周五，7=周六
// 转换公式：
// 如果是周日(6)，转成1
// 如果是其他日子(0-5)，加2就得到对应的值(2-7)
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
    
    // 清除控制寄存器
    DS3231_WR_Byte(Address_control, 0x00);
    delay_ms(2);
    
    // 清除状态寄存器中的所有标志位
    DS3231_WR_Byte(Address_control_status, Clear_A1IE_Flag);
    delay_ms(2);
    
    // 配置控制寄存器：
    // - 启用晶振 (OSC_Enable = 0)
    // - 启用闹钟1中断 (A1IE_Enable = 1)
    // - 使用中断输出模式 (OUTPUT_INTSQW = 1)
    // 0x05 = 0000 0101
    // bit 7: OSC_Enable = 0    (启用晶振)
    // bit 6: BBSQW = 0        (未使用)
    // bit 5: CONV = 0         (未使用)
    // bit 4-3: RS2-1 = 00     (未使用)
    // bit 2: INTCN = 1        (使用中断输出模式)
    // bit 1: A2IE = 0         (禁用闹钟2中断)
    // bit 0: A1IE = 1         (启用闹钟1中断)
    uint8_t ctrl = OSC_Enable | OUTPUT_INTSQW | A1IE_Enable;
    DS3231_WR_Byte(Address_control, ctrl);
    delay_ms(2);

    // 读取并打印配置
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
//     // 清除闹钟1的中断标志位
//     // 如果不清除这个标志位，INT引脚会一直保持低电平
//     // 下次闹钟就无法正常触发中断了
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

    // 转换星期格式：0-6（周一到周日）转换为1-7（周日到周六）
    u8 ds3231_day = system_week_to_DS3231_week(timeDate.date.day);
    temp = B_BCD(ds3231_day);
    DS3231_WR_Byte(0x03, temp);  // 写入转换后的星期值

    // 打印调试信息
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

    /*************读取3231芯片温度***************************************/

    calendar.temper_H = DS3231_RD_Byte(0x11);
    calendar.temper_L = (DS3231_RD_Byte(0x12) >> 6) * 25;
    DS3231_Temp = (float)calendar.temper_H + (float)calendar.temper_L / 100;
}

void DS3231_Set_alarm1(byte hour, byte min, day_t day)
{
		u8 temp_a = 0;
    printf("Setting alarm for %02d:%02d\n", hour, min);

    // 1. 秒设为0，A1M1=0（需要匹配）
    DS3231_WR_Byte(Alarm1_Address_Second, 0x00);  // 秒设为0，A1M1=0（需要匹配）
    // DS3231_WR_Byte(Alarm1_Address_Second, 0x80);  // 不需要匹配，A1M1=1（忽略匹配）不能用这个, 必须要秒设为0, 不然导致闹钟不触发
    
    // 2. 设置分钟，A1M2=0（需要匹配）
    temp_a = B_BCD(min);
    DS3231_WR_Byte(Alarm1_Address_Minute, temp_a);  // 不需要&0x7F，因为BCD值最大是0x59
    
    // 3. 设置小时，A1M3=0（需要匹配），24小时制
    temp_a = B_BCD(hour);
    DS3231_WR_Byte(Alarm1_Address_Hour, temp_a);  // 不需要&0x3F，因为BCD值最大是0x23
    
    // 4. 设置日期/星期，A1M4=1（忽略匹配）
    /*
    DS3231_WR_Byte(Alarm1_Address_Week, 0x80);
    // 读取并打印所有相关寄存器的值进行调试
    printf("\nDS3231 Registers after alarm set:\n");
    printf("Control(0x0E): 0x%02X (should be 0x05)\n", DS3231_RD_Byte(Address_control));
    printf("Status(0x0F): 0x%02X (should be 0x00)\n", DS3231_RD_Byte(Address_control_status));
    printf("Alarm1 Second: 0x%02X (should be 0x00)\n", DS3231_RD_Byte(Alarm1_Address_Second));
    printf("Alarm1 Minute: 0x%02X (should match %02d)\n", DS3231_RD_Byte(Alarm1_Address_Minute), min);
    printf("Alarm1 Hour: 0x%02X (should match %02d)\n", DS3231_RD_Byte(Alarm1_Address_Hour), hour);
    printf("Alarm1 Week/Date: 0x%02X\n", DS3231_RD_Byte(Alarm1_Address_Week));
    */

    // 闹钟1的星期/日期寄存器（0x0A）的格式如下：
    // bit 7: A1M4 (匹配位)
    // bit 6: DY/DT (1=按星期匹配，0=按日期匹配)
    // bit 5-0: 星期(1-7)或日期(1-31)的BCD值
    // 设置星期，A1M4=0（需要匹配），DY/DT=1（按星期匹配）
    
    byte nextAlarmDay = (byte)day;  // 下一个闹钟的星期, 0-6, 0:周一, 6:周日
    uint8_t ds3231_day = system_week_to_DS3231_week(nextAlarmDay); // (nextAlarmDay == 6) ? 1 : (nextAlarmDay + 2);
    temp_a = B_BCD(ds3231_day);  // 转换为DS3231的星期格式, 是因为 DS3231 的星期从 1 开始（1=周日，7=周六）
    DS3231_WR_Byte(Alarm1_Address_Week, (0x40 | temp_a));  // 0x40=按星期匹配，A1M4=0 0b 0100 0000

    // 打印调试信息
    printf("\nAlarm Configuration Verification:\n");
    printf("Day conversion: %d -> %d (DS3231 format)\n", nextAlarmDay, ds3231_day);
    printf("Time: %02d:%02d:00\n", hour, min);
    printf("Week register: 0x%02X\n", DS3231_RD_Byte(Alarm1_Address_Week));
    printf("Control: 0x%02X, Status: 0x%02X\n", 
           DS3231_RD_Byte(Address_control),
           DS3231_RD_Byte(Address_control_status));
    // 打印当前week
    printf("Current week: %d\n", nextAlarmDay); // 0-6, 0:周一, 6:周日
}

// 没用了, 因为至少有整点的闹钟
void DS3231_Clear_alarm1(void)
{
    // 清空闹钟
    DS3231_WR_Byte(Alarm1_Address_Second, 0x00);  // 秒设为0，A1M1=0
    DS3231_WR_Byte(Alarm1_Address_Minute, 0x00);  // 分钟设为0，A1M2=0
    DS3231_WR_Byte(Alarm1_Address_Hour, 0x00);    // 小时设为0，A1M3=0
    DS3231_WR_Byte(Alarm1_Address_Week, 0x00);    // 星期设为0，A1M4=0
    
    // 清除闹钟中断标志位
    // uint8_t status = DS3231_RD_Byte(Address_control_status);
    // DS3231_WR_Byte(Address_control_status, status & ~0x01);  // 清除A1F标志位

    // // 禁用闹钟中断
    // uint8_t ctrl = DS3231_RD_Byte(Address_control);
    // DS3231_WR_Byte(Address_control, ctrl & ~0x01);  // 清除A1IE位
}

/*********************************************************************************************************************************************/
// 闹钟设定函数，内部已设定为按照时，分，秒匹配闹钟。
// 可选择是否匹配星期
void DS3231_Set_alarm1_old(void)
{
    alarm_s alarm;
    u8 temp_a = 0;
    
    if (!alarm_getNext(&alarm))
    {
        printf("DS3231 alarm is not set, clear alarm\n");
        // 清空闹钟, 这里清空有问题
        // DS3231_WR_Byte(Alarm1_Address_Second, 0x80);  // A1M1=1，忽略秒匹配
        // DS3231_WR_Byte(Alarm1_Address_Minute, 0x80);  // A1M2=1，忽略分钟匹配
        // DS3231_WR_Byte(Alarm1_Address_Hour, 0x80);    // A1M3=1，忽略小时匹配
        // DS3231_WR_Byte(Alarm1_Address_Week, 0x80);    // A1M4=1, 忽略日期/星期匹配

        // 清空闹钟
        DS3231_WR_Byte(Alarm1_Address_Second, 0x00);  // 秒设为0，A1M1=0
        DS3231_WR_Byte(Alarm1_Address_Minute, 0x00);  // 分钟设为0，A1M2=0
        DS3231_WR_Byte(Alarm1_Address_Hour, 0x00);    // 小时设为0，A1M3=0
        DS3231_WR_Byte(Alarm1_Address_Week, 0x00);    // 星期设为0，A1M4=0
        
        // 清除闹钟中断标志位
        // uint8_t status = DS3231_RD_Byte(Address_control_status);
        // DS3231_WR_Byte(Address_control_status, status & ~0x01);  // 清除A1F标志位

        // // 禁用闹钟中断
        // uint8_t ctrl = DS3231_RD_Byte(Address_control);
        // DS3231_WR_Byte(Address_control, ctrl & ~0x01);  // 清除A1IE位

        return;
    }

    /*
    // 测试用：设置闹钟时间为当前时间后1分钟
    DS3231_Get_Time();  // 获取当前时间
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

void DS3231_Get_alarm1(void) // 获取闹钟1各参数
{
    /**********闹钟调试用*************/
    // test1 = DS3231_RD_Byte(Alarm1_Address_Second); // test变量用于显示寄存器中的值，转换进制后用于核对使用，可观察已修改寄存器内数据
    // test2 = DS3231_RD_Byte(Address_control);
    // test3 = DS3231_RD_Byte(Address_control_status);
    /********************************/

    // calendar.alarm_week = DS3231_RD_Byte(Alarm1_Address_Week) & 0x0f; // 获取0X0A中的星期数据，低四位为所需数据，通过&运算屏蔽高四位
    // calendar.alarm_week = BCD2HEX(calendar.alarm_week);

    // calendar.alarm_hour = DS3231_RD_Byte(Alarm1_Address_Hour);
    // calendar.alarm_hour &= 0x3f; // 屏蔽0X09中最高位（闹钟A1M3标志位）
    // calendar.alarm_hour = BCD2HEX(calendar.alarm_hour);

    // calendar.alarm_min = DS3231_RD_Byte(Alarm1_Address_Minute);
    // calendar.alarm_min = BCD2HEX(calendar.alarm_min);
    // calendar.alarm_sec = DS3231_RD_Byte(Alarm1_Address_Second);
    // calendar.alarm_sec = BCD2HEX(calendar.alarm_sec);
}

// void get_alarm1_status(void)//检测0X0F闹钟标志位A1F是否触发，触发则显示1，否则默认为0
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
    printf("Status Register (0x0F): 0x%02X\n", status); // 0x01表示闹钟1触发
    printf("Control Register (0x0E): 0x%02X\n", ctrl); // 0x05表示闹钟1中断使能
    
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
