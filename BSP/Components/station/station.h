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

/*
 * Estrutura de flags
 */
typedef union
{
 uint8_t     all_flags;      /* Allows us to refer to the flags 'en masse' */
 struct
 {
  uint8_t pluviometer : 1,      		/* 1: verificacao do horario para zerar dados do pluviometro */
          bmp_failed : 1,       		/* 1: Indica falha nos sensores do BMP280 (Temperatura, Pressao, Humidade) */
		  active_irradiator : 1,		/* 1: Irradiador presente */
          receive_measure_irrad : 1, 	/* 1: Recebeu uma medida válida do Medidor de irradiacao */
		  read_sensors : 1,     		/* 1: Realiza leitura dos sensores */
          alarm_b : 1,     				/* 1: Alarm B foi ativado */
          spare4 : 1,     		/* Unused */
          spare5 : 1;     		/* Unused */
 };
} flags_station;

extern flags_station flagsStation;
extern uint16_t pluviometer_count;
extern uint32_t aux_count_velo;
extern uint32_t count_velo;
extern Estation_Parameters Parameters;

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



/* Functions */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

void read_sensors(Estation_Parameters *Parameters);

void muda_buffer(Sensor_AppData *AppData, char Buffer_to_send[]);

void mede_mm_chuva();

void mede_velocidade_vento();

void init_station();


#endif /* SRC_STATION_H_ */
