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
		crc16_mcrf4xx(tagList[freePosition)].ta)
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

