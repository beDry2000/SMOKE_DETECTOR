/**
  ******************************************************************************
  * @file    app_config.h
  * @brief   Pin definitions and operational constants for Smoke Detector Node
  ******************************************************************************
  */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include "main.h"
#include "comm_protocol.h"

/* ========================================================================== */
/*                           HARDWARE PIN ASSIGNMENTS                         */
/* ========================================================================== */

/* Onboard Status LED: PB2 */
#define STATUS_LED_PORT             GPIOB
#define STATUS_LED_PIN              GPIO_PIN_2

/* Passive Buzzer: PB0 */
#define BUZZER_PORT                 GPIOB
#define BUZZER_PIN                  GPIO_PIN_0

/* DHT22 1-Wire: PA1 (Push-Pull with External Pull-Up) */
#define DHT22_PORT                  GPIOA
#define DHT22_PIN                   GPIO_PIN_1

/* MQ-2 Analog AO: PA0 (ADC1_IN0) */
#define MQ2_AO_PORT                 GPIOA
#define MQ2_AO_PIN                  GPIO_PIN_0

/* Hardware Switch: PB12 */
#define SWITCH_PORT                 GPIOB
#define SWITCH_PIN                  GPIO_PIN_12

/* ========================================================================== */
/*                           NODE IDENTIFICATION                              */
/* ========================================================================== */

/* Node Identification: Kitchen Smoke 1 (Room 2, Smoke 0 -> 0x20) */
#define THIS_ROOM_ID                0x02U   /* Kitchen */
#define THIS_SENSOR_INDEX           0x00U   /* Smoke Node 1 */
#define THIS_NODE_IDENTIFIER        ((THIS_ROOM_ID << 4) | (THIS_SENSOR_INDEX))

/* ========================================================================== */
/*                           I2C SLAVE CONFIGURATION                          */
/* ========================================================================== */
#define I2C_SLAVE_PORT              GPIOB
#define I2C_SCL_PIN                 GPIO_PIN_6
#define I2C_SDA_PIN                 GPIO_PIN_7

#define I2C_SLAVE_ADDR_7BIT         THIS_NODE_IDENTIFIER        /* 0x20 (7-bit address) */
#define I2C_SLAVE_OWN_ADDRESS       (I2C_SLAVE_ADDR_7BIT << 1)  /* 0x40 (8-bit HAL OwnAddress1) */

/* ========================================================================== */
/*                             TIMING & THRESHOLDS                            */
/* ========================================================================== */

/* Timing Constants (ms) */
#define MQ2_WARMUP_TIME_MS          20000U
#define DHT22_SAMPLE_INTERVAL_MS    2000U
#define MQ2_SAMPLE_INTERVAL_MS      100U
#define LED_OK_TOGGLE_MS            2000U

/* Default Thresholds */
#define DEFAULT_SMOKE_ALERT_PPM     800U    /* PPM */
#define DEFAULT_SMOKE_ALARM_PPM     1500U   /* PPM */
#define DEFAULT_TEMP_ALERT_C        50U     /* Celsius */
#define DEFAULT_TEMP_ALARM_C        60U     /* Celsius */
#define DEFAULT_STEAM_HUMIDITY_RH   80U     /* %RH */

#endif /* __APP_CONFIG_H */
