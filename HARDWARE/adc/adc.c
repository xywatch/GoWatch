#include "adc.h"
#include "stm32l1xx_adc.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "delay.h"

/*

F103中ADC时钟来源：HSE ---> PLLCLK ---> HCLK ---> APB2 ---> ADC时钟

L151中ADC时钟来源：HSI ---> ADC时钟

F103中GPIO时钟来源：HSE ---> PLLCLK ---> HCLK ---> PCLK2 ---> APB2外设时钟

L151中GPIO时钟来源：HSE ---> PLLCLK ---> HCLK ---> PCLK1 ---> APB1外设时钟

所以在STM32L151中，必须使能初始化HSI时钟，不然ADC没法用！！

原文链接：https://blog.csdn.net/m0_37845735/article/details/105890138
*/
void Adc_HSI_Enable (void) {
    // 使能 HSI
    RCC_HSICmd(ENABLE);
    // 等待 HSI 就绪
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
}

/*
 * ADC 初始化函数
 * 功能：初始化 ADC_IN0，配置 PA0 为模拟输入
 * 参数：无
 * 返回值：无
 */
void Adc_Init(void) // PA0
{
    Adc_HSI_Enable();

    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能 GPIOA 和 ADC1 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // 配置 PA0 为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;    // 模拟输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;// 无上下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 复位 ADC1
    ADC_DeInit(ADC1);

    // 配置 ADC 参数
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;           // 12位分辨率
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                   // 非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;             // 非连续转换模式
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 无外部触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;          // 数据右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                      // 1个转换通道
    ADC_Init(ADC1, &ADC_InitStructure);

    // 使能 ADC1
    ADC_Cmd(ADC1, ENABLE);

    // 等待 ADC 稳定
    delay_ms(10);
}

/*
 * 获取 ADC 转换值
 * 功能：读取指定通道的 ADC 值
 * 参数：
 *   ch: ADC 通道号
 * 返回值：ADC 转换值（0-4095）
 */
u16 Get_Adc(u8 ch)
{
    // 配置 ADC 通道和采样时间
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_384Cycles);

    // 启动转换
    ADC_SoftwareStartConv(ADC1);

    // 等待转换完成
    uint32_t timeout = 0xFFFF;
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
        if(--timeout == 0) {
            printf("adc timeout\n");
            return 0;  // 超时保护
        }
    }

    return ADC_GetConversionValue(ADC1);
}

/*
 * 获取 ADC 平均值
 * 功能：对指定通道进行多次采样并计算平均值，去除最大最小值
 * 参数：
 *   ch: ADC 通道号
 *   times: 采样次数
 * 返回值：ADC 平均值（0-4095）
 */
u16 Get_Adc_Average(u8 ch, u8 times)
{
    u32 temp_val = 0;
    u8 t;
    u16 max_val = 0, min_val = 0xFFFF;
    u16 current_val;
    
    // 丢弃前4次采样值，等待 ADC 稳定
    for(t = 0; t < 4; t++) {
        Get_Adc(ch);
        delay_ms(1);  // 增加延时，确保 ADC 稳定
    }

    // 进行多次采样，记录最大最小值
    for (t = 0; t < times; t++)
    {
        current_val = Get_Adc(ch);
        if(current_val == 0) continue;  // 跳过超时值
        
        // 更新最大最小值
        if(current_val > max_val) max_val = current_val;
        if(current_val < min_val) min_val = current_val;
        
        temp_val += current_val;
        delay_ms(1);  // 增加采样间隔
    }
    
    // 去除最大最小值后计算平均值
    if(times > 2) {
        temp_val = (temp_val - max_val - min_val) / (times - 2);
    } else {
        temp_val = temp_val / times;
    }

    return temp_val;
}

// 配置电源控制引脚，确保在 STOP 模式下保持高电平
void Power_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能 GPIO 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    
    // 配置 GPIO 为推挽输出，高速模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  // 假设是 PA15，请根据实际引脚修改
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 设置高电平
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
}

// 在进入 STOP 模式前调用此函数
void Enter_STOP_Mode(void)
{
    // 确保电源控制引脚配置正确
    Power_GPIO_Config();
    
    // 配置唤醒源（如果需要）
    // PWR_WakeUpPinCmd(ENABLE);
    
    // 进入 STOP 模式
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    
    // 退出 STOP 模式后的处理
    // 重新配置系统时钟
    SystemInit();
    
    // 重新配置电源控制引脚
    Power_GPIO_Config();
}
