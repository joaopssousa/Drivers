#include "chafon_4_antennas.h"
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart2;

//#define ANTENNA1 0x80
//#define ANTENNA2 0x81
//#define ANTENNA3 0x82
//#define ANTENNA4 0x83

#define ANSWER_COMMUNICATION_SIZE 0X11
#define EARRINGS_DATA_SIZE		  0X15
#define END_PACK_DATA_SIZE		  0X07

#define EARRING_SIZE			  12
#define EARRINGS_MAX_SIZE		  500

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

uint8_t	DATA_REQUEST[] 				= {0x09, 0x00, 0x01, 0x04, 0x00, 0x00, 0x80, 0x14, 0xdd, 0x23};

uint8_t	CHAFON_ANSWER[]				= {0x11, 0x00, 0x21, 0x00, 0x02, 0x01, 0x62, 0x02, 0x31,
									   0x80, 0x21, 0x00, 0x01, 0x01, 0x00, 0x00, 0xcd, 0xe0};

uint8_t reciver_buffer[1];
uint8_t data[EARRINGS_MAX_SIZE] = {};
uint8_t verification_buffer[21] = {};

uint16_t last_earring = 0;
uint16_t earring_current = 0;
uint16_t number_earrings = 0;
uint16_t count_byte = 0;

bool verification_flag = 0;
bool communication_validation_flag = 0;
bool reciever_flag = 0;

Model_earrings earrings[EARRINGS_MAX_SIZE] = {};

static void chafon_init_GPIO(void);
static void verification_Comunication_Buffer();
static uint16_t check_earring_size();

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
	chafon_init_GPIO() ;

}

static void chafon_init_GPIO(void)
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


void init_Communication()
{
	HAL_UART_Receive_IT(&huart2, reciver_buffer,1);
	HAL_UART_Transmit(&huart2,(uint8_t *)INIT_COMMUNICATION_CHAFON, INIT_COMMUNICATION_CHAFON[0]+1,100);
}

uint16_t get_Earrings(Model_earrings *earring)
{
	if(last_earring > 0 && check_earring_size() )
	{
		memcpy(earring->N_TAG, &earrings[number_earrings].N_TAG, EARRING_SIZE);
		//PRINTF("(%d) ", number_earrings);
		return 1;
	}
	return 0;
}

static uint16_t check_earring_size()
{
	if(number_earrings < last_earring){
		return 1;
	}

	else
	{
		PRINTF("\n-----ZEROU------(%d)(%d)",number_earrings,last_earring);
		number_earrings = 0;
		last_earring = 0;
		return 0;
	}
}

void data_request_chafon(ANTENNAS antenna)
{
	DATA_REQUEST[6] = antenna;
	ANTENNA_CRC(antenna, DATA_REQUEST[8],DATA_REQUEST[9]);
	HAL_UART_Transmit(&huart2, (uint8_t *)DATA_REQUEST, DATA_REQUEST[0]+1, 100);
}

void data_Validation()
{
	memcpy(verification_buffer,data,21);

	if(!verification_flag && data[0] == ANSWER_COMMUNICATION_SIZE)
	{
		verification_Comunication_Buffer();
	}

	if(reciever_flag && communication_validation_flag && verification_buffer[0] == EARRINGS_DATA_SIZE )
	{
		memcpy(earrings[last_earring].N_TAG, &verification_buffer[7], EARRING_SIZE);
		PRINTF("\n-----last: (%d)",last_earring);
		communication_validation_flag = 0;
		last_earring++;
		if(last_earring == EARRINGS_MAX_SIZE)
			last_earring = EARRINGS_MAX_SIZE - 1;

	}


}

static void verification_Comunication_Buffer()
{
	memcpy(verification_buffer,data,19);

	if(verification_buffer[0] == 0x11 && memcmp(verification_buffer,CHAFON_ANSWER,CHAFON_ANSWER[0]+1) == 0)
	{
		communication_validation_flag = 1;
		PRINTF("Successful Communication CHAFON \r\n");
		reciever_flag = 1;
		verification_flag = 1;
	}else
		PRINTF("\n Communication Fail CHAFON \n");
}


