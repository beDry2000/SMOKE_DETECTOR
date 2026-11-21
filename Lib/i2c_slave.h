/**
 ******************************************************************************
 * @file    i2c_slave.h
 * @brief   Interrupt-driven I2C Slave communication module for Smoke Node
 ******************************************************************************
 */

#ifndef __I2C_SLAVE_H
#define __I2C_SLAVE_H

#include "main.h"
#include "app_config.h"
#include "comm_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize the I2C Slave driver and start listening on the bus.
 * @param  hi2c Pointer to the I2C handle (e.g. &hi2c1)
 */
void i2c_slave_init(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_SLAVE_H */
