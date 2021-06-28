/*
 * station.h
 *
 *  Created on: Nov 5, 2020
 *      Author: LOCEM
 */

#ifndef SRC_STATION_H_
#define SRC_STATION_H_


#include "bmp280.h"

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{

	uint16_t temperatura;
	uint32_t pressao;
	uint16_t humidade;
	uint16_t pluviometria;
	uint8_t direcao_vento;
	uint16_t velocidade_vento;

} Estation_Parameters;

#pragma pack(pop)

typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize;
  /*Port on which the LoRa App is data is sent/ received*/
  uint8_t Port;

} Sensor_AppData;

extern uint8_t flag_pluv;
enum Direcoes {
	Norte,
	Nordeste,
	Leste,
	Sudeste,
	Sul,
	Sudoeste,
	Oeste,
	Noroeste,
};
typedef enum Direcoes Direcoes;

enum sens {
	temperatura,
	pressao,
	humidade,
	pluviometria,
	direcao_vento,
	velocidade_vento,
};
typedef enum sens sens;

// Inicialização dos parâmetros da estação para posterior atribuição.
Estation_Parameters Parameters;

/* Functions */
//TODO: Alterar nome da função sensores
bool Call_BME280(Estation_Parameters *Parameters);

void Call_Pluviometer(Estation_Parameters *Parameters);

void Call_Anemometro(Estation_Parameters *Parameters);

void Call_Anemometro2(Estation_Parameters *Parameters);

bool Call_Biruta(Estation_Parameters *Parameters);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

void corrige_data(int dia, int mes, int ano);

void corrige_hora(int hora, int minutos, int segundos);

void Sensores(Estation_Parameters *Parameters);

void muda_buffer(Sensor_AppData *AppData, char Buffer_to_send[]);

void mede_mm_chuva();

void mede_velocidade_vento();

void init_station();


#endif /* SRC_STATION_H_ */
