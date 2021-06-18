/*
 * portal_impinj.h
 *
 *  Created on: Feb 8, 2021
 *      Author: LOCEM
 */

#ifndef __PORTAL_IMPINJ_H_
#define __PORTAL_IMPINJ_H_


#define     PKG_SIZE 					16
#define     USER_BANK_3W                15
#define 	TAG_SIZE 					36
#define 	ID_SIZE						12
#define 	MSG_BLE_SIZE				 3
#define 	MSG_RFID_SIZE 				 3
#define 	MSG_USER_8W_SIZE			 9
#define 	STORAGE_SIZE				50
#define		EMPTY_QUEUE         		-1
#define 	TIMEOUT_BETWEEN_RESEND_TAG 100

typedef enum{
	YES = 1,
	NO = 0,
} confirm;

/*
 * 	Estrutura modelo para TAG com atributos de:
 * 	- Numeração da TAG
 * 	- Data e hora para de leitura
 */

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct {

	char N_TAG[TAG_SIZE];
	int time[3];
	int date[3];

} Model_TAG;
#pragma pack(pop)

/*
 *  Vetores de envio e resposta da comunicação
 */
extern char READ_EPC_SINGLE_TAG[MSG_BLE_SIZE];
extern char READ_USER_SINGLE_TAG[MSG_USER_8W_SIZE];
extern char READ_ID_READER[MSG_BLE_SIZE];
extern char READ_MULTIPLE_TAG[MSG_BLE_SIZE];

extern char BLE_TAG_RECEIVED[MSG_BLE_SIZE];
extern char BLE_ESTABLISHED_CONECTION[MSG_BLE_SIZE];
//char BLE_TAG_TO_SEND[TAG_SIZE] = TAG;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;	// Porta serial do módulo RFID

extern unsigned char flag_start;
extern unsigned char flag_tag;
extern unsigned char flag_confirm;
extern unsigned char flag_conection;
extern char message[TAG_SIZE];
extern int message_index;
extern char message_ble[MSG_BLE_SIZE];
extern int ble_index;
extern char TAG[TAG_SIZE];

extern uint8_t rx_byte_uart1[1];
extern uint8_t rx_byte_uart2[1];

extern Model_TAG store_TAG[STORAGE_SIZE];
extern int last_TAG;
extern int in_use_TAG;

void MX_TIM3_Init(void);
void MX_TIM2_Init(void);

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

int message_handler(char *message, int index);

int ble_handler(char *message);

void break_conection();

void clear_buffers();

void Ble_Init_GPIO();

#endif /* INC_HANDLERS_H_ */
