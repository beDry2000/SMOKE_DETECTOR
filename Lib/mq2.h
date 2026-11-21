/**
  ******************************************************************************
  * @file    mq2.h
  * @brief   ADC1 driver for MQ-2 Analog Output (AO) with 0-10,000 PPM calibration
  ******************************************************************************
  */

#ifndef __MQ2_H
#define __MQ2_H

#include "stm32f1xx_hal.h"
#include "app_config.h"
#include "main.h"

void     mq2_init(ADC_HandleTypeDef *hadc);
void     mq2_update(uint32_t now_ms);
uint16_t mq2_get_raw_adc(void);
uint16_t mq2_get_ppm(void);           /* Calibrated 0 to 10,000 PPM */
uint8_t  mq2_is_healthy(void);        /* 1 = OK, 0 = Sensor open/dead */

#endif /* __MQ2_H */
