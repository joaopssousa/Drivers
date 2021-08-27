/*
 * battery_monitor.c
 *
 *  Created on: Apr 22, 2021
 *  Author: Germano Henrique
 *  Brief: Declare the functions used in the main file.
 */

#include "battery_monitor.h"
#include "stm32f4xx_hal.h"

#define MAX_RESOLUTION_ADC 4095	// 12 bits resolution


/***************************
 *  VBAT List
 */

#define VBAT_4V  0
#define VBAT_12V 1

//**************************

#define VBAT VBAT_4V // Indicates Max battery voltage

#if VBAT == VBAT4V
	#define RESISTOR1 470U
	#define RESISTOR2 1000U
#elif VBAT == VBAT_12V
	#define RESISTOR1 10000U
	#define RESISTOR2 3400U
#endif


#define VOLTAGE_DIVIDER_RATIO 		((RESISTOR1 + RESISTOR2) / (double)(RESISTOR2)) // ratio between Vin and Vout
#define MAX_INPUT_VOLTAGE_ON_ADC 	3.3
#define OFFSET 						0.1 										   // offset value used to correct final voltage

static void config_vbat_reader(void);

static ADC_HandleTypeDef hadc_bat_monitor;

// starts and configures the peripherals that are to be used
void init_battery_monitor(void)
{
	config_vbat_reader();
}

// returns the voltage of a battery
double get_battery_voltage (void)
{
	HAL_ADC_Start(&hadc_bat_monitor);
	// inicializa a conversão em PC14
	HAL_ADC_PollForConversion(&hadc_bat_monitor, 1000);

	uint16_t adc_return_value = HAL_ADC_GetValue(&hadc_bat_monitor);
	double voltage_on_adc_pin = adc_return_value *  MAX_INPUT_VOLTAGE_ON_ADC / MAX_RESOLUTION_ADC;
	double battery_voltage = (VOLTAGE_DIVIDER_RATIO * voltage_on_adc_pin) - OFFSET;

	return battery_voltage;
}

/**
  VBAT (PC14) peripheral initialization
  */
static void config_vbat_reader(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc_bat_monitor.Instance = ADC1;
  hadc_bat_monitor.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc_bat_monitor.Init.Resolution = ADC_RESOLUTION_12B;
  hadc_bat_monitor.Init.ScanConvMode = DISABLE;
  hadc_bat_monitor.Init.ContinuousConvMode = ENABLE;
  hadc_bat_monitor.Init.DiscontinuousConvMode = DISABLE;
  hadc_bat_monitor.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc_bat_monitor.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc_bat_monitor.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc_bat_monitor.Init.NbrOfConversion = 1;
  hadc_bat_monitor.Init.DMAContinuousRequests = DISABLE;
  hadc_bat_monitor.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc_bat_monitor) != HAL_OK)
  {
    while(1);
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc_bat_monitor, &sConfig) != HAL_OK)
  {
	  while(1);
  }

}
