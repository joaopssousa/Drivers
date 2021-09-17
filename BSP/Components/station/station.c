/*
 * sensors.c
 *
 *  Created on: Nov 5, 2020
 *      Author: LOCEM
 */

#include "station.h"
#include "bmp280.h"
#include "hw.h"
#include <string.h>


#define TOLERANCIA 15
#define MIN_VAL(X) X*(1-(TOLERANCIA/100))
#define MAX_VAL(X) X*(1+(TOLERANCIA/100))
#define GET_RAW_VOLTAGE(X) (3.255/4096)*X
#define MAX_READINGS 100
#define TOTAL_WIND_POSITION 8

#define RADIUS_ANEMOMETER_MM 147
#define PI_NUMBER 3.14159265f
#define PERIOD 5


Estation_Parameters Parameters;
float pressure, temperature, humidity;
flags_station flagsStation;


BMP280_HandleTypedef bmp280;

uint32_t count_velo = 0;
uint32_t aux_count_velo = 0;
uint16_t pluviometer_count=0;

extern char Buffer_to_send[sizeof(Estation_Parameters)];

extern ADC_HandleTypeDef hadc2;
extern I2C_HandleTypeDef hi2c1;

static void Call_Pluviometer(Estation_Parameters *Parameters);
static bool Call_BME280(Estation_Parameters *Parameters);
static bool Call_Biruta(Estation_Parameters *Parameters);
static void Call_Anemometro(Estation_Parameters *Parameters);

void init_station()
{

	/*
	 * Inicializa os parâmetros padrão do BMP280.
	 */
	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
	bmp280.i2c = &hi2c1;

	if (!bmp280_init(&bmp280, &bmp280.params)) {
		flagsStation.bmp_failed=1;
	}

	GPIO_InitTypeDef initStruct={0};

	/* Configura Interrupção do Anemômetro */
	initStruct.Mode = GPIO_MODE_IT_RISING;
	initStruct.Pull = GPIO_PULLUP;
	HW_GPIO_Init(ANEMOMETRO_GPIO_Port, ANEMOMETRO_Pin, &initStruct);

	HW_GPIO_SetIrq(ANEMOMETRO_GPIO_Port, ANEMOMETRO_Pin, 0, mede_velocidade_vento);

	/* Configura Interrupção do Pluviometro */
	initStruct.Mode = GPIO_MODE_IT_RISING;
	initStruct.Pull = GPIO_PULLDOWN;

	HW_GPIO_Init(PLUVIOMETRO_GPIO_Port, PLUVIOMETRO_Pin, &initStruct);

	HW_GPIO_SetIrq(PLUVIOMETRO_GPIO_Port, PLUVIOMETRO_Pin, 0, mede_mm_chuva);

}

/*
 * Função que chama retorna dados de temperatuda, pressão e humidade
*/
static bool Call_BME280(Estation_Parameters *Parameters)
{

	if(!flagsStation.bmp_failed){
		if (!bmp280_read_float(&bmp280, &temperature, &pressure, &humidity))
		{
			// ACIONAR UMA FLAG PARA DIZER QUE DEU PROBLEMA NA LEITURA DO BMP280
			// Por padrão se a inicialização não funcionar todos retornam 0xFF.
			Parameters->temperatura = 0xFF;
			Parameters->pressao = 0xFF;
			Parameters->humidade = 0xFF;
			return 0;
		}

		// Faz a requisição dos dados do BME280
		bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);

		// Muda os dados para poder enviar com duas casa decimais para envio
		Parameters->temperatura = (float)temperature*100;
		Parameters->pressao = (float)pressure*100;
		Parameters->humidade = (float)humidity*100;
		return 1;
	}
	return 0;
}

/*
 * Função que retorna dados de precipitação em mm.
 */
static void Call_Pluviometer(Estation_Parameters *Parameters)
{

	Parameters->pluviometria = ((double)(pluviometer_count/4))*1000;
	PRINTF("Pluvi %d\r\n", pluviometer_count);

}


static bool Call_Biruta(Estation_Parameters *Parameters)
{
	ADC_ChannelConfTypeDef adcConf = {0};

    adcConf.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    adcConf.Channel = ADC_CHANNEL_7;
    adcConf.Rank = 1;
    HAL_ADC_ConfigChannel(&hadc2, &adcConf);

	// Recebe leitura do ADC da biruta e passa para variavel posicao
	double tensao_de_posicao = 0;
	//static float resistencia[8] = {4.67, 4.63, 4.58, 4.51, 4.4, 4.25, 3.97, 3.33};
    //static float tensao[] = {1.639, 1.098, 0.825, 0.660, 0.550, 0.470, 0.412, 0.366};

	static float tensao[TOTAL_WIND_POSITION] = {1.0, 0.609, 0.436, 0.338, 0.278, 0.234, 0.2, 0.18};

	double raw = 0;
	double raw2 = 0;
	// Ciclo que faz leituras e em seguida calcula a media das mesmas
	for(int i = 0;i<MAX_READINGS;i++)
	{
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
		raw = (double) HAL_ADC_GetValue(&hadc2);
		raw2 = GET_RAW_VOLTAGE(raw);
		// Divide cada amostra pelo total de leituras para que ao fim do ciclo ja tenha-se a média
		tensao_de_posicao += raw2/MAX_READINGS;
		HAL_ADC_Stop(&hadc2);
	}

	// Por conta de algum erro ou incongruência a leitura fica 4-5% abaixo do real então
	// adicionei um ganho de 4% ao valor obtido.
	//tensao_de_posicao *= 1.04;
	for(int i=TOTAL_WIND_POSITION-1; i>=0; i--){
		if (tensao_de_posicao <= MAX_VAL(tensao[i])){
			Parameters->direcao_vento = i;
			return 1;
		}
	}
	return 0;
}


static void Call_Anemometro(Estation_Parameters *Parameters)
{
	float RPM = (float)count_velo*60/PERIOD;		// Rotações por Minuto
	float Velo_mps = ((float)4*PI_NUMBER*RADIUS_ANEMOMETER_MM*RPM/60)/1000.0;	// Comprimento da circunferência em milimetros.

	Parameters->velocidade_vento = Velo_mps*100;
	//float Velo_mps = (float)comprimento*RPS;  	// Calculate wind speed on m/s
	//float Velo_kmph = Velo_mps*3.6;  		// Calculate wind speed on km/h
}

void read_sensors(Estation_Parameters *Parameters)
{
	Call_BME280(Parameters);

	Call_Pluviometer(Parameters);

	Call_Anemometro(Parameters);

	PRINTF("Contador Vento %d\r\n", count_velo);

	Call_Biruta(Parameters);


	int i = sizeof(Estation_Parameters) - 1;
	int j = 0;
	//inverte a ordem dos dados.
	for(int k = i/2;k>0;k--)
	{
		char temp = Buffer_to_send[j];
		Buffer_to_send[j] = Buffer_to_send[i];
		Buffer_to_send[i] = temp;
		j++;i--;
	}
	memcpy(Buffer_to_send,Parameters,sizeof(Estation_Parameters));

}

void muda_buffer(Sensor_AppData *AppData, char Buffer_to_send[])
{
	memcpy(AppData->Buff,Buffer_to_send,sizeof(Estation_Parameters));
}

void mede_mm_chuva(){
	pluviometer_count++;
	flagsStation.pluviometer=1;
}

void mede_velocidade_vento(){
	aux_count_velo++;
}
