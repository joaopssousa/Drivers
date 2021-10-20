#include "chafon_4_antennas.h"
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart2;

uint8_t initCommandData[] = {0x04, 0xFF, 0x21, 0x19, 0x95};
uint8_t	requestData[] = {0x09, 0x00, 0x01, 0x04, 0x00, 0x00, 0x80, 0x14, 0xdd, 0x23};
uint8_t	communicationData[] = {0x11, 0x00, 0x21, 0x00, 0x02, 0x01, 0x62, 0x02, 0x31,
							   0x80, 0x21, 0x00, 0x01, 0x01, 0x00, 0x00, 0xcd, 0xe0};
bool recieverFlag = 0;

uint8_t data[500] = {};
uint8_t earring[100] = {};
uint8_t reciverBuffer[500]= {};


/* lista*/
typedef struct node {
    int val;
    struct node * next;
} node_t;

struct node *head = NULL;
struct node *current = NULL;

node_t * head = NULL;

head = (node_t *) malloc(sizeof(node_t));
head->val = 1;
head->next = (node_t *) malloc(sizeof(node_t));
head->next->val = 2;
head->next->next = NULL;

/* Fim lista*/

void initReciver();
void init_Communication();
void Irradiator_Init_GPIO(void);
void saveData();
void communicationSucessefull(bool erro);

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
	Irradiator_Init_GPIO() ;
	init_Communication();
	initReciver();

}
void Irradiator_Init_GPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_USART2_CLK_ENABLE();
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  //__HAL_RCC_GPIOB_CLK_ENABLE();


  /*Configure GPIO pin : BLE_BRK_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	recieverFlag = 1;

	saveData();

	HAL_UART_Receive_IT(&huart2,reciverBuffer, 22);

}
void init_Communication()
{
	HAL_UART_Transmit(&huart2,(uint8_t *)initCommandData, initCommandData[0]+1,100);
}

void sendUART()
{
	//HAL_UART_Receive_IT(&huart2, reciverBuffer, 5);
	if(recieverFlag)
	{
		//HAL_UART_Transmit(&huart2,(uint8_t *)data, data[0]+1,100);
		recieverFlag = 0;
	}

}
void initReciver()
{
	HAL_UART_Receive_IT(&huart2, reciverBuffer, 18);
}


void saveData()
{
	memcpy(data,reciverBuffer,50);
	uint8_t errordata[] = {0x04, 0xFF, 0xFF, 0xFF, 0xFF};

	if(data[0] == 0x11 && memcmp(data,communicationData,communicationData[0]) == 0)
	{
		HAL_UART_Transmit(&huart2, (uint8_t *)requestData, requestData[0]+1, 100);

	}
	else if(data[0] == 0x15)
		{

		}
	else if(data[0] == 0x11)
		{
			HAL_UART_Transmit(&huart2, (uint8_t *)errordata, errordata[0]+1, 100);
		}

	//memset(reciverBuffer, 0 , reciverBuffer[0]+1);
}


void sendEarring(u_int8_t *earring)
{
	memcpy(earring,data,11);
}

