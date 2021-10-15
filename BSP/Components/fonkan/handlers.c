/*
 * handlers.c
 *
 *  Created on: Feb 8, 2021
 *      Author: LOCEM
 */

#include "main.h"
#include "handlers.h"
#include "firmware_version.h"

#include <string.h>


UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

int count_send = 0;

unsigned char flag_send_timeout = RESET;


#define REQUEST_CONNECTION 			0x60
#define TAG_CONFIRMATION 			0x61
#define ESTABLISHED_CONNECTION		0x62
#define REQUEST_ID 					0x63
#define END_CONNECTION 				0x6F
#define WRITE_EARRING_NUMBER		0X65

#define REQUEST_FIRMWARE_VERSION	0x20
#define REQUEST_DEVICE_TYPE			0x21
#define REQUEST_EXECUTE_UPDATE		0x22
#define REQUEST_WRITE_EARRING		0x23


#define ANSWER_FIRMWARE_VERSION		0x30
#define ANSWER_EXECUTE_UPDATE		0x31
#define ANSWER_WRONG_FILE			0x32
#define ANSWER_UPDATE_SUCCESS		0x33
#define ANSWER_DEVICE_TYPE			0x34
#define ANSWER_WRONG_DEVICE_TYPE	0x35

#define ANSWER_END_CONNECTION		0x5F

uint8_t message[500/*TAG_SIZE*/] = { 0 };				// Vetor de retorno do módulo RFID (Versão, armazenamento da TAG)
int message_index = 0;						// Index da mensagem
uint8_t message_ble[MSG_BLE_SIZE] = { 0 };		// Mensagem do bluetooth
int ble_index = 0;							// Index de mensagem do bluetooth
int ble_state = 0;							// Flag de estado do bluetooth

uint8_t TAG[500/*TAG_SIZE*/] = { 0 };					// Vetor de resposta do módulo RFID (Resposta, versão ou TAG)

Model_TAG store_TAG[STORAGE_SIZE] = { 0 };	// Armazenamento de TAGs em caso de sobreposição de leitura, ja iniciada limpa.
int last_TAG = -1;							// Variável que mostra a ultima posição ocupada na lista
int in_use_TAG = 0;							// Variável usada para envio sequencial da lista

// Estruturas e indices para armazenamento em cartão SD em caso de falta de conexão com o gateway
Model_TAG delayed_store_TAG[STORAGE_SIZE] = { 0 };		// Armazenamento de TAGs em caso de sobreposição de leitura, ja iniciada limpa.
int delayed_last_TAG = -1;								// Variável que mostra a ultima posição ocupada na lista
int delayed_in_use_TAG = 0;								// Variável usada para envio sequencial da lista
int delayed_store_flag = 0;

uint8_t rx_byte_uart1[1];					// Recepcao da uart1
//uint8_t rx_byte_uart2[3];					// Recepcao da uart2

uint8_t bytes_read_rfid =0;

uint8_t READ_EPC_SINGLE_TAG[MSG_RFID_SIZE] = {0xA, 0x51, 0x0D};
uint8_t READ_USER_SINGLE_TAG[MSG_USER_8W_SIZE] = {0x0A, 0x52, 0x33, 0x2C, 0x30, 0x2C, 0x30, 0x33, 0x0D};
uint8_t READ_ID_READER[MSG_ID_SIZE] = {0x0A, 0x53, 0x0D};
uint8_t READ_MULTIPLE_TAG[MSG_MULTI_TAG_SIZE] = {0x0A, 0x55, 0x0D};
uint8_t MSG_WRITE_EARRING [MSG_WRITE_EARRING_SIZE] = {0x0A, 0x57, 0x31, 0x2C, 0x32, 0x2C ,0x36, 0x2C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x0D};

uint8_t answer_end_connection[3] = {0x0A, ANSWER_END_CONNECTION, 0x0D};

uint8_t answer_firmware_version_buffer [6] = {0x0A, ANSWER_FIRMWARE_VERSION, MAJOR_FIRMWARE_VERSION, MINOR_FIRMWARE_VERSION, PATCH_FIRMWARE_VERSION, 0x0D};
uint8_t answer_execute_update_buffer [3] =   {0x0A, ANSWER_EXECUTE_UPDATE, 0x0D};
uint8_t answer_wrong_file_buffer [3] =       {0x0A, ANSWER_WRONG_FILE, 0x0D};
uint8_t answer_wrong_device_type [3] =		 {0x0A, ANSWER_WRONG_DEVICE_TYPE, 0x0D};
uint8_t answer_update_success_buffer [3] =   {0x0A, ANSWER_UPDATE_SUCCESS, 0x0D};
uint8_t answer_device_type[4] = 			 {0x0A, ANSWER_DEVICE_TYPE, DEVICE_TYPE, 0x0D};

