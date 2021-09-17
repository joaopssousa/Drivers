/*
 * handlers.c
 *
 *  Created on: Feb 8, 2021
 *      Author: LOCEM
 */

#include "main.h"
#include "ble.h"
#include "firmware_version.h"
#include "Commissioning.h"

#include <string.h>


UART_HandleTypeDef huart1;

int count_send = 0;

unsigned char flag_send_timeout = RESET;


#define REQUEST_FIRMWARE_VERSION	0x20
#define REQUEST_DEVICE_TYPE			0x21
#define REQUEST_EXECUTE_UPDATE		0x22


#define ANSWER_FIRMWARE_VERSION		0x30
#define ANSWER_EXECUTE_UPDATE		0x31
#define ANSWER_WRONG_FILE			0x32
#define ANSWER_UPDATE_SUCCESS		0x33
#define ANSWER_DEVICE_TYPE			0x34
#define ANSWER_WRONG_DEVICE_TYPE	0x35

uint8_t message_ble[MSG_BLE_SIZE] = { 0 };		// Mensagem do bluetooth
int ble_index = 0;							// Index de mensagem do bluetooth
int ble_state = 0;							// Flag de estado do bluetooth

uint8_t rx_byte_uart1[1];					// Recepcao da uart1

uint8_t answer_firmware_version_buffer [6] = {0x0A, ANSWER_FIRMWARE_VERSION, MAJOR_FIRMWARE_VERSION, MINOR_FIRMWARE_VERSION, PATCH_FIRMWARE_VERSION, 0x0D};
uint8_t answer_execute_update_buffer [3] =   {0x0A, ANSWER_EXECUTE_UPDATE, 0x0D};
uint8_t answer_wrong_file_buffer [3] =       {0x0A, ANSWER_WRONG_FILE, 0x0D};
uint8_t answer_wrong_device_type [3] =		 {0x0A, ANSWER_WRONG_DEVICE_TYPE, 0x0D};
uint8_t answer_update_success_buffer [3] =   {0x0A, ANSWER_UPDATE_SUCCESS, 0x0D};
uint8_t answer_device_type[4] = 			 {0x0A, ANSWER_DEVICE_TYPE, DEVICE_TYPE, 0x0D};


FRESULT res; 						/* FatFs function common result code */
uint32_t byteswritten, bytesread; 	/* File write/read counts */
uint8_t wtext[TAG_SIZE] = { 0 }; 	/* File write buffer */
uint8_t rtext[_MAX_SS] = { 0 };		/* File read buffer */

flags_connectivity flags_ble;

//static uint8_t dataUART1[32];

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


int ble_handler(uint8_t *message)
{
	switch (message[1]) {
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

void ble_config(void) {
//	/* Set Baudrate	*/
	HAL_UART_Transmit(&huart1, (uint8_t *)BAUD_9600, sizeof(BAUD_9600)-1, 100); //9600

	//HAL_UART_Receive(&huart1, dataUART1, 8, 1000);

	uint8_t setName[26] = {0};
	uint8_t name_Id[4] = {0};
	uint16_t dev_addr = LORAWAN_DEVICE_ADDRESS;//0x0002
	uint8_t hex[4] = {0};
	sprintf((char*)hex, "%x", dev_addr);//2
	uint8_t lenght = strlen((char*)hex);

	switch(lenght) {
	case 1:
		sprintf((char*)name_Id, "000%s", hex);
		break;
	case 2:
		sprintf((char*)name_Id, "00%s", hex);
		break;
	case 3:
		sprintf((char*)name_Id, "0%s", hex);
		break;
	case 4:
		sprintf((char*)name_Id, "%s", hex);
		break;
	default:
		break;
	}

	//HAL_UART_Receive(&huart1, dataUART1, 8, 1000);

	/* Set Ble name	*/
	switch(DEVICE_TYPE) {
	case WEATHERSTATION:
		strcat(strcpy((char*)setName, WEATHERSTATION_NAME), (char*)name_Id);
		HAL_UART_Transmit(&huart1, (uint8_t *)setName, sizeof(setName)-7, 100);
		break;
	case CURRAL:
		strcat(strcpy((char*)setName, CURRAL_NAME), (char*)name_Id);
		HAL_UART_Transmit(&huart1, (uint8_t *)setName, sizeof(setName)-8, 100);
		break;
	case PORTAL:
		strcat(strcpy((char*)setName, PORTAL_NAME), (char*)name_Id);
		HAL_UART_Transmit(&huart1, (uint8_t *)setName, sizeof(setName)-8, 100);
		break;
	case ELETRIFICADOR:
		strcat(strcpy((char*)setName, ELETRIFICADOR_NAME), (char*)name_Id);
		HAL_UART_Transmit(&huart1, (uint8_t *)setName, sizeof(setName)-1, 100);
		break;
	default:
		break;
	}

	//HAL_UART_Transmit(&huart1, dataUART1, 8, 1000);
}


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


