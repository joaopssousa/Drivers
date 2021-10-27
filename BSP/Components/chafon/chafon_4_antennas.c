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
uint8_t verification[18] = {};
uint8_t reciverBuffer[500]= {};

int lastEarring = 0;
int contbyte = 0;
int contarray = 0;
int numberOfEarrings = 0;
int change = 0;
bool communFlag = 0;
bool cleanBuffFlag = 0;
bool requestFlag = 0;
bool verificationFlag = 0;

Model_earrings earrings[200];

void initReciver();
void init_Communication();
void Chafon_Init_GPIO(void);
void verificationComunication();
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
	Chafon_Init_GPIO() ;
	init_Communication();
	//initReciver();



}

void Chafon_Init_GPIO(void)
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
//HAL_UART_Transmit(&huart2, (uint8_t *)reciverBuffer, reciverBuffer[0]+1, 100);

		data[contbyte] =  *reciverBuffer;
		//PRINTF("teste ---- %x -- %d\n\n", data[contbyte], contbyte);
		contbyte++;
		if(contbyte == data[0]+1 && data[0] == 0x11)
		{
			contbyte = 0;
		}
		else if(contbyte == data[0]+1 && data[0] == 0x15)
		{
			communFlag = 1;
			contbyte = 0;
		}else if(contbyte == data[0]+1 && data[0] == 0x07)
		{
			cleanBuffFlag = 1;
			contbyte = 0;
		}
		HAL_UART_Receive_IT(&huart2, reciverBuffer,1);
//	recieverFlag = 1;


}
void init_Communication()
{
	HAL_UART_Transmit(&huart2,(uint8_t *)initCommandData, initCommandData[0]+1,100);
	HAL_UART_Receive_IT(&huart2, reciverBuffer,1);

}
void getEarrings()
{
	sendUART();
}

uint8_t antennachange(bool ant1, bool ant2, bool ant3, bool ant4)
{
	int contAntenna = 0;

	if(ant1 && change == 0)
	{
		return 0x80;
	}else if (ant2 && change == 1)
	{
		return 0x81;
	}else if (ant3 && change == 2)
	{
		return 0x82;
	}else if (ant4 && change == 3)
	{
		return 0x83;
	}

}

void sendUART()
{
	//HAL_UART_Receive_IT(&huart2, reciverBuffer, 5);
	if(!verificationFlag)
		verificationComunication();

	if(recieverFlag)
	{
		if(communFlag)
		{
			memcpy(earring,data,data[0]+1);

			memcpy(earrings[lastEarring++].N_TAG, &earring[8], 12);

			communFlag = 0;
			memset(earring,0, 12);

		}else if(cleanBuffFlag)
			{
				memset(data, 0 , data[0]+1);
				cleanBuffFlag = 0;
				HAL_UART_Transmit(&huart2, (uint8_t *)requestData, requestData[0]+1, 100);

			}
	}

}
void initReciver()
{
	HAL_UART_Receive_IT(&huart2, reciverBuffer, 18);
	communFlag = 1;

//	if(contbyte == data[0]+1)
//	{
//		HAL_UART_Transmit(&huart2, (uint8_t *)data, data[0]+1, 100);
//		contbyte = 0;
//		verificationComunication();
//	}



}


void verificationComunication()
{
	memcpy(verification,data,18);

	if(verification[0] == 0x11 && memcmp(verification,communicationData,communicationData[0]+1) == 0)
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)requestData, requestData[0]+1, 100);
				communFlag = 1;
				PRINTF("communication sucessfull chafon \n");
				memset(data, 0 , data[0]+1);
				recieverFlag = 1;
				verificationFlag = 1;

			}
			else if(memcmp(verification,communicationData,communicationData[0]+1) != 0)
			{

				//PRINTF("communication fail chafon \n");
				recieverFlag = 0;


			}

	//memset(reciverBuffer, 0 , reciverBuffer[0]+1);
}


void sendEarring(u_int8_t *earring)
{
	//memcpy(earring,data,data[0]+1);
}