uint8_t BLE_TAG_RECEIVED[MSG_TAG_RCV_SIZE] = {0x0A, TAG_CONFIRMATION, 0x0D};
uint8_t BLE_ESTABLISHED_CONNECTION[MSG_CONNECTION_ESTABLISHED_SIZE] = {0x0A, ESTABLISHED_CONNECTION, 0x0D};
//char BLE_TAG_TO_SEND[TAG_SIZE] = TAG;


FRESULT res; 						/* FatFs function common result code */
uint32_t byteswritten, bytesread; 	/* File write/read counts */
uint8_t wtext[TAG_SIZE] = { 0 }; 	/* File write buffer */
uint8_t rtext[_MAX_SS] = { 0 };		/* File read buffer */

flags_connectivity flags_ble;

bool assert_device_type (uint8_t device_type){
	if (DEVICE_TYPE == device_type)
		return true;
	return false;
}

bool assert_version(uint8_t major_version, uint8_t minor_version, uint8_t patch_version){
	if(MAJOR_FIRMWARE_VERSION == major_version){ // Criar define para posições da versão
		if (MINOR_FIRMWARE_VERSION == minor_version){
			if (PATCH_FIRMWARE_VERSION < patch_version){
				return true;
			}
		}
		else if(MINOR_FIRMWARE_VERSION < minor_version){
			return true;
		}
	}
	else if (MAJOR_FIRMWARE_VERSION < major_version){
		return true;
	}
	return false;
}

/*
 * Função de tratamento e interpretação da mensagem vinda do módulo RFID
 */
#if (DEVICE_TYPE == CURRAL)
int message_handler(uint8_t *message, int pkg_length)
{
	uint8_t total_de_brincos = (pkg_length - 4) / TAG_SIZE;
	memcpy(TAG, message, pkg_length);					// Copia a TAG lida para o vetor

	// TODO Melhorar a rotina de comparação entre TAG recebida e ja armazenadas para todas e não apenas a ultima lida
//	if(memcmp(TAG, message, TAG_SIZE) != 0)			// Como a comparação é feita aqui, deve-se limpar o buffer 'message' depois
//	{
//		memset(message, 0, TAG_SIZE);				// Limpa buffer de mensagem para nova recepção
//		return 2;									// Código de retorno quando a TAG lida é igual a lida anteriormente
//	}


	//memset(message, 0, TAG_SIZE);					// Limpa buffer de mensagem para nova recepção

	if(pkg_length >= TAG_SIZE + 4/*- 1*/)
	{
		PRINTF("Tamanho = %d \r\n", pkg_length);
		/*
		 * 	Verifica que recebido foi a leitura de uma TAG válida, então:
		 *  - Copia a TAG para o armazenamento para envio;
		 *  - Incrementa a referência do último armazenamento.
		 */

		if(last_TAG>0){
			if(memcmp(&store_TAG[last_TAG-1], TAG, TAG_SIZE) != 0)	// Como a comparação é feita aqui, deve-se limpar o buffer 'message' depois
			{
				memset(message, 0, TAG_SIZE);				// Limpa buffer de mensagem para nova recepção
				return 2;									// Código de retorno quando a TAG lida é igual a lida anteriormente
			}
		}

		if(last_TAG == STORAGE_SIZE - 1){			// Se for vista a ultima TAG, então começa a sobreescrever
			clear_buffers();
		}

		for (int i=0; i<total_de_brincos;i++){
			memcpy(store_TAG[++last_TAG].N_TAG, &TAG[(i*TAG_SIZE)], TAG_SIZE-1);
		}

		return 1;							// Confirmação que foi lida e armazenada uma TAG
	}
	else
	{
		if (pkg_length == MSG_RFID_SIZE+1) // Em caso de retorno padrão, sem leitura de TAG. Passa 0xFF ao vetor TAG
			memset(TAG, 255, TAG_SIZE);

		return 0;                            // Sinaliza retorno padrão ou de ID
	}

	return 3; 								// Alguma coisa deu errado
}
#endif

