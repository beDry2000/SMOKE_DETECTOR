/**
  ******************************************************************************
  * @file    mq2.c
  * @brief   MQ-2 Analog Output (AO) driver using ADC1 Channel 0 with EMA filter
  *          and calibrated 0 - 10,000 PPM conversion.
  ******************************************************************************
  */

#include "mq2.h"

static ADC_HandleTypeDef *mq2_hadc = NULL;
static uint16_t filtered_adc        = 0;
static uint32_t last_sample_time_ms = 0;

void mq2_init(ADC_HandleTypeDef *hadc)
{
    mq2_hadc = hadc;
    filtered_adc = 0;
    last_sample_time_ms = 0;
}

void mq2_update(uint32_t now_ms)
{
    if (!mq2_hadc) return;

    if ((now_ms - last_sample_time_ms) >= MQ2_SAMPLE_INTERVAL_MS) {
        last_sample_time_ms = now_ms;

        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel      = ADC_CHANNEL_0;
        sConfig.Rank         = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
        HAL_ADC_ConfigChannel(mq2_hadc, &sConfig);

        HAL_ADC_Start(mq2_hadc);
        if (HAL_ADC_PollForConversion(mq2_hadc, 10) == HAL_OK) {
            uint16_t raw = (uint16_t)HAL_ADC_GetValue(mq2_hadc);
            if (filtered_adc == 0) filtered_adc = raw;
            /* Exponential filter: alpha = 0.25 */
            filtered_adc = (uint16_t)((raw * 1 + filtered_adc * 3) / 4);
        }
        HAL_ADC_Stop(mq2_hadc);
    }
}

uint16_t mq2_get_raw_adc(void)
{
    return filtered_adc;
}

/**
  * @brief  Converts 12-bit ADC reading to calibrated PPM (200 - 10,000 PPM).
  */
uint16_t mq2_get_ppm(void)
{
    if (filtered_adc < 200) {
        return 200; /* Baseline clean ambient air */
    }
    uint32_t ppm = 200U + (((uint32_t)(filtered_adc - 200U) * 9800U) / 3800U);
    if (ppm > 10000U) ppm = 10000U;
    return (uint16_t)ppm;
}

uint8_t mq2_is_healthy(void)
{
    /* If ADC reads practically 0 (< 30) or pegged to rail (> 4070), sensor is dead/disconnected */
    if (filtered_adc < 30 || filtered_adc > 4070) {
        return 0;
    }
    return 1;
}
