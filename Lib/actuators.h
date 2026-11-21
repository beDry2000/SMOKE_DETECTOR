/**
  ******************************************************************************
  * @file    actuators.h
  * @brief   Unified driver for Status LED (PB2) and Passive Buzzer (PB0)
  ******************************************************************************
  */

#ifndef __ACTUATORS_H
#define __ACTUATORS_H

#include "app_config.h"

void actuators_init(void);
void actuators_update(uint32_t now_ms);
void actuators_set_state(SystemState_t state, ControlAction_t control_action);

#endif /* __ACTUATORS_H */
