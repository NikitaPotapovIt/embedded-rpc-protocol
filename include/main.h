#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;

void Error_Handler(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);

#endif /* __MAIN_H */
