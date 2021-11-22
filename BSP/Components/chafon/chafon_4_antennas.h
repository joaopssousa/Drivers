#ifndef CHAFON_4_ANTENNAS_H_
#define CHAFON_4_ANTENNAS_H_
#include "hw.h"


#define EARRING_SIZE			  12
#define EARRINGS_MAX_SIZE		  500
#define DATA_MAX_SIZE		  	  100

extern bool communication_validation_flag;
extern uint8_t reciver_buffer[1];
extern uint8_t data[DATA_MAX_SIZE];
extern uint16_t count_byte;
extern int number_earrings;
extern bool reciever_flag;
extern uint8_t send_flag;
extern uint8_t count_send_flag;


typedef enum  {
	 ANTENNA1 = 0x80,
	 ANTENNA2 = 0x81,
	 ANTENNA3 = 0x82,
	 ANTENNA4 = 0x83
}ANTENNAS;

extern uint8_t flag_recebe;
extern uint8_t flag_data_comuniation;

#pragma pack(push)  /* push current alignment to stack  */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct {

	char N_TAG[EARRING_SIZE];
	int time[3];
	int date[3];

} Model_earrings;
#pragma pack(pop)

extern Model_earrings earrings[EARRINGS_MAX_SIZE];

void uart_callback();
void data_request_chafon(ANTENNAS antenna);
void INIT_ReaderUART(USART_TypeDef * uartPort,uint32_t baudRate);
uint8_t get_Earrings();
void init_Communication();
void data_Validation();

#endif /*CHAFON_4_ANTENNAS_H_*/
