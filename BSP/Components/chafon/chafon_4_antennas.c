#include "chafon_4_antennas.h"
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart2;

//#define ANTENNA1 0x80
//#define ANTENNA2 0x81
//#define ANTENNA3 0x82
//#define ANTENNA4 0x83

#define ANSWER_COMMUNICATION_SIZE 0X11
#define EARRINGS_DATA_SIZE 0X15
#define END_PACK_DATA_SIZE 0X07
#define ANTENNA_CRC(ANT, CRC1, CRC2) \
    switch(ANT){             \
    case 0x80:               \
        CRC1 = 0xdd;     	 \
        CRC2 = 0x23;    	 \
    break;  			     \
    case 0x81:         		 \
        CRC1 = 0x05;   	 	 \
        CRC2 = 0x3a;    	 \
    break;  				 \
    case 0x82: 				 \
        CRC1 = 0x6d;   	   	 \
        CRC2 = 0x10;   		 \
    break;  				 \
    case 0x83: 				 \
        CRC1 = 0xb5;  		 \
        CRC2 = 0x09;    	 \
    break;  				 \
    }



uint8_t INIT_COMMUNICATION_CHAFON[] = {0x04, 0xFF, 0x21, 0x19, 0x95};
uint8_t	DATA_REQUEST[] = {0x09, 0x00, 0x01, 0x04, 0x00, 0x00, 0x80, 0x14, 0xdd, 0x23};
uint8_t	CHAFON_ANSWER[] = {	0x11, 0x00, 0x21, 0x00, 0x02, 0x01, 0x62, 0x02, 0x31, 0x80, 0x21, 0x00, 0x01, 0x01, 0x00, 0x00, 0xcd, 0xe0};




uint8_t reciverBuffer[1];
uint8_t data[500] = {};
uint8_t earring[100] = {};
uint8_t verification_buffer[18] = {};

uint16_t lastEarring = 0;
uint16_t earring_current = 0;
uint16_t contarray = 0;
uint16_t numberOfEarrings = 0;
uint16_t change = 0;
uint16_t contbyte = 0;

bool requestFlag = 0;
bool verificationFlag = 0;
bool communicationValidationFlag = 0;
bool cleanBuffFlag = 0;
bool recieverFlag = 0;

Model_earrings earrings[500];

static void init_Communication();
static void Chafon_Init_GPIO(void);
static void verification_Comunication_Buffer();
static void data_Validation();



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

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	uart_callback();
//}
void uart_callback()
{

	//HAL_UART_Receive_IT(&huart2, reciverBuffer,1);
}

void init_Communication()
{
	HAL_UART_Transmit(&huart2,(uint8_t *)INIT_COMMUNICATION_CHAFON, INIT_COMMUNICATION_CHAFON[0]+1,100);
	HAL_UART_Receive_IT(&huart2, reciverBuffer,1);
	data_Validation();

}

void getEarrings(Model_earrings *earring)
{
//	if(lastEarring >0 )
		memcpy(earring->N_TAG, &earrings->N_TAG, 12);
		//PRINTF("%x\n\n",*earring->N_TAG);
}

void data_request_chafon(ANTENNAS antenna)
{

	DATA_REQUEST[6] = antenna;
	ANTENNA_CRC(antenna, DATA_REQUEST[8],DATA_REQUEST[9]);
	HAL_UART_Transmit(&huart2, (uint8_t *)DATA_REQUEST, DATA_REQUEST[0]+1, 100);

}

void data_Validation()
{
	PRINTF("-----------------data: %x----------------------",data[0]);
	if(!verificationFlag  && data[0] == ANSWER_COMMUNICATION_SIZE)
		verification_Comunication_Buffer();

	if(recieverFlag && data[0] == EARRINGS_DATA_SIZE)
	{
		if(communicationValidationFlag)
		{
			PRINTF("-----------------COPY EARRINGS----------------------");
			memcpy(earrings[lastEarring++].N_TAG, &data[8], 12);
			communicationValidationFlag = 0;

		}else if(cleanBuffFlag  && data[0] == END_PACK_DATA_SIZE)
			{
				memset(data, 0 , data[0]+1);
				cleanBuffFlag = 0;
			}
	}

}


void verification_Comunication_Buffer()
{
	memcpy(verification_buffer,data,18);
	PRINTF("-----------------verification----------------------");

	if(verification_buffer[0] == 0x11 && memcmp(verification_buffer,CHAFON_ANSWER,CHAFON_ANSWER[0]+1) == 0)
	{
		communicationValidationFlag = 1;
		PRINTF("communication sucessfull chafon \n");
		memset(data, 0 , data[0]+1);
		recieverFlag = 1;
		verificationFlag = 1;

	}
	else if(memcmp(verification_buffer,CHAFON_ANSWER,CHAFON_ANSWER[0]+1) != 0)
	{

		//PRINTF("communication fail chafon \n");
//		recieverFlag = 0;


	}

}


