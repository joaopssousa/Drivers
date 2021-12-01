/*
 * station.h
 *
 *  Created on: Nov 5, 2020
 *      Author: LOCEM
 */

#ifndef SRC_STATION_H_
#define SRC_STATION_H_


#include "bmp280.h"

#define BME_CONTROL_PIN GPIO_PIN_7
#define BME_CONTROL_PORT GPIOD

#define BIRUTA_Pin GPIO_PIN_7
#define BIRUTA_GPIO_Port GPIOA

#define ANEMOMETRO_Pin GPIO_PIN_14
#define ANEMOMETRO_GPIO_Port GPIOE
#define ANEMOMETRO_EXTI_IRQn EXTI0_IRQn

#define PLUVIOMETRO_Pin GPIO_PIN_15
#define PLUVIOMETRO_GPIO_Port GPIOE
#define PLUVIOMETRO_EXTI_IRQn EXTI2_IRQn

#define SCL_BME280_Pin GPIO_PIN_6
#define SCL_BME280_GPIO_Port GPIOB

#define SDA_BME280_Pin GPIO_PIN_7
#define SDA_BME280_GPIO_Port GPIOB


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
          spare3 : 1,     		/* 1: Habilita BME */
          spare4 : 1,     		/* 1: Habilita contador de tempo */
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

void enable_bme(void);

void reset_bme(void);

void turn_off_bme(void);


#endif /* SRC_STATION_H_ */
