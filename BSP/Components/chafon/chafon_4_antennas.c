#include "chafon_4_antennas.h"


UART_HandleTypeDef huart2;

uint8_t initCommandData[] = {0x04, 0xFF, 0x21, 0x19, 0x95};
uint8_t reciverBuffer[100]= {};


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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Transmit(&huart2,(uint8_t *)reciverBuffer, 1,100);
}
void init_Communication()
{
	HAL_UART_Transmit(&huart2,(uint8_t *)initCommandData, initCommandData[0]+1,100);
}

void callbackUART()
{
	HAL_UART_Receive_IT(&huart2, reciverBuffer, 1);

}

uint8_t reciverData()
{
	HAL_UART_Receive(&huart2, (uint8_t *)reciverBuffer, 15, 100);
	return reciverBuffer;
}




