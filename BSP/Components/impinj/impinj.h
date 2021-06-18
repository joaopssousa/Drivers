#ifndef __IMPINJ_H
#define __IMPINJ_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"



#define MAX_SIZE_LIST 2000
#define MAX_LEN_TAG 20
#define MAX_ANTENNAS 4
#define USED_ANTENNAS 1
#define INIT_CMDS 3
#define ANTENNA_1 (0x00)
#define ANTENNA_2 (ANTENNA_1 + 1)
#define ANTENNA_3 (ANTENNA_2 + 1)
#define ANTENNA_4 (ANTENNA_3 + 1)

#define QUEUE_OK 0
#define QUEUE_ERROR -1

#define LENGTH_PACKAGE_POSITION 0
#define LENGTH_TAG_POSITION 6
#define TAG_POSITION 7

#define MAX_LENGTH_PACKAGE 50*MAX_LEN_TAG

#define RSSI_POSITION(X) X-2

#define SECT_ADDRESS 10//11
#define BASE_ADDRESS 0x080C0000//0x080E0000

#define BAUDRATE 38400

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct {
	uint8_t rssi;
	uint8_t tag_len;
	uint8_t tag_sn[MAX_LEN_TAG];
} st_tag;

#pragma pack(pop)

void impinj_UART_Init(USART_TypeDef* usart);
void init_Impinj();
void request_Tag();
int8_t get_Tag_to_send(char * buffer);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif
