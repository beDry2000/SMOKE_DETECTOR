/**
  ******************************************************************************
  * @file    actuators.c
  * @brief   Unified driver: PB2 Status LED pattern engine and PB0 Passive Buzzer
  *          tone generator (2.7 kHz resonant square wave).
  ******************************************************************************
  */

#include "actuators.h"

static uint8_t  led_mode           = 0; /* 0: OFF, 1: ON, 2: BLINK_4S, 3: FAST_BLINK */
static uint32_t last_toggle_ms     = 0;
static uint8_t  led_pin_state      = 0;
static uint8_t  buzzer_tone_active = 0;
static uint32_t last_chirp_ms      = 0;
static uint8_t  fault_chirp_mode   = 0;

void actuators_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PB2 Status LED */
    GPIO_InitStruct.Pin   = STATUS_LED_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STATUS_LED_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET);

    /* PB0 Passive Buzzer */
    GPIO_InitStruct.Pin   = BUZZER_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void actuators_set_state(SystemState_t state, ControlAction_t control_action)
{
    fault_chirp_mode = 0;

    switch (state) {
        case STATE_OFF:
            led_mode = 0;
            buzzer_tone_active = 0;
            HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
            break;

        case STATE_STABILIZING:
        case STATE_ALERT:
            led_mode = 1; /* Steady ON */
            buzzer_tone_active = 0;
            HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
            break;

        case STATE_OK:
            led_mode = 2; /* Blink 2s ON / 2s OFF */
            buzzer_tone_active = 0;
            HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
            break;

        case STATE_ALARM:
            led_mode = 1; /* Steady ON */
            buzzer_tone_active = 1; /* Continuous 2.7 kHz tone */
            HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET);
            break;

        case STATE_CONTROL:
            if (control_action == CONTROL_ALARM_ON) {
                led_mode = 1;
                buzzer_tone_active = 1;
                HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET);
            } else {
                led_mode = 0;
                buzzer_tone_active = 0;
                HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
            }
            break;

        case STATE_FAULT:
            led_mode = 3; /* Fast Blink: 150ms ON / 150ms OFF */
            fault_chirp_mode = 1; /* Periodic warning chirp */
            break;
    }
}

void actuators_update(uint32_t now_ms)
{
    /* Normal 4-second Blink (STATE_OK: 2s ON / 2s OFF) */
    if (led_mode == 2) {
        if ((now_ms - last_toggle_ms) >= LED_OK_TOGGLE_MS) {
            last_toggle_ms = now_ms;
            led_pin_state = !led_pin_state;
            HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, 
                              led_pin_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    }
    /* Fast Blink for STATE_FAULT (150ms ON / 150ms OFF) */
    else if (led_mode == 3) {
        if ((now_ms - last_toggle_ms) >= 150U) {
            last_toggle_ms = now_ms;
            led_pin_state = !led_pin_state;
            HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, 
                              led_pin_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    }

    /* Continuous Tone in STATE_ALARM */
    if (buzzer_tone_active) {
        BUZZER_PORT->ODR ^= BUZZER_PIN;
    }
    /* Periodic Fault Chirp (50ms pulse every 15 seconds) */
    else if (fault_chirp_mode) {
        if ((now_ms - last_chirp_ms) >= 15000U) {
            last_chirp_ms = now_ms;
        }
        if ((now_ms - last_chirp_ms) < 50U) {
            BUZZER_PORT->ODR ^= BUZZER_PIN;
        } else {
            BUZZER_PORT->BRR = BUZZER_PIN;
        }
    } else {
        BUZZER_PORT->BRR = BUZZER_PIN;
    }
}
