/*
 * handlers.c
 *
 *  Created on: Feb 8, 2021
 *      Author: LOCEM
 */

#include "main.h"
#include "portal_impinj.h"
#include <string.h>


unsigned char flag_start = RESET;			// Flag de inicio do processo
unsigned char flag_tag = RESET;				// Flag de Leitura do módulo RFID (Versão ou TAG)
unsigned char flag_confirm = RESET;			// Flag de confirmação pelo bluetooth
unsigned char flag_conection = RESET;		// Flag de confirmação de conexão com bluetooth

char message[TAG_SIZE] = { 0 };				// Vetor de retorno do módulo RFID (Versão, armazenamento da TAG)
int message_index = 0;						// Index da mensagem
char message_ble[MSG_BLE_SIZE] = { 0 };		// Mensagem do bluetooth
int ble_index = 0;							// Index de mensagem do bluetooth
int ble_state = 0;							// Flag de estado do bluetooth

char TAG[TAG_SIZE] = { 0 };					// Vetor de resposta do módulo RFID (Resposta, versão ou TAG)

Model_TAG store_TAG[STORAGE_SIZE] = { 0 };	// Armazenamento de TAGs em caso de sobreposição de leitura, ja iniciada limpa.
int last_TAG = -1;							// Variável que mostra a ultima posição ocupada na lista
int in_use_TAG = 0;							// Variável usada para envio sequencial da lista
uint8_t rx_byte_uart1[1];					// Recepcao da uart1
uint8_t rx_byte_uart2[1];					// Recepcao da uart2

char READ_EPC_SINGLE_TAG[MSG_RFID_SIZE] = {0xA, 0x51, 0x0D};
char READ_USER_SINGLE_TAG[MSG_USER_8W_SIZE] = {0x0A, 0x52, 0x33, 0x2C, 0x30, 0x2C, 0x30, 0x33, 0x0D};
char READ_ID_READER[MSG_BLE_SIZE] = {0x0A, 0x53, 0x0D};
char READ_MULTIPLE_TAG[MSG_BLE_SIZE] = {0x0A, 0x55, 0x0D};

char BLE_TAG_RECEIVED[MSG_BLE_SIZE] = {0x0A, 0x61, 0x0D};
char BLE_ESTABLISHED_CONECTION[MSG_BLE_SIZE] = {0x0A, 0x62, 0x0D};
//char BLE_TAG_TO_SEND[TAG_SIZE] = TAG;


/*
 * Função de tratamento e interpretação da mensagem vinda do módulo RFID
 */
int message_handler(char *message, int index)
{
	memcpy(TAG, message, index+1);				// Copia a TAG lida para o vetor (sem o ultimo 0x0A)
	message_index = 0;								// Reseta o índice no vetor de mensagem

	// TODO Melhorar a rotina de comparação entre TAG recebida e ja armazenadas para todas e não apenas a ultima lida
//	if(memcmp(TAG, message, TAG_SIZE) != 0)			// Como a comparação é feita aqui, deve-se limpar o buffer 'message' depois
//	{
//		memset(message, 0, TAG_SIZE);				// Limpa buffer de mensagem para nova recepção
//		return 2;									// Código de retorno quando a TAG lida é igual a lida anteriormente
//	}



	// TODO Acionar aqui ou incrementar rotina para guardar as TAGs lidas em uma matriz
	// TODO Além da matriz, derivar um índice de ultima posição gravada para coordenar o envio
	// TODO Criar, aqui ou fora, rotina para teste de envio e se não poder haver envio que sejam armazenadas as TAGs
	//memset(message, 0, TAG_SIZE);					// Limpa buffer de mensagem para nova recepção
	if(index == TAG_SIZE -1)
	{
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

		if(last_TAG == STORAGE_SIZE - 1)	// Se for vista a ultima TAG, então começa a sobreescrever
			last_TAG = -1;					// Se o último espaço de armazenamento estiver ocupado, reinicia o armazenamento da base.

		memcpy(store_TAG[++last_TAG].N_TAG, TAG, TAG_SIZE-1);

		return 1;							// Confirmação que foi lida e armazenada uma TAG
	}
	else if(index < TAG_SIZE - 1)
	{
		if (index == MSG_RFID_SIZE) // Em caso de retorno padrão, sem leitura de TAG. Passa 0xFF ao vetor TAG
			memset(TAG, 255, TAG_SIZE);
		if (index == ID_SIZE) // Em caso de retorno de ID do leitor RFID. Mantém a mensagem no vetor TAG
		{
		}
		if (index == USER_BANK_3W) {
			if (last_TAG == STORAGE_SIZE - 1) // Se for vista a ultima TAG, então começa a sobreescrever
				last_TAG = -1; // Se o último espaço de armazenamento estiver ocupado, reinicia o armazenamento da base.

			memcpy(store_TAG[++last_TAG].N_TAG, TAG, index + 1);
		}

		return 0;                            // Sinaliza retorno padrão ou de ID
	}

	return 3; 								// Alguma coisa deu errado
}

