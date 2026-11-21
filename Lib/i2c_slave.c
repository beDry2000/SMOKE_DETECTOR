/**
 ******************************************************************************
 * @file    i2c_slave.c
 * @brief   Interrupt-driven I2C Slave communication module for Smoke Node
 ******************************************************************************
 */

#include "i2c_slave.h"
#include "fsm_smoke.h"

static I2C_HandleTypeDef *slave_hi2c = NULL;

/* Static buffers for fixed 6-byte packets */
static TelemetryPacket6B_t tx_telemetry;
static uint8_t rx_buffer[sizeof(CommandPacket6B_t)];

void i2c_slave_init(I2C_HandleTypeDef *hi2c)
{
    slave_hi2c = hi2c;

    /* Start listening on the I2C bus with interrupts enabled */
    HAL_I2C_EnableListen_IT(slave_hi2c);
}

/**
 * @brief  Address match callback.
 * @param  hi2c Pointer to I2C handle
 * @param  TransferDirection Direction requested by Master:
 *         - I2C_DIRECTION_TRANSMIT: Master wants to write data to Slave
 *         - I2C_DIRECTION_RECEIVE:  Master wants to read data from Slave
 * @param  AddrMatchCode Matched slave address
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    UNUSED(AddrMatchCode);

    if (hi2c != slave_hi2c) return;

    if (TransferDirection == I2C_DIRECTION_TRANSMIT)
    {
        /* Master is writing to Slave -> Slave receives 6-byte command packet */
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_buffer, sizeof(rx_buffer), I2C_LAST_FRAME);
    }
    else
    {
        /* Master is reading from Slave -> Slave transmits fresh 6-byte telemetry packet */
        fsm_smoke_get_telemetry_6b(&tx_telemetry);
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t *)&tx_telemetry, sizeof(TelemetryPacket6B_t), I2C_LAST_FRAME);
    }
}

/**
 * @brief  Slave Reception Complete callback.
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != slave_hi2c) return;

    /* Apply the received 6-byte command */
    fsm_smoke_apply_command_6b((const CommandPacket6B_t *)rx_buffer);

    /* Re-arm listen mode */
    HAL_I2C_EnableListen_IT(hi2c);
}

/**
 * @brief  Slave Transmission Complete callback.
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != slave_hi2c) return;

    /* Re-arm listen mode */
    HAL_I2C_EnableListen_IT(hi2c);
}

/**
 * @brief  Listen Complete callback (bus STOP or repeated START detected).
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != slave_hi2c) return;

    /* Always ensure listen mode remains active */
    HAL_I2C_EnableListen_IT(hi2c);
}

/**
 * @brief  I2C error callback (bus error, arbitration lost, timeout, etc.).
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != slave_hi2c) return;

    /* Acknowledge Failure (AF) is expected at the end of slave transmit when Master sends NACK */
    if (HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF)
    {
        /* NACK received from master at end of read - normal completion */
    }

    /* Re-enable listen mode to recover gracefully */
    HAL_I2C_EnableListen_IT(hi2c);
}
