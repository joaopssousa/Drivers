/*
 * timer.h
 *
 *  Created on: Nov 9, 2020
 *      Author: LOCEM
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

/*
 * 	Contém os comportamentos e tratamentos dos timers de forma externa
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/*
 * 	Contém os comportamentos e tratamentos para interrupções de forma externa
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


#endif /* INC_TIMER_H_ */
