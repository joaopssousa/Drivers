#ifndef CHAFON_4_ANTENNAS_H_
#define CHAFON_4_ANTENNAS_H_
#include "hw.h"

#define EARRING_SIZE 12
#define PACK_SIZE 0x15


extern bool communicationValidationFlag;
extern bool cleanBuffFlag;
extern uint8_t reciverBuffer[1];
extern uint8_t data[500];
extern uint16_t contbyte;
extern bool recieverFlag;

typedef enum  {
	 ANTENNA1 = 0x80,
	 ANTENNA2,
	 ANTENNA3,
	 ANTENNA4
}ANTENNAS;

#pragma pack(push)  /* push current alignment to stack */
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
void getEarrings();

#endif /*CHAFON_4_ANTENNAS_H_*/
