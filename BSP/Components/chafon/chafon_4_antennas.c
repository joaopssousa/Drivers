#include "chafon_4_antennas.h"


UART_HandleTypeDef huart2;

uint8_t InitCommandData[] = {0x04, 0xFF, 0x21, 0x19, 0x95};


void INIT_ReaderUART(USART_TypeDef * uartPort,uint32_t baudRate)
{
	huart2.Instance = uartPort;
	huart2.Init.BaudRate = baudRate;
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


void Init_Communication()
{
	HAL_UART_Transmit(&huart2,(uint8_t *)InitCommandData, InitCommandData[0]+1,100);
}


