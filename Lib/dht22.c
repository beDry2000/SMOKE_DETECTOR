/**
  ******************************************************************************
  * @file    dht22.c
  * @brief   DHT22 1-Wire Driver using GPIO Push-Pull with External Pull-Up
  *          Switches between Output Push-Pull and Input Floating at runtime.
  ******************************************************************************
  */

#include "dht22.h"

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < cycles);
}

static void set_pin_output_pp(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = DHT22_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);
}

static void set_pin_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; /* External pull-up resistor handles line */
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);
}

void dht22_init(void)
{
    dwt_init();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    set_pin_output_pp();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
}

static uint8_t wait_state(GPIO_PinState state, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t timeout_cycles = timeout_us * (SystemCoreClock / 1000000U);
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) != state) {
        if ((DWT->CYCCNT - start) > timeout_cycles) return 0;
    }
    return 1;
}

DHT22_Status_t dht22_read(DHT22_Data_t *p_data)
{
    uint8_t raw[5] = {0};

    /* 1. Drive LOW as Push-Pull for 2 ms */
    set_pin_output_pp();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);

    /* 2. Release to Input (External resistor pulls line HIGH) */
    __disable_irq();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
    set_pin_input();
    delay_us(30);

    /* 3. Wait for sensor response: LOW 80us, HIGH 80us */
    if (!wait_state(GPIO_PIN_RESET, 100)) { __enable_irq(); goto err_exit; }
    if (!wait_state(GPIO_PIN_SET,   100)) { __enable_irq(); goto err_exit; }
    if (!wait_state(GPIO_PIN_RESET, 100)) { __enable_irq(); goto err_exit; }

    /* 4. Read 40 data bits */
    for (int i = 0; i < 40; i++) {
        if (!wait_state(GPIO_PIN_SET, 70)) { __enable_irq(); goto err_exit; }

        uint32_t t_start = DWT->CYCCNT;
        if (!wait_state(GPIO_PIN_RESET, 100)) { __enable_irq(); goto err_exit; }
        uint32_t width_us = (DWT->CYCCNT - t_start) / (SystemCoreClock / 1000000U);

        raw[i / 8] <<= 1;
        if (width_us > 40) {
            raw[i / 8] |= 1;
        }
    }
    __enable_irq();

    set_pin_output_pp();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);

    /* 5. Checksum verification */
    if (raw[4] != ((raw[0] + raw[1] + raw[2] + raw[3]) & 0xFF)) {
        p_data->is_valid = 0;
        return DHT22_ERR_CHECKSUM;
    }

    p_data->humidity_rh   = raw[0];
    p_data->temperature_c = raw[2];
    p_data->is_valid      = 1;
    return DHT22_OK;

err_exit:
    set_pin_output_pp();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
    p_data->is_valid = 0;
    return DHT22_ERR_TIMEOUT;
}
