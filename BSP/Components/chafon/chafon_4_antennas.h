#ifndef CHAFON_4_ANTENNAS_H_
#define CHAFON_4_ANTENNAS_H_
#include "hw.h"

extern bool communication_validation_flag;
extern uint8_t reciver_buffer[1];
extern uint8_t data[500];
extern uint16_t count_byte;
extern int number_earrings;
extern bool reciever_flag;

typedef enum  {
	 ANTENNA1 = 0x80,
	 ANTENNA2 = 0x81,
	 ANTENNA3 = 0x82,
	 ANTENNA4 = 0x83
}ANTENNAS;



#pragma pack(push)  /* push current alignment to stack  */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct {

	char N_TAG[12];
	int time[3];
	int date[3];

} Model_earrings;
#pragma pack(pop)

extern Model_earrings earrings[500];

void uart_callback();
void data_request_chafon(ANTENNAS antenna);
void INIT_ReaderUART(USART_TypeDef * uartPort,uint32_t baudRate);
uint8_t get_Earrings();
void init_Communication();
void data_Validation();

#endif /*CHAFON_4_ANTENNAS_H_*/