int ble_handler(char *message)
{
	switch (message[1]) {
		case 0x60:
			/*
			 * 	Pedido de Conexão
			 */
			if(flag_conection == SET)
			{
				// Se a flag de conexão estiver ativa devido a verificação pelo timer, confirme.
				HAL_UART_Transmit(&huart1, (uint8_t *)BLE_ESTABLISHED_CONECTION, MSG_BLE_SIZE, 100);
			  	HAL_TIM_Base_Start_IT(&htim2);			// Inicia o timer que envia as requisições para o módulo RFID
			  	flag_start = SET;
			}
			else
			{
				// TODO Testar se a quebra de conexão continua travando e como ajeitar
				HAL_TIM_Base_Stop_IT(&htim2);			// Para momentâneamente as requisições e leituras de TAG para requisição do ID do RFID
				flag_start = RESET;						// Reseta a flag de inicio da comunicação
				break_conection();						// Quebra conexão pois houve algum erro no módulo BLE
			}

			break;
		case 0x61:
			/*
			 * 	Confirmação de TAG recebida, destravar para enviar nova TAG
			 */
			flag_confirm = SET;

				// TODO Criar trava de sistema
			break;
		case 0x63:
			/*
			 *  Pedido da ID do leitor RFID
			 */
			HAL_TIM_Base_Stop_IT(&htim2);			// Para momentâneamente as requisições e leituras de TAG para requisição do ID do RFID
			HAL_UART_Transmit(&huart2, (uint8_t *)READ_ID_READER, MSG_RFID_SIZE, 100);		// Enviou a requisição ao módulo RFID

			while(flag_tag != SET){
				;
			}
			HAL_UART_Transmit(&huart1, (uint8_t *) TAG, ID_SIZE, 100);
			break;
		case 0x6F:
			// Pedido de encerramento de conexão
			HAL_TIM_Base_Stop_IT(&htim2);			// Para momentâneamente as requisições e leituras de TAG para requisição do ID do RFID
		  	flag_start = RESET;						// Reseta a flag de inicio da comunicação
		  	last_TAG = EMPTY_QUEUE;// Restaura o identificador ao ponto padrão sem armazenamento
		  	clear_buffers();
			break_conection();						// Função de quebra de conexão

			break;
		default:
			// Sem requisição válida
			break;
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

void break_conection(){

	HAL_GPIO_WritePin(BLE_BRK_GPIO_Port, BLE_BRK_Pin, RESET);		// Aciona o pino que interrompe a possível conexão errada.

	uint32_t aux = 0;
	while(aux<1*100000)
		aux++;

	HAL_GPIO_WritePin(BLE_BRK_GPIO_Port, BLE_BRK_Pin, SET);			// Se a conexão for quebrada, restaura a forma original.

}

void clear_buffers(){
	int i;			// Variavel auxiliar para apagar o armazenamento
	for (i = 0; i < STORAGE_SIZE; i++) {
		memset(store_TAG[i].N_TAG, 0, TAG_SIZE);
	}
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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





#include <stdio.h>
#include <string.h>
#include "util_console.h"
#include "impinj.h"

uint8_t rx_byte_impinj[1];
extern uint8_t flag_connected;
uint8_t impinj_buffer_rx[MAX_LENGTH_PACKAGE];
uint8_t impinj_buffer[MAX_LENGTH_PACKAGE];
uint8_t rx_counter, flag_new_pack, flag_receiving, flag_save_to_flash,
		tag_to_send = 0;
//TIM_HandleTypeDef htim3, htim2;

uint16_t freePosition = 0;

static uint32_t freeAddrPosition = 0;

UART_HandleTypeDef huart_impinj;

st_tag tagList[MAX_SIZE_LIST];

#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408
//#define DEBUG_IMP

//TODO Implementar CRC
uint16_t crc16_mcrf4xx(uint8_t *data, size_t len)
{
	crc = PRESET_VALUE;
    if (!data || len < 0)
        return crc;

    while (len--) {
        crc ^= *data++;
        for (int i=0; i<8; i++) {
            if (crc & 1)  crc = (crc >> 1) ^ POLYNOMIAL;
            else          crc = (crc >> 1);
        }
    }
    return crc;
}

static void send_to_uart(unsigned char *buffer, uint8_t size);
//static void send_commands(char* commands, uint8_t size);

static unsigned char initImpinj[INIT_CMDS][6] = {
		{ 0x04, 0xFF, 0x21, 0x19, 0x95 },
		{ 0x05, 0xFF, 0x76, 0x00, 0x91, 0x0F },
		{ 0x05, 0xFF, 0x2F, 0x1B, 0x2C,	0xA5 } };

static unsigned char readAntenna[MAX_ANTENNAS][14] = {
		{ 0x0D, 0xFF, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80	| ANTENNA_1, 0x14, 0x0D, 0x93 },
		{ 0x0D, 0xFF, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80	| ANTENNA_2, 0x14, 0xD5, 0x8A },
		{ 0x0D, 0xFF, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80	| ANTENNA_3, 0x14, 0x91, 0x6F },
		{ 0x0D, 0xFF, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80	| ANTENNA_4, 0x14, 0x65, 0xB9 }};

void impinj_UART_Init(USART_TypeDef *usart) {
	huart_impinj.Instance = usart;
	huart_impinj.Init.BaudRate = BAUDRATE;
	huart_impinj.Init.WordLength = UART_WORDLENGTH_8B;
	huart_impinj.Init.StopBits = UART_STOPBITS_1;
	huart_impinj.Init.Parity = UART_PARITY_NONE;
	huart_impinj.Init.Mode = UART_MODE_TX_RX;
	huart_impinj.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart_impinj.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart_impinj) != HAL_OK) {
		while (1)
			;
	}
}

void init_Impinj() {
	for (int i = 0; i < INIT_CMDS; i++) {
		send_to_uart(&initImpinj[i], initImpinj[i][LENGTH_PACKAGE_POSITION] + 1);
	}
}

void request_Tag() {
	for (int i = 0; i < USED_ANTENNAS; i++) {
		send_to_uart(&readAntenna[i], readAntenna[i][LENGTH_PACKAGE_POSITION] + 1);
	}
}

static void send_to_uart(unsigned char *buffer, uint8_t size) {
#ifdef DEBUG_IMP
	if (impinj_buffer[LENGTH_PACKAGE_POSITION] > 0) {
		for (int i = 0; i < impinj_buffer[LENGTH_PACKAGE_POSITION] + 1; i++)
			PRINTF("buffer %02X ", impinj_buffer[i]);
	}
#endif
	HAL_UART_Transmit(&huart_impinj, buffer, size, 1000);
}

void handle_impinj() {
#ifdef DEBUG_IMP
	if (impinj_buffer[LENGTH_PACKAGE_POSITION] > 0) {
		for (int i = 0; i < impinj_buffer[LENGTH_PACKAGE_POSITION] + 1; i++)
			PRINTF("buffer %02X ", impinj_buffer[i]);
	}
#endif
	// tamanho do pacote 7 - Tag não detectada
	if (impinj_buffer[LENGTH_PACKAGE_POSITION] == 0x07) {

	}
	// tamanho do pacote > 12 - Tag detectada
	else if (impinj_buffer[LENGTH_PACKAGE_POSITION] >= 0x12) {
		tagList[freePosition].tag_len = impinj_buffer[LENGTH_TAG_POSITION];
		tagList[freePosition].rssi = impinj_buffer[RSSI_POSITION(
				impinj_buffer[LENGTH_PACKAGE_POSITION])];
		memcpy(&tagList[freePosition].tag_sn[0], &impinj_buffer[TAG_POSITION],
				tagList[freePosition].tag_len);
		//crc16_mcrf4xx(tagList[freePosition)].ta)
		if (freePosition < MAX_SIZE_LIST)
			freePosition++;
		else
			freePosition = 0;

	}
	// tamanho do pacote 11 - Modulo impinj reconhecido
	else if (impinj_buffer[0] == 0x11) {
		//   flag_init = 1;
		// } else {
		//flag_init = 0;
	}
	flag_receiving = 0;
}

void save_to_flash() {
//     MY_FLASH_SetSectorAddrs(SECT_ADDRESS, BASE_ADDRESS+freePosition);
//     int i;
//     pData = (uint32_t* )data;
//     HAL_FLASH_Unlock();
//     for(i=0;i<sizeof(buffer);i+=4){
//         HAL_StatusTypeDef retVal = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, save_addr, *pData);
//         HAL_Delay(10);
//         if(retVal == HAL_OK){
//             pData++;
//             save_addr+=4;
//             HAL_Delay(10);
//         }
//     }
//     HAL_FLASH_Lock();

}

void USART2_IRQHandler(void) {

	HAL_UART_IRQHandler(&huart_impinj);

	impinj_buffer_rx[rx_counter] = rx_byte_impinj[0];
	flag_receiving = 1;
	if (rx_counter++ >= impinj_buffer_rx[LENGTH_PACKAGE_POSITION]) {
		//if(impinj_buffer_rx[0]>0x11){
		//	if(rx_counter>impinj_buffer_rx[0]+8)
		//	{
		handle_impinj();
		rx_counter = 0;
		flag_new_pack = 1;
		//	}
		//}
		//else{
		//	rx_counter = 0;
		//	flag_new_pack=1;
		//}
	}
	HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
	HAL_UART_Abort_IT(&huart_impinj);
	HAL_UART_Receive_IT(&huart_impinj, rx_byte_impinj, 1);

	/* USER CODE END USART2_IRQn 1 */
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		//if (flag_connected)
		save_to_flash();

	} else if (htim->Instance == TIM3) {
		request_Tag();
		//flag_read_tags = true;
	}
}

int8_t get_Tag_to_send(char *buffer) {
	if (freePosition > 0) {
		if (tag_to_send < freePosition) {
			memcpy(buffer, &tagList[tag_to_send], sizeof(st_tag));
			tag_to_send++;
			return QUEUE_OK;
		}
	}
	return QUEUE_ERROR;
}




