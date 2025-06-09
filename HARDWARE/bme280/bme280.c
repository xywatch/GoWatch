#include "bme280.h"
#include "common.h"
#include "oled_driver.h"

// BME280 传感器数据结构体
BME280_Data bme280_data;
// BME280 校准数据结构体
Bme280_Calib bme280_cal;
// 标准气压值（海平面气压）
float P_standard = 1015.0;
// 气压差值
uint difference_P;
// 海拔高度
int altitude;

/* BME280 过采样和工作模式配置 */
// 气压过采样设置：8倍过采样
#define BME280_PRES_OSR (BME280_OVERSAMP_8X)
// 温度过采样设置：8倍过采样
#define BME280_TEMP_OSR (BME280_OVERSAMP_8X)
// 湿度过采样设置：8倍过采样
#define BME280_HUMI_OSR (BME280_OVERSAMP_8X)
// 工作模式配置：组合过采样设置和强制模式
#define BME280_MODE (BME280_PRES_OSR << 2 | BME280_TEMP_OSR << 5 | BME280_FORCED_MODE)

/*
 * BME280 初始化函数
 * 功能：初始化 BME280 传感器，配置工作模式和读取校准数据
 * 硬件连接：
 *     SPI2_SCK  - PB13
 *     SPI2_MISO - PB14
 *     SPI2_MOSI - PB15
 *     SPI2_BMECS- PC12
 */
void Bme280_Init()
{
    // 复位 BME280
    Bme280_WriteData(0xE0, 0xB6);
    delay_ms(20);
    
    // 读取芯片 ID 进行验证
    bme280_data.id = Bme280_ReadData(0xD0);
    
    // 配置 BME280 工作模式
    Bme280_WriteData(BME280_CTRL_HUM, BME280_HUMI_OSR);    // 设置湿度过采样
    Bme280_WriteData(BME280_CTRL_MEAS_REG, 0xFF);          // 设置温度和气压过采样
    Bme280_WriteData(BME280_CONFIG_REG, 0x04);             // 设置滤波器系数
    
    // 读取校准数据
    readTrim();
}

/*
 * 读取 BME280 校准数据
 * 功能：读取并存储传感器的校准参数，用于后续的数据补偿计算
 */
void readTrim(void)
{
    // 读取温度校准数据
    bme280_cal.dig_T1 = (Bme280_ReadData(0x89) << 8) | Bme280_ReadData(0x88);
    bme280_cal.dig_T2 = (Bme280_ReadData(0x8B) << 8) | Bme280_ReadData(0x8A);
    bme280_cal.dig_T3 = (Bme280_ReadData(0x8D) << 8) | Bme280_ReadData(0x8C);
    
    // 读取气压校准数据
    bme280_cal.dig_P1 = (Bme280_ReadData(0x8F) << 8) | Bme280_ReadData(0x8E);
    bme280_cal.dig_P2 = (Bme280_ReadData(0x91) << 8) | Bme280_ReadData(0x90);
    bme280_cal.dig_P3 = (Bme280_ReadData(0x93) << 8) | Bme280_ReadData(0x92);
    bme280_cal.dig_P4 = (Bme280_ReadData(0x95) << 8) | Bme280_ReadData(0x94);
    bme280_cal.dig_P5 = (Bme280_ReadData(0x97) << 8) | Bme280_ReadData(0x96);
    bme280_cal.dig_P6 = (Bme280_ReadData(0x99) << 8) | Bme280_ReadData(0x98);
    bme280_cal.dig_P7 = (Bme280_ReadData(0x9B) << 8) | Bme280_ReadData(0x9A);
    bme280_cal.dig_P8 = (Bme280_ReadData(0x9D) << 8) | Bme280_ReadData(0x9C);
    bme280_cal.dig_P9 = (Bme280_ReadData(0x9F) << 8) | Bme280_ReadData(0x9E);
    
    // 读取湿度校准数据
    bme280_cal.dig_H1 = Bme280_ReadData(0xA1);
    bme280_cal.dig_H2 = (Bme280_ReadData(0xE2) << 8) | Bme280_ReadData(0xE1);
    bme280_cal.dig_H3 = Bme280_ReadData(0xE3);
    bme280_cal.dig_H4 = (Bme280_ReadData(0xE4) << 4) | (0x0F & Bme280_ReadData(0xE5));
    bme280_cal.dig_H5 = (Bme280_ReadData(0xE6) << 4) | ((Bme280_ReadData(0x5E) >> 4) & 0x0F);
    bme280_cal.dig_H6 = Bme280_ReadData(0xE7);
    delay_ms(200);
}

