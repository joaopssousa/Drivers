#ifndef CHAFON_4_ANTENNAS_H_
#define CHAFON_4_ANTENNAS_H_
#include "hw.h"

#define EARRING_SIZE 12
#define PACK_SIZE 0x15




#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct {

	char N_TAG[100];
	int time[3];
	int date[3];

} Model_earrings;
#pragma pack(pop)

extern Model_earrings earrings[200];

void INIT_ReaderUART(USART_TypeDef * USART_PORT, uint32_t baudRate);

void Init_Communication();


#endif /*CHAFON_4_ANTENNAS_H_*/
