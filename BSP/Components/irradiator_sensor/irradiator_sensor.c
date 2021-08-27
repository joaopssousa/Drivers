/*
 * irradiator_sensor.c
 *
 *  Created on: Jul 6, 2021
 */


#include "main.h"
#include "irradiator_sensor.h"
#include "station.h"
#include <string.h>
#include <stdlib.h>

//Macros locais
#define ZERO	        0x30
#define NOVE        	0x39
#define MAX_MEASURES  5

UART_HandleTypeDef huart2;

uint8_t bit_Data[PACKET_SIZE] = {0};
uint8_t count_byte_irradiator = 0;
uint8_t count_measures = 0;

float measures = 0;


/*!
 * @brief Configure gpio
 * @param none
 * @retval none
 */
static void Irradiator_Init_GPIO(void);

/*!
 * @brief Configure uart2
 * @param none
 * @retval none
 */
static void MX_USART2_UART_Init(void);

void init_irradiator(void) {
	Irradiator_Init_GPIO();
	MX_USART2_UART_Init();
	HAL_UART_Receive_IT(&huart2, (uint8_t*)bit_Data, 1);
	//PRINTF("\nIRRADIATOR INICIALIZADO\n");
}

uint32_t getIntMeasure(void){
    uint8_t pos=0;
    uint8_t bufferInt[10];

		for(int i=0;i<(count_byte_irradiator-1);i++){
			if(bit_Data[i]==' ')
				break;
			if(bit_Data[i] >= ZERO && bit_Data[i] <= NOVE) {
				bufferInt[pos] = bit_Data[i];
				pos++;
			}
		}
    return atoi((const char*)&bufferInt);
}

uint32_t mediaCalculator(uint8_t number) {
	float media = 0;
	media = (float)(measures/number);
	measures = 0;
	return (int)media;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance != USART2) {
		return;
	}
  else{
    count_byte_irradiator++;
    if(count_byte_irradiator > 4) {
      if(bit_Data[count_byte_irradiator-1] == '\n') {
        flagsStation.receive_measure_irrad = 1;
      }
    }
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&(bit_Data[count_byte_irradiator]), 1);
  }
}

static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}


static void Irradiator_Init_GPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_USART2_CLK_ENABLE();
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  //__HAL_RCC_GPIOB_CLK_ENABLE();


  GPIO_InitStruct.Pin = IRRADIATOR_TX_Pin|IRRADIATOR_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}
