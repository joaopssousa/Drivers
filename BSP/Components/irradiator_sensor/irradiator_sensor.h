/*
 * irradiator_sensor.h
 *
 *  Created on: Jul 6, 2021
 */

#ifndef __IRRADIATOR_SENSOR_H
#define __IRRADIATOR_SENSOR_H

#define PACKET_SIZE 30
#define MAX_MEASURES  5

#define IRRADIATOR_TX_Pin GPIO_PIN_2
#define IRRADIATOR_TX_GPIO_Port GPIOA
#define IRRADIATOR_RX_Pin GPIO_PIN_3
#define IRRADIATOR_RX_GPIO_Port GPIOA

extern UART_HandleTypeDef huart2;

extern uint8_t count_measures;
extern uint32_t measures;


//TODO Comentar descrição da função
uint32_t getIntMeasure(void);

/*!
 * @brief Start uart2, gpio, open to receive first packet
 * @param none
 * @retval none
 */
void init_irradiator(void);

/*!
 * @brief Calculate the mean
 * @param number	Number of measurements to be calculated
 * @retval Mean
 */
uint32_t mediaCalculator(uint8_t number);

#endif /* __IRRADIATOR_SENSOR_H */
