/*
 * timer.c
 *
 *  Created on: Nov 9, 2020
 *      Author: LOCEM
 */

/*
 * 		Biblioteca para armazenar a função do timer.
 * 		Caso se queira modificar e adicionar comportamento a outro timer modificar a instancia
 */

#include "timer.h"
#include "station.h"


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2)
	{
		// Quando estourar o timer de 1 segundo exporta o numero de ciclos
		count_velo = aux_count_velo;
		aux_count_velo = 0;
		flagsStation.pluviometer = 1;
	}
	if (htim->Instance == TIM3)
	{
		//Timeouts
		flagsStation.read_sensors=1;
	}
}

