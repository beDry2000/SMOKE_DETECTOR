/**
  ******************************************************************************
  * @file    fsm_smoke.c
  * @brief   FSM implementation with multi-sensor logic and 6-byte telemetry
  ******************************************************************************
  */

#include "fsm_smoke.h"
#include "actuators.h"
#include "dht22.h"
#include "mq2.h"

static SystemState_t   current_state   = STATE_OFF;
static FaultCode_t     current_fault   = FAULT_NONE;
static ControlAction_t control_action  = CONTROL_ALARM_OFF;
static uint32_t        state_entry_ms  = 0;
static DHT22_Data_t    dht_data        = {0};
static uint32_t        last_dht_ms     = 0;
static uint8_t         dht_err_count   = 0;

/* Active Thresholds */
static uint16_t        th_smoke_alert_ppm = DEFAULT_SMOKE_ALERT_PPM;
static uint16_t        th_smoke_alarm_ppm = DEFAULT_SMOKE_ALARM_PPM;
static uint8_t         th_temp_alert_c    = DEFAULT_TEMP_ALERT_C;
static uint8_t         th_temp_alarm_c    = DEFAULT_TEMP_ALARM_C;

void fsm_smoke_init(void)
{
    current_state  = STATE_OFF;
    current_fault  = FAULT_NONE;
    state_entry_ms = HAL_GetTick();
    actuators_set_state(STATE_OFF, CONTROL_ALARM_OFF);
}

static SystemState_t evaluate_sensor_rules(void)
{
    /* Check sensor hardware health */
    if (!mq2_is_healthy()) {
        current_fault = FAULT_MQ2_OPEN;
        return STATE_FAULT;
    }
    if (dht_err_count >= 3) {
        current_fault = FAULT_DHT_TIMEOUT;
        return STATE_FAULT;
    }

    current_fault = FAULT_NONE;

    uint16_t smoke_ppm = mq2_get_ppm();
    uint8_t  temp      = dht_data.temperature_c;
    uint8_t  rh        = dht_data.humidity_rh;

    /* 1. Critical Alarm: Temp >= 60 C OR Smoke >= Alarm Threshold */
    if (temp >= th_temp_alarm_c || smoke_ppm >= th_smoke_alarm_ppm) {
        return STATE_ALARM;
    }

    /* 2. Dual-Sensor Confirmation */
    if (smoke_ppm >= th_smoke_alert_ppm && temp >= th_temp_alert_c) {
        return STATE_ALARM;
    }

    /* 3. Steam Suppression: Smoke detected but high humidity & normal temp */
    if (smoke_ppm >= th_smoke_alert_ppm && rh >= DEFAULT_STEAM_HUMIDITY_RH && temp < th_temp_alert_c) {
        return STATE_ALERT; /* Suppress buzzer alarm */
    }

    /* 4. Warning Alert: Light smoke alone or moderate temperature alone */
    if (smoke_ppm >= th_smoke_alert_ppm || temp >= th_temp_alert_c) {
        return STATE_ALERT;
    }

    /* 5. Normal */
    return STATE_OK;
}

void fsm_smoke_update(uint32_t now_ms)
{
    uint8_t switch_is_on = (HAL_GPIO_ReadPin(SWITCH_PORT, SWITCH_PIN) == GPIO_PIN_RESET);

    /* Periodic DHT22 sampling (every 2.0s) */
    if (current_state != STATE_OFF && (now_ms - last_dht_ms) >= DHT22_SAMPLE_INTERVAL_MS) {
        last_dht_ms = now_ms;
        DHT22_Status_t st = dht22_read(&dht_data);
        if (st == DHT22_OK) {
            dht_err_count = 0;
        } else {
            if (dht_err_count < 10) dht_err_count++;
        }
    }

    switch (current_state) {
        case STATE_OFF:
            if (switch_is_on) {
                current_state  = STATE_STABILIZING;
                state_entry_ms = now_ms;
                actuators_set_state(STATE_STABILIZING, CONTROL_ALARM_OFF);
            }
            break;

        case STATE_STABILIZING:
            if (!switch_is_on) {
                current_state = STATE_OFF;
                actuators_set_state(STATE_OFF, CONTROL_ALARM_OFF);
            } else if ((now_ms - state_entry_ms) >= MQ2_WARMUP_TIME_MS) {
                current_state = STATE_OK;
                actuators_set_state(STATE_OK, CONTROL_ALARM_OFF);
            }
            break;

        case STATE_OK:
        case STATE_ALERT:
        case STATE_ALARM:
        case STATE_FAULT:
            if (!switch_is_on) {
                current_state = STATE_OFF;
                actuators_set_state(STATE_OFF, CONTROL_ALARM_OFF);
            } else {
                SystemState_t recommended = evaluate_sensor_rules();
                if (recommended != current_state) {
                    current_state = recommended;
                    actuators_set_state(current_state, CONTROL_ALARM_OFF);
                }
            }
            break;

        case STATE_CONTROL:
            if (!switch_is_on) {
                current_state = STATE_OFF;
                actuators_set_state(STATE_OFF, CONTROL_ALARM_OFF);
            }
            break;
    }
}

void fsm_smoke_get_telemetry_6b(TelemetryPacket6B_t *pkt)
{
    pkt->node_id       = THIS_NODE_IDENTIFIER;
    pkt->status_reg    = pack_status_reg(current_state, current_fault);
    pkt->temperature_c = dht_data.temperature_c;
    pkt->humidity_rh   = dht_data.humidity_rh;
    pkt->smoke_ppm     = mq2_get_ppm();
}

void fsm_smoke_apply_command_6b(const CommandPacket6B_t *cmd)
{
    if (cmd->target_node_id != THIS_NODE_IDENTIFIER && cmd->target_node_id != 0xFFU) {
        return;
    }

    uint8_t cmd_id = cmd->command_reg & 0x03U;

    if (cmd_id == CMD_SET_CONTROL) {
        control_action = (cmd->command_reg >> 2) & 0x01U;
        if (control_action == CONTROL_ALARM_ON) {
            current_state = STATE_CONTROL;
            actuators_set_state(STATE_CONTROL, CONTROL_ALARM_ON);
        } else {
            current_state = STATE_OK;
            actuators_set_state(STATE_OK, CONTROL_ALARM_OFF);
        }
    } else if (cmd_id == CMD_SET_THRESHOLDS) {
        if (cmd->temp_alert_c > 0)    th_temp_alert_c    = cmd->temp_alert_c;
        if (cmd->temp_alarm_c > 0)    th_temp_alarm_c    = cmd->temp_alarm_c;
        if (cmd->smoke_alarm_ppm > 0) th_smoke_alarm_ppm = cmd->smoke_alarm_ppm;
    }
}
