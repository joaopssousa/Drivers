#include "chafon_4_antennas.h"
#include <stdio.h>
#include <string.h>


UART_HandleTypeDef huart2;


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
//uint8_t verification_buffer[21] = {};

int last_earring = -1;
uint16_t earring_current = 0;
int number_earrings = -1;
uint16_t count_byte = 0;

bool verification_flag = 0;
bool communication_validation_flag = 0;
bool reciever_flag = 0;

Model_earrings earrings[EARRINGS_MAX_SIZE] = {};
Model_earrings earrings_ascii;

static void chafon_init_GPIO(void);
static void verification_Comunication_Buffer();
static uint8_t check_earring_size();

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

//unsigned char nibble_to_ascii(unsigned char c){
//    if((c>=0)&&(c<=9)){
//        return (c+=48);
//    }
//    else{
//        return (c+=55);
//    }
//}
//
//int hex_to_ascii(unsigned char *buffer_out, unsigned char *buffer_in, int tamanho){
//    int j=0;
//    for(int i=0; i<(tamanho);i++)
//    {
//        buffer_out[j] = nibble_to_ascii((buffer_in[i]&0xF0)>>4);
//        buffer_out[j+1] = nibble_to_ascii(buffer_in[i]&0x0F);
//        j+=2;
//    }
//    return 0;
//}

void init_Communication()
{
	HAL_UART_Receive_IT(&huart2, reciver_buffer,1);
	HAL_UART_Transmit(&huart2,(uint8_t *)INIT_COMMUNICATION_CHAFON, INIT_COMMUNICATION_CHAFON[0]+1,100);
}

uint8_t get_Earrings(Model_earrings *earring)
{
	if(check_earring_size() )
	{
		memcpy(earring->N_TAG, &earrings[number_earrings].N_TAG, EARRING_SIZE);

		return 1;
	}
	return 0;
}

//uint8_t get_earrings_ascii(Model_earrings *earring)
//{
//	if(check_earring_size() &&  last_earring > 0)
//	{
//		//hex_to_ascii(&earrings_ascii,&earrings[number_earrings].N_TAG, EARRING_SIZE);
//		memcpy(earring->N_TAG, &earrings_ascii.N_TAG, EARRING_SIZE);
//
//		return 1;
//	}
//	return 0;
//}

static uint8_t check_earring_size()
{
	if(last_earring > -1 && last_earring >= number_earrings){
		return 1;
	}
	else
	{
		PRINTF("\n-----ZEROU------(%d)(%d)\n",number_earrings,last_earring);
		number_earrings = 0;
		last_earring = -1;
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
	uint8_t verification_buffer[EARRINGS_DATA_SIZE];
	memcpy(verification_buffer,data,EARRINGS_DATA_SIZE);
//	for(int i = 0; i < data[0]+1; i++)
//		PRINTF("(%x) ",verification_buffer[i]);
//	PRINTF("\n");
	if(!verification_flag && verification_buffer[0] == ANSWER_COMMUNICATION_SIZE)
	{
		verification_Comunication_Buffer(verification_buffer);
	}

	if(reciever_flag && communication_validation_flag && verification_buffer[0] == EARRINGS_DATA_SIZE )
	{

		memcpy(earrings[++last_earring].N_TAG, &verification_buffer[7], EARRING_SIZE);
		PRINTF("\n-----last: (%d)",last_earring);
		communication_validation_flag = 0;
		if(last_earring == EARRINGS_MAX_SIZE){
			last_earring = EARRINGS_MAX_SIZE - 2;
		}

	}

}

static void verification_Comunication_Buffer(uint8_t verification_buffer[EARRINGS_DATA_SIZE])
{
	//memcpy(verification_buffer,data,19);

	if(verification_buffer[0] == 0x11 && memcmp(verification_buffer,CHAFON_ANSWER,CHAFON_ANSWER[0]+1) == 0)
	{
		communication_validation_flag = 1;
		reciever_flag = 1;
		verification_flag = 1;
		PRINTF("Successful Communication CHAFON \r\n");

	}else
		PRINTF("\n Communication Fail CHAFON \n");
}


