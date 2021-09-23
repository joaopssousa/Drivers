/*
 * handlers.h
 *
 *  Created on: Feb 8, 2021
 *      Author: LOCEM
 */

#ifndef INC_HANDLERS_H_
#define INC_HANDLERS_H_

#include "hw.h"


#define     PKG_SIZE 						16
#define     USER_BANK_3W                	15
#define 	TAG_SIZE 						36
#define 	ID_SIZE							12
#define 	MSG_BLE_SIZE					10
#define 	MSG_RFID_SIZE 				 	3
#define 	MSG_USER_8W_SIZE			 	9
#define 	STORAGE_SIZE					50
#define		EMPTY_QUEUE         			-1
#define 	TIMEOUT_BETWEEN_RESEND_TAG 		100
#define     MSG_ID_SIZE 					3
#define     MSG_MULTI_TAG_SIZE 				3
#define		MSG_TAG_RCV_SIZE 				3
#define 	MSG_CONNECTION_ESTABLISHED_SIZE 3

#define		BAUD_9600	"AT+BAUD0"
#define		BAUD_19200	"AT+BAUD1"
#define		BAUD_38400	"AT+BAUD2"
#define		BAUD_57600	"AT+BAUD3"
#define		BAUD_115200	"AT+BAUD4"
#define		BAUD_4800	"AT+BAUD5"
#define		BAUD_2400	"AT+BAUD6"
#define		BAUD_1200	"AT+BAUD7"
#define		BAUD_230400	"AT+BAUD8"

#define		WEATHERSTATION_NAME	"AT+NAMEEstacao"
#define		CURRAL_NAME			"AT+NAMECurral"
#define		PORTAL_NAME			"AT+NAMEPortal"
#define		ELETRIFICADOR_NAME	"AT+NAMEEletrificador"

typedef enum{
	YES = 1,
	NO = 0,
} confirm;


/*
 * Estrutura de flags
 */
typedef union
{
 uint8_t     all_flags;      /* Allows us to refer to the flags 'en masse' */
 struct
 {
  uint8_t start : 1,      /* Explanation of foo */
          tag : 1,        /* Explanation of bar */
          confirm : 1,    /* Unused */
          connection : 1, /* Unused */
		  update_mode : 1,     /* Unused */
          enable_handler : 1,     /* Unused */
          spare1 : 1,     /* Unused */
          spare0 : 1;     /* Unused */
 };
} flags_connectivity;

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


extern flags_connectivity flags_ble;
/*
 *  Vetores de envio e resposta da comunicação
 */
extern uint8_t READ_EPC_SINGLE_TAG[MSG_RFID_SIZE];
extern uint8_t READ_USER_SINGLE_TAG[MSG_USER_8W_SIZE];
extern uint8_t READ_ID_READER[MSG_ID_SIZE];
extern uint8_t READ_MULTIPLE_TAG[MSG_MULTI_TAG_SIZE];

extern uint8_t BLE_TAG_RECEIVED[MSG_TAG_RCV_SIZE];
extern uint8_t BLE_ESTABLISHED_CONECTION[MSG_CONNECTION_ESTABLISHED_SIZE];
//char BLE_TAG_TO_SEND[TAG_SIZE] = TAG;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;	// Porta serial do módulo RFID

extern int count_send;

//extern unsigned char flag_start;
//extern unsigned char flag_tag;
//extern unsigned char flag_confirm;
//extern unsigned char flag_connection;
extern unsigned char flag_send_timeout;

extern uint8_t message[TAG_SIZE];
extern int message_index;
extern uint8_t message_ble[MSG_BLE_SIZE];
extern int ble_index;
extern uint8_t TAG[TAG_SIZE];

extern uint8_t rx_byte_uart1[1];
extern uint8_t rx_byte_uart2[1];

extern Model_TAG store_TAG[STORAGE_SIZE];
extern int last_TAG;
extern int in_use_TAG;

extern Model_TAG delayed_store_TAG[STORAGE_SIZE];
extern int delayed_last_TAG;
extern int delayed_in_use_TAG;
extern int delayed_store_flag;

extern FRESULT res;
extern uint32_t byteswritten, bytesread;
extern uint8_t wtext[TAG_SIZE];
extern uint8_t rtext[_MAX_SS];
extern uint8_t answer_update_success_buffer [3];

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;


void MX_TIM3_Init(void);
void MX_TIM2_Init(void);

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

int message_handler(uint8_t *message, int index);

int ble_handler(uint8_t *message);

void ble_config(void);

void break_conection();

void clear_buffers();

void Ble_Init_GPIO();

#endif /* INC_HANDLERS_H_ */