int ble_handler(uint8_t *message)
{
	uint8_t sizeofEarring;
	uint8_t initialPosition;
	switch (message[1]) {
#if (DEVICE_TYPE == CURRAL)
		case REQUEST_CONNECTION:
			/*
			 * 	Pedido de Conexão
			 */
			if(flags_ble.connection == SET)
			{
				PRINTF("Recebeu Start\r\n");
				// Se a flag de conexão estiver ativa devido a verificação pelo timer, confirme.
				HAL_UART_Transmit(&huart1, (uint8_t *)BLE_ESTABLISHED_CONNECTION, MSG_CONNECTION_ESTABLISHED_SIZE, 100);
			  	HAL_TIM_Base_Start_IT(&htim2);			// Inicia o timer que envia as requisições para o módulo RFID
			  	flags_ble.start = SET;
			}
			else
			{
				// TODO Testar se a quebra de conexão continua travando e como ajeitar
				HAL_TIM_Base_Stop_IT(&htim2);			// Para momentâneamente as requisições e leituras de TAG para requisição do ID do RFID
				flags_ble.start = RESET;						// Reseta a flag de inicio da comunicação
				break_connection();						// Quebra conexão pois houve algum erro no módulo BLE
			}

			break;
		case TAG_CONFIRMATION:
			/*
			 * 	Confirmação de TAG recebida, destravar para enviar nova TAG
			 */
			flags_ble.confirm = SET;
			PRINTF("====>   confirm = SET \r\n");

				// TODO Criar trava de sistema
			break;
		case REQUEST_ID:
			/*
			 *  Pedido de ID do leitor RFID
			 */
			break;

		case END_CONNECTION:
			// Pedido de encerramento de conexão

			HAL_UART_Transmit(&huart1, (uint8_t *) answer_end_connection, 3, 100);
			if (flags_ble.start == SET)
			{
				HAL_TIM_Base_Stop_IT(&htim2);			// Para momentâneamente as requisições e leituras de TAG
				flags_ble.start = RESET;				// Reseta a flag de inicio da comunicação
				//clear_buffers();
				break_connection();						// Função de quebra de conexão
			}

			break;

		case WRITE_EARRING_NUMBER:
			for (sizeofEarring=2; sizeofEarring<TAG_SIZE; sizeofEarring++)
			{
				if(message[sizeofEarring] == 0x0D)
				{
					break;
				}
			}
			sizeofEarring -= 2;
			initialPosition = (MSG_WRITE_EARRING_SIZE-1) - sizeofEarring;
			memcpy(&MSG_WRITE_EARRING[initialPosition], &message[2], sizeofEarring );
			HAL_UART_Transmit(&huart1, (uint8_t *) MSG_WRITE_EARRING, MSG_WRITE_EARRING_SIZE, 100);
			break;

#endif
/****************************************** Common to all devices - Commands to Remote Update **************************************/

		case REQUEST_DEVICE_TYPE:
			HAL_UART_Transmit(&huart1, (uint8_t *) &answer_device_type,sizeof(answer_device_type),100);
			break;

		case REQUEST_FIRMWARE_VERSION:
			HAL_UART_Transmit(&huart1, (uint8_t *) &answer_firmware_version_buffer, sizeof(answer_firmware_version_buffer), 100);
			break;

		case REQUEST_EXECUTE_UPDATE:
			if(assert_version(message[2], message[3], message[4])){
				if(assert_device_type(message[5]))
				{
					flags_ble.update_mode = SET;
					HAL_UART_Transmit(&huart1, (uint8_t *) &answer_execute_update_buffer, sizeof(answer_execute_update_buffer), 100);

				}
				else
				{
					HAL_UART_Transmit(&huart1, (uint8_t *) &answer_wrong_device_type, sizeof(answer_wrong_device_type),100);
				}
			}
			else{
				HAL_UART_Transmit(&huart1, (uint8_t *) &answer_wrong_file_buffer, sizeof(answer_wrong_file_buffer), 100);
			}
			break;

		default:
			// Sem requisição válida
			break;
/*********************************************************************************************************************************/
	}

	return 0;
}


//char buffer[50];
//uint8_t timer_count = 0, buffer_index = 0;

//uint8_t string_compare(char array1[], char array2[], uint16_t length)
//{
//	 uint8_t comVAR=0, i;
//	 for(i=0;i<length;i++)
//	   	{
//	   		  if(array1[i]==array2[i])
//	   	  		  comVAR++;
//	   	  	  else comVAR=0;
//	   	}
//	 if (comVAR==length)
//		 	return 1;
//	 else 	return 0;
//}

void break_connection(){

	HAL_GPIO_WritePin(BLE_BRK_GPIO_Port, BLE_BRK_Pin, RESET);		// Aciona o pino que interrompe a possível conexão errada.

	uint32_t aux = 0;
	while(aux<1*100000)
		aux++;

	HAL_GPIO_WritePin(BLE_BRK_GPIO_Port, BLE_BRK_Pin, SET);			// Se a conexão for quebrada, restaura a forma original.

}

#if (DEVICE_TYPE == CURRAL)
void clear_buffers(){
	last_TAG = EMPTY_QUEUE;
	memset(&store_TAG, 0, sizeof(store_TAG));
}
#endif



/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
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


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void Ble_Init_GPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLE_BRK_GPIO_Port, BLE_BRK_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : BLE_BRK_Pin */
  GPIO_InitStruct.Pin = BLE_BRK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BLE_BRK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_STATE_Pin */
  GPIO_InitStruct.Pin = BLE_STATE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_STATE_GPIO_Port, &GPIO_InitStruct);

}


