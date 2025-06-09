#include "common.h"

extern float BatteryVol; // �ĳ�u16�ᵼ���޷�������ʾ
millis_t milliseconds;

// Initialise library
// 1s = 1000ms ÿһms�ж�һ��
// 50 * 1440 / 72M = 1000/M = 1ms
void millis_init()
{
    // Timer settings
    // 1������Ƕ���жϿ�����NVIC
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//���ȼ�����
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM3_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic);

    // 2����ʱ����ʼ������
    TIM_TimeBaseInitTypeDef tim; // �ṹ��
    // ���ȼ���������
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // ����ʱ��
    TIM_DeInit(TIM3);                                    // ����������
    tim.TIM_ClockDivision = TIM_CKD_DIV1;                // ������Ƶ
    tim.TIM_CounterMode = TIM_CounterMode_Up;            // ���ϼ���
    tim.TIM_Period = 50 - 1;                             // ֮ǰ��50, Ӧ����49; //�Զ���װ�ؼĴ�����ֵ
    tim.TIM_Prescaler = 1440 - 1;                        // ʱ��Ԥ��Ƶ
    // tim.TIM_RepetitionCounter=
    TIM_TimeBaseInit(TIM3, &tim);         // ��ʼ���ṹ��
    TIM_ClearFlag(TIM3, TIM_FLAG_Update); // �������жϱ�־
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE); // ����ʱ��
}

// Get current milliseconds
millis_t millis_get()
{
    millis_t ms;
    ms = milliseconds;
    return ms;
}

extern int altitude;
u8 log_time = 1;

// ��ص�ѹ�˲�
static float batteryFilter(float newValue) {
    static float filteredValue = 0;
    static float lastValue = 0;
    const float alpha = 0.3f;    // �����˲�ϵ�����õ�ѹ�仯������Ӧ
    const float jumpThreshold = 0.1f; // ��ѹͻ����ֵ
    
    if(filteredValue == 0) {
        filteredValue = newValue;
        lastValue = newValue;
        return newValue;
    }
    
    // ����ѹͻ��
    if(fabs(newValue - lastValue) > jumpThreshold) {
        // ��ѹͻ��ʱ��С�˲�ϵ�����ӿ���Ӧ
        filteredValue = 0.6f * newValue + 0.4f * filteredValue;
    } else {
        // �����˲�
        filteredValue = alpha * newValue + (1-alpha) * filteredValue;
    }
    
    lastValue = newValue;
    return filteredValue;
}

// ��ص�ѹУ׼
static float calibrateVoltage(float rawVoltage) {
    // ����ʵ���ѹУ׼
    const float offset = -0.12f;  // 0 ƫ��У׼
    const float scale = 1.05f;    // 1 ����У׼

    float calibratedVoltage = rawVoltage * scale + offset;
    
    return calibratedVoltage;
}

// ��ʱ���жϺ�������
void TIM3_IRQHandler(void)
{
    static u8 adcCount = 0;
    static float voltageSum = 0;
    const u8 ADC_AVERAGE_COUNT = 5;  // ÿ5�β���ȡƽ��

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        milliseconds++;

        if (milliseconds % 200 == 0) // ÿ200ms����һ��ʱ��
        {
            update = true;
        }

        if (milliseconds % 2000 == 0) {  // ÿ2�����һ��
            printf("adc start\n");
            // ���ﵼ�¿���
            float rawVoltage = Get_Adc_Average(0, 50) * (3.3f / 4096.0f) * 2.0f;
            voltageSum += rawVoltage;
            adcCount++;
            
            if(adcCount >= ADC_AVERAGE_COUNT) {
                float avgVoltage = voltageSum / ADC_AVERAGE_COUNT;
                float filteredVoltage = batteryFilter(avgVoltage);
                BatteryVol = calibrateVoltage(filteredVoltage);
                printf("avgVoltage: %f, filteredVoltage: %f, BatteryVol: %f\n", avgVoltage, filteredVoltage, BatteryVol);
                
                voltageSum = 0;
                adcCount = 0;
            }
        }
    }

    TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update); // ����жϱ�־λ
}