/*
 * 向 BME280 写入数据
 * 参数：
 *   addr: 寄存器地址
 *   data: 要写入的数据
 */
void Bme280_WriteData(u8 addr, u8 data)
{
    I2C_Start();
    I2C_SendByte(BME280_WRITE_ADDR);
    
    if (!I2C_WaitAck())
    {
        printf("BME280 write error at address 0x%02X\n", addr);
        I2C_Stop();
        return;
    }
    
    I2C_SendByte(addr);
    if (!I2C_WaitAck())
    {
        printf("BME280 write error at data 0x%02X\n", data);
        I2C_Stop();
        return;
    }
    
    I2C_SendByte(data);
    I2C_WaitAck();
    I2C_Stop();
}

/*
 * 从 BME280 读取数据
 * 参数：
 *   addr: 寄存器地址
 * 返回值：读取到的数据
 */
u8 Bme280_ReadData(u8 addr)
{
    u8 data = 0;
    
    I2C_Start();
    I2C_SendByte(BME280_WRITE_ADDR);
    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return 0;
    }
    
    I2C_SendByte(addr);
    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return 0;
    }
    
    I2C_Start();
    I2C_SendByte(BME280_READ_ADDR);
    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return 0;
    }
    
    data = I2C_ReceiveByte();
    I2C_NoAck();
    I2C_Stop();
    
    return data;
}

/*
 * 温度补偿计算
 * 功能：根据原始温度数据和校准参数计算实际温度
 * 返回值：温度值，分辨率为0.01°C
 * 例如：返回值5123表示51.23°C
 */
void bme280CompensateT(void)
{
    s32 Temp = 0;
    s32 var1, var2;
    // 读取原始温度数据
    Temp = (s32)((((u32)Bme280_ReadData(0xFA)) << 12) | (((u32)(Bme280_ReadData(0xFB))) << 4) | ((u32)Bme280_ReadData(0xFC) >> 4));
    // 温度补偿计算
    var1 = ((((Temp >> 3) - ((s32)bme280_cal.dig_T1 << 1))) * ((s32)bme280_cal.dig_T2)) >> 11;
    var2 = (((((Temp >> 4) - ((s32)bme280_cal.dig_T1)) * ((Temp >> 4) - ((s32)bme280_cal.dig_T1))) >> 12) * ((s32)bme280_cal.dig_T3)) >> 14;
    bme280_cal.t_fine = var1 + var2;
    // 转换为实际温度值
    bme280_data.T = (float)(((bme280_cal.t_fine * 5 + 128) >> 8) / 100.0);
}

/*
 * 气压补偿计算
 * 功能：根据原始气压数据和校准参数计算实际气压
 * 返回值：气压值，单位为Pa
 * 例如：返回值24674867表示96386.2 Pa = 963.862 hPa
 */
