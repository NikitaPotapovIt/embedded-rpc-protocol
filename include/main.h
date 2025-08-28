#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

/**
 * Основной заголовочный файл проекта STM32
 * 
 * Содержит объявления HAL-объектов и функций инициализации
 */

// Обработчик UART2 для коммуникации
extern UART_HandleTypeDef huart2;

void Error_Handler(void);           // Обработчик критических ошибок
void SystemClock_Config(void);      // Конфигурация системной частоты
void MX_GPIO_Init(void);            // Инициализация GPIO
void MX_USART2_UART_Init(void);     // Инициализация UART2

#endif /* __MAIN_H */
