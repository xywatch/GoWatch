#include "adc.h"
#include "stm32l1xx_adc.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "delay.h"

/*

F103��ADCʱ����Դ��HSE ---> PLLCLK ---> HCLK ---> APB2 ---> ADCʱ��

L151��ADCʱ����Դ��HSI ---> ADCʱ��

F103��GPIOʱ����Դ��HSE ---> PLLCLK ---> HCLK ---> PCLK2 ---> APB2����ʱ��

L151��GPIOʱ����Դ��HSE ---> PLLCLK ---> HCLK ---> PCLK1 ---> APB1����ʱ��

������STM32L151�У�����ʹ�ܳ�ʼ��HSIʱ�ӣ���ȻADCû���ã���

ԭ�����ӣ�https://blog.csdn.net/m0_37845735/article/details/105890138
*/
void Adc_HSI_Enable (void) {
    // ʹ�� HSI
    RCC_HSICmd(ENABLE);
    // �ȴ� HSI ����
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
}

/*
 * ADC ��ʼ������
 * ���ܣ���ʼ�� ADC_IN0������ PA0 Ϊģ������
 * ��������
 * ����ֵ����
 */
void Adc_Init(void) // PA0
{
    Adc_HSI_Enable();

    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // ʹ�� GPIOA �� ADC1 ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // ���� PA0 Ϊģ������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;    // ģ������ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;// ��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ��λ ADC1
    ADC_DeInit(ADC1);

    // ���� ADC ����
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;           // 12λ�ֱ���
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                   // ��ɨ��ģʽ
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;             // ������ת��ģʽ
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // ���ⲿ����
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;          // �����Ҷ���
    ADC_InitStructure.ADC_NbrOfConversion = 1;                      // 1��ת��ͨ��
    ADC_Init(ADC1, &ADC_InitStructure);

    // ʹ�� ADC1
    ADC_Cmd(ADC1, ENABLE);

    // �ȴ� ADC �ȶ�
    delay_ms(10);
}

/*
 * ��ȡ ADC ת��ֵ
 * ���ܣ���ȡָ��ͨ���� ADC ֵ
 * ������
 *   ch: ADC ͨ����
 * ����ֵ��ADC ת��ֵ��0-4095��
 */
u16 Get_Adc(u8 ch)
{
    // ���� ADC ͨ���Ͳ���ʱ��
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_384Cycles);

    // ����ת��
    ADC_SoftwareStartConv(ADC1);

    // �ȴ�ת�����
    uint32_t timeout = 0xFFFF;
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
        if(--timeout == 0) {
            printf("adc timeout\n");
            return 0;  // ��ʱ����
        }
    }

    return ADC_GetConversionValue(ADC1);
}

/*
 * ��ȡ ADC ƽ��ֵ
 * ���ܣ���ָ��ͨ�����ж�β���������ƽ��ֵ��ȥ�������Сֵ
 * ������
 *   ch: ADC ͨ����
 *   times: ��������
 * ����ֵ��ADC ƽ��ֵ��0-4095��
 */
u16 Get_Adc_Average(u8 ch, u8 times)
{
    u32 temp_val = 0;
    u8 t;
    u16 max_val = 0, min_val = 0xFFFF;
    u16 current_val;
    
    // ����ǰ4�β���ֵ���ȴ� ADC �ȶ�
    for(t = 0; t < 4; t++) {
        Get_Adc(ch);
        delay_ms(1);  // ������ʱ��ȷ�� ADC �ȶ�
    }

    // ���ж�β�������¼�����Сֵ
    for (t = 0; t < times; t++)
    {
        current_val = Get_Adc(ch);
        if(current_val == 0) continue;  // ������ʱֵ
        
        // ���������Сֵ
        if(current_val > max_val) max_val = current_val;
        if(current_val < min_val) min_val = current_val;
        
        temp_val += current_val;
        delay_ms(1);  // ���Ӳ������
    }
    
    // ȥ�������Сֵ�����ƽ��ֵ
    if(times > 2) {
        temp_val = (temp_val - max_val - min_val) / (times - 2);
    } else {
        temp_val = temp_val / times;
    }

    return temp_val;
}

// ���õ�Դ�������ţ�ȷ���� STOP ģʽ�±��ָߵ�ƽ
void Power_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ʹ�� GPIO ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    
    // ���� GPIO Ϊ�������������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  // ������ PA15�������ʵ�������޸�
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ���øߵ�ƽ
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
}

// �ڽ��� STOP ģʽǰ���ô˺���
void Enter_STOP_Mode(void)
{
    // ȷ����Դ��������������ȷ
    Power_GPIO_Config();
    
    // ���û���Դ�������Ҫ��
    // PWR_WakeUpPinCmd(ENABLE);
    
    // ���� STOP ģʽ
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    
    // �˳� STOP ģʽ��Ĵ���
    // ��������ϵͳʱ��
    SystemInit();
    
    // �������õ�Դ��������
    Power_GPIO_Config();
}
