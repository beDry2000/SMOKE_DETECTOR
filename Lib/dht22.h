/**
  ******************************************************************************
  * @file    dht22.h
  * @brief   1-wire driver for DHT22 using GPIO Push-Pull with external pull-up
  ******************************************************************************
  */

#ifndef __DHT22_H
#define __DHT22_H

#include "app_config.h"

typedef enum {
    DHT22_OK           = 0,
    DHT22_ERR_TIMEOUT  = 1,
    DHT22_ERR_CHECKSUM = 2
} DHT22_Status_t;

typedef struct {
    uint8_t  temperature_c;
    uint8_t  humidity_rh;
    uint8_t  is_valid;
} DHT22_Data_t;

void           dht22_init(void);
DHT22_Status_t dht22_read(DHT22_Data_t *p_data);

#endif /* __DHT22_H */
