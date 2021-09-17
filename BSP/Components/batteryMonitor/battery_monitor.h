/*
 * battery_monitor.h
 *
 *  Created on: Apr 22, 2021
 *  Author: Germano Henrique
 *  Brief: Declaration of macros and functions used by the program
 */

#ifndef INC_BATTERY_MONITOR_H_
#define INC_BATTERY_MONITOR_H_

#include "stm32f4xx_hal.h"
// function prototypes
void init_battery_monitor(ADC_HandleTypeDef *hadc_batt);
double get_battery_voltage (void);

#endif /* INC_BATTERY_MONITOR_H_ */


