#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);

#define LD2_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5

#endif /* __MAIN_H */