void bme280CompensateP(void)
{
    s32 var1, var2, Pres = 0;
    u32 p;
    float q;
    // 读取原始气压数据
    Pres = (s32)((((u32)Bme280_ReadData(0xF7)) << 12) | (((u32)(Bme280_ReadData(0xF8))) << 4) | ((u32)Bme280_ReadData(0xF9) >> 4));
    // 气压补偿计算
    var1 = (((s32)bme280_cal.t_fine) >> 1) - (s32)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((s32)bme280_cal.dig_P6);
    var2 = var2 + ((var1 * ((s32)bme280_cal.dig_P5)) << 1);
    var2 = (var2 >> 2) + (((s32)bme280_cal.dig_P4) << 16);
    var1 = (((bme280_cal.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((s32)bme280_cal.dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((s32)bme280_cal.dig_P1)) >> 15);

    if (var1 == 0)
    {
        return; // 避免除以零错误
    }

    p = (((u32)(((s32)1048576) - Pres) - (var2 >> 12))) * 3125;

    if (p < 0x80000000)
    {
        p = (p << 1) / ((u32)var1);
    }
    else
    {
        p = (p / (u32)var1) * 2;
    }

    var1 = (((s32)bme280_cal.dig_P9) * ((s32)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((s32)(p >> 2)) * ((s32)bme280_cal.dig_P8)) >> 13;
    p = (u32)((s32)p + ((var1 + var2 + bme280_cal.dig_P7) >> 4));
    q = (float)(p / 100.0 + 0); // 气压值补偿
    presssureFilter(&q, &bme280_data.P);
}

/*
 * 湿度补偿计算
 * 功能：根据原始湿度数据和校准参数计算实际湿度
 * 返回值：湿度值，单位为%RH
 */
void bme280CompensateH(void)
{
    s32 Humi = 0;
    double var_H;
    // 读取原始湿度数据
    Humi = (s32)((((u32)(Bme280_ReadData(0xFD))) << 8) | (u32)(Bme280_ReadData(0xFE)));
    // 湿度补偿计算
    var_H = (((double)bme280_cal.t_fine) - 76800.0);
    var_H = (Humi - (((double)bme280_cal.dig_H4) * 64.0 + ((double)bme280_cal.dig_H5) / 16384.0 * var_H)) * (((double)bme280_cal.dig_H2) / 65536.0 * (1.0 + ((double)bme280_cal.dig_H6) / 67108864.0 * var_H * (1.0 + ((double)bme280_cal.dig_H3) / 67108864.0 * var_H)));
    var_H = var_H * (1.0 - ((double)bme280_cal.dig_H1) * var_H / 524288.0);

    // 限制湿度值在0-100%范围内
    if (var_H > 100.0)
    {
        var_H = 100.0;
    }
    else if (var_H < 0.0)
    {
        var_H = 0.0;
    }

    bme280_data.H = (float)(((double)(var_H)) / 1.0);
    // 计算海拔高度（每升高1米气压下降9Pa）
    altitude = (P_standard - bme280_data.P) * 100 / 9;
}

// 滤波参数定义
#define FILTER_NUM 20    // 滤波数据长度
#define FILTER_A 0.1f    // 滤波系数

/*
 * 气压滤波函数
 * 功能：使用限幅平均滤波法对气压数据进行滤波处理
 * 参数：
 *   in: 输入气压值
 *   out: 输出滤波后的气压值
 */
void presssureFilter(float *in, float *out)
{
    u8 i = 0;
    float filter_buf[FILTER_NUM] = {0.0};
    double filter_sum = 0.0;
    u8 cnt = 0;
    float deta;

    // 初始化滤波缓冲区
    if (filter_buf[i] == 0.0f)
    {
        filter_buf[i] = *in;
        *out = *in;

        if (++i >= FILTER_NUM)
        {
            i = 0;
        }
    }
    else
    {
        // 计算相邻数据差值
        if (i)
        {
            deta = *in - filter_buf[i - 1];
        }
        else
        {
            deta = *in - filter_buf[FILTER_NUM - 1];
        }

        // 限幅滤波
        if (fabs(deta) < FILTER_A)
        {
            filter_buf[i] = *in;

            if (++i >= FILTER_NUM)
            {
                i = 0;
            }
        }

        // 计算平均值
        for (cnt = 0; cnt < FILTER_NUM; cnt++)
        {
            filter_sum += filter_buf[cnt];
        }

        *out = filter_sum / FILTER_NUM;
    }
}

/*
 * 显示海拔高度信息
 * 功能：在OLED上显示当前气压、标准气压、气压差和海拔高度
 */
void display_altitude(void)
{
    char height[1];
    char p[1];

    // 显示当前气压
    draw_string("Now_P:", NOINVERT, 0, 16);
    sprintf((char *)p, "%.1f", bme280_data.P);
    draw_string(p, NOINVERT, 50, 16);
    draw_string("hPa", NOINVERT, 100, 16);

    // 显示标准气压
    draw_string("Std_P:", NOINVERT, 0, 0);
    sprintf((char *)height, "%.1f", P_standard);
    draw_string(height, NOINVERT, 50, 0);
    draw_string("hpa", NOINVERT, 100, 0);

    // 计算并显示气压差
    difference_P = (P_standard - bme280_data.P) * 100;
    draw_string("dif:", NOINVERT, 0, 32);
    sprintf((char *)height, "%7d", difference_P);
    draw_string(height, NOINVERT, 30, 32);
    draw_string("Pa", NOINVERT, 100, 32);

    // 显示海拔高度
    sprintf((char *)height, "%d", 0);
    draw_string("Alt:", NOINVERT, 0, 48);
    altitude = (P_standard - bme280_data.P) * 100 / 9;
    sprintf((char *)height, "%d", altitude);
    draw_string(height, NOINVERT, 45, 48);
    draw_string("Meter", NOINVERT, 85, 48);
    sprintf((char *)height, "%d", 0);
}
