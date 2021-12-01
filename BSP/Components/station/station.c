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
static float tensao[TOTAL_WIND_POSITION] = {1.0, 0.609, 0.436, 0.338, 0.278, 0.234, 0.2, 0.18};

extern char Buffer_to_send[sizeof(Estation_Parameters)];

extern ADC_HandleTypeDef hadc2;
extern I2C_HandleTypeDef hi2c1;

static void Call_Pluviometer(Estation_Parameters *Parameters);
static bool Call_BME280(Estation_Parameters *Parameters);
static bool Call_Biruta(Estation_Parameters *Parameters);
static void Call_Anemometro(Estation_Parameters *Parameters);
static void turn_on_bme(void);
//static void turn_off_bme(void);

void reset_bme(void){
	turn_off_bme();
	HAL_Delay(50);
	enable_bme();
}

void enable_bme(void){
	turn_on_bme();
	PRINTF("Iniciando BME\r\n");
	if (!bmp280_init(&bmp280, &bmp280.params)) {
			PRINTF("Falha no BME\r\n");
			flagsStation.bmp_failed=1;
	}
	PRINTF("OK\r\n");
}

void init_station()
{
	/*
	 * Inicializa os parâmetros padrão do BMP280.
	 */
	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
	bmp280.i2c = &hi2c1;

	enable_bme();

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

	/* Configura pino de controle de alimentação do BME */
	initStruct.Mode = GPIO_MODE_OUTPUT_PP;
	initStruct.Pull = GPIO_PULLUP;
	initStruct.Speed = GPIO_SPEED_MEDIUM;
	HW_GPIO_Init(BME_CONTROL_PORT, BME_CONTROL_PIN, &initStruct);
}

/*
 * Função que chama retorna dados de temperatuda, pressão e humidade
*/
static bool Call_BME280(Estation_Parameters *Parameters)
{
	if(!flagsStation.bmp_failed){
		if (!bmp280_read_float(&bmp280, &temperature, &pressure, &humidity))
		{
			PRINTF("Erro na Leitura\r\n");
			Parameters->temperatura = 0xFF;
			Parameters->pressao = 0xFF;
			Parameters->humidade = 0xFF;
			return 0;
		}

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
}


static bool Call_Biruta(Estation_Parameters *Parameters)
{
	ADC_ChannelConfTypeDef adcConf = {0};

	double raw = 0;
	double raw2 = 0;

    adcConf.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    adcConf.Channel = ADC_CHANNEL_7;
    adcConf.Rank = 1;
    HAL_ADC_ConfigChannel(&hadc2, &adcConf);

	// Recebe leitura do ADC da biruta e passa para variavel posicao
	double tensao_de_posicao = 0;

	// Faz MAX_READINGS leituras e em seguida calcula a media
	for(int i = 0;i<MAX_READINGS;i++)
	{
		refresh_iwdg();
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
		raw = (double) HAL_ADC_GetValue(&hadc2);
		raw2 = GET_RAW_VOLTAGE(raw);
		tensao_de_posicao += raw2/MAX_READINGS;
		HAL_ADC_Stop(&hadc2);
	}

	//tensao_de_posicao *= 1.04;	// adicionei um ganho de 4% ao valor obtido.
	for(int i=TOTAL_WIND_POSITION-1; i>=0; i--){
		refresh_iwdg();
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

static void turn_on_bme(void){
	HAL_GPIO_WritePin(BME_CONTROL_PORT, BME_CONTROL_PIN, GPIO_PIN_SET);
	HAL_Delay(50);
}

void turn_off_bme(void){
	HAL_GPIO_WritePin(BME_CONTROL_PORT, BME_CONTROL_PIN, GPIO_PIN_RESET);
}

void read_sensors(Estation_Parameters *Parameters)
{
	refresh_iwdg();
	PRINTF("Leitura do BME\r\n");
	Call_BME280(Parameters);
	refresh_iwdg();
	PRINTF("Leitura do Pluviometro\r\n");
	Call_Pluviometer(Parameters);
	refresh_iwdg();
	PRINTF("Leitura do Anemometro\r\n");
	Call_Anemometro(Parameters);
	refresh_iwdg();
	PRINTF("Contador Vento %d\r\n", count_velo);
	refresh_iwdg();
	PRINTF("Leitura da Biruta\r\n");
	Call_Biruta(Parameters);

	refresh_iwdg();
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
	refresh_iwdg();
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
