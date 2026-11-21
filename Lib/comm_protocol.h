/**
  ******************************************************************************
  * @file    comm_protocol.h
  * @brief   Standardized 6-byte packet definitions (No Checksum)
  *          Shared between Master and Slave Nodes.
  ******************************************************************************
  */

#ifndef __COMM_PROTOCOL_H
#define __COMM_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ========================================================================== */
/*                              STATE MACHINE ENUMS                           */
/* ========================================================================== */
typedef enum {
    STATE_OFF          = 0x00, /* Hardware switch OFF: All outputs OFF */
    STATE_STABILIZING  = 0x01, /* Sensor warmup: LED Steady ON, Buzzer OFF */
    STATE_OK           = 0x02, /* Normal: LED Blinks (2s ON/2s OFF), Buzzer OFF */
    STATE_ALERT        = 0x03, /* Warning: LED Steady ON, Buzzer OFF */
    STATE_ALARM        = 0x04, /* Fire Hazard: LED Steady ON, Buzzer Steady ON */
    STATE_CONTROL      = 0x05, /* Master Forced Override (Force Alarm ON or OFF) */
    STATE_FAULT        = 0x06  /* Sensor / Bus failure */
} SystemState_t;

/* ========================================================================== */
/*                              FAULT CODE ENUMS                              */
/* ========================================================================== */
typedef enum {
    FAULT_NONE         = 0x00,
    FAULT_DHT_TIMEOUT  = 0x01, /* DHT22 disconnected / cut line */
    FAULT_DHT_CHECKSUM = 0x02, /* DHT22 parity mismatch */
    FAULT_MQ2_OPEN     = 0x03, /* MQ2 open circuit / heater dead */
    FAULT_MQ2_WARMUP   = 0x04, /* MQ2 warmup stabilization timeout */
    FAULT_I2C_BUS      = 0x05  /* I2C bus error */
} FaultCode_t;

/* ========================================================================== */
/*                            MASTER COMMAND ENUMS                            */
/* ========================================================================== */
typedef enum {
    CMD_PING           = 0x00,
    CMD_SET_CONTROL    = 0x01,
    CMD_SET_THRESHOLDS = 0x02
} CommandId_t;

typedef enum {
    CONTROL_ALARM_OFF  = 0x00,
    CONTROL_ALARM_ON   = 0x01
} ControlAction_t;

/* ========================================================================== */
/*                        6-BYTE TELEMETRY STRUCTURE                          */
/* ========================================================================== */
typedef struct __attribute__((packed)) {
    uint8_t  node_id;         /* (Room_ID << 4) | Sensor_Index */
    uint8_t  status_reg;      /* Bits [2:0]: State, Bits [7:3]: FaultCode */
    uint8_t  temperature_c;   /* Celsius (0 to 120 C) */
    uint8_t  humidity_rh;     /* %RH (0 to 100 %RH) */
    uint16_t smoke_ppm;       /* Calibrated PPM (0 to 10,000 PPM) */
} TelemetryPacket6B_t;

/* ========================================================================== */
/*                          6-BYTE COMMAND STRUCTURE                          */
/* ========================================================================== */
typedef struct __attribute__((packed)) {
    uint8_t  target_node_id;  /* (Room_ID << 4) | Sensor_Index */
    uint8_t  command_reg;     /* Bits [1:0]: CmdID, Bit 2: Action */
    uint8_t  temp_alert_c;    /* Warning threshold in C */
    uint8_t  temp_alarm_c;    /* Alarm threshold in C */
    uint16_t smoke_alarm_ppm; /* Alarm threshold in PPM */
} CommandPacket6B_t;

/* Packing Helper */
static inline uint8_t pack_status_reg(SystemState_t state, FaultCode_t fault)
{
    return (uint8_t)(((fault & 0x1FU) << 3) | (state & 0x07U));
}

#ifdef __cplusplus
}
#endif

#endif /* __COMM_PROTOCOL_H */
