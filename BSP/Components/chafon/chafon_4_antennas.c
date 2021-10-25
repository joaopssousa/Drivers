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
uint8_t lastEarring = 0;

int contbyte = 0;
int contarray = 0;
bool communFlag = 0;
bool cleanBuffFlag = 0;

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
			recieverFlag = 1;
		}
		else if(contbyte == data[0]+1 && data[0] == 0x15)
		{
			//recieverFlag = 1;
			communFlag = 1;
			contbyte = 0;
		}else if(contbyte == data[0]+1 && data[0] == 0x07)
		{cleanBuffFlag = 1;}

		HAL_UART_Receive_IT(&huart2,reciverBuffer, 1);
//	recieverFlag = 1;


}
void init_Communication()
{
	HAL_UART_Transmit(&huart2,(uint8_t *)initCommandData, initCommandData[0]+1,100);
	HAL_UART_Receive_IT(&huart2, reciverBuffer,1);

}

void sendUART()
{
	//HAL_UART_Receive_IT(&huart2, reciverBuffer, 5);
	if(recieverFlag)
	{
		recieverFlag = 0;
		verificationComunication();

	}
	if(communFlag)
	{
		memcpy(earring,data,data[0]+1);
		for(int i = 0; i < earring[0]+1 ; i++ )
		{
			PRINTF(" %x",earring[i]);
		}
		PRINTF("\n");
		communFlag = 0;
	}

	if(cleanBuffFlag)
	{
		memset(data, 0 , data[0]+1);
		PRINT("---clean---");
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

	if(data[0] == 0x11 && memcmp(data,communicationData,communicationData[0]+1) == 0)
			{
				HAL_UART_Transmit(&huart2, (uint8_t *)requestData, requestData[0]+1, 100);
				communFlag = 1;
				PRINTF("communication sucessfull chafon \n");
				memset(data, 0 , data[0]+1);

			}
			else
			{


				PRINTF("communication fail chafon \n");
			}

	//memset(reciverBuffer, 0 , reciverBuffer[0]+1);
}


void sendEarring(u_int8_t *earring)
{
	memcpy(earring,data,data[0]+1);
}

