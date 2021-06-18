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

extern uint32_t aux_count_velo;
extern uint32_t count_velo;
extern uint16_t flag;
extern TIM_HandleTypeDef htim2;

void muda_contador_zera_aux(uint32_t *count_velo, uint32_t *aux_count_velo) {
	*count_velo = *aux_count_velo;
	*aux_count_velo = 0;
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		//flag++;
		// Quando estourar o timer de 1 segundo exporta o numero de ciclos
		count_velo = aux_count_velo;
		aux_count_velo = 0;
		//muda_contador_zera_aux(&count_velo, &aux_count_velo);
	}
}

