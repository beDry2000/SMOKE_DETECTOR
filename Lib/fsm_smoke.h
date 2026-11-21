/**
  ******************************************************************************
  * @file    fsm_smoke.h
  * @brief   Finite State Machine for Smoke Detector Node
  ******************************************************************************
  */

#ifndef __FSM_SMOKE_H
#define __FSM_SMOKE_H

#include "app_config.h"

void fsm_smoke_init(void);
void fsm_smoke_update(uint32_t now_ms);
void fsm_smoke_get_telemetry_6b(TelemetryPacket6B_t *pkt);
void fsm_smoke_apply_command_6b(const CommandPacket6B_t *cmd);

#endif /* __FSM_SMOKE_H */
