#include "main.h"

/**
 * Инициализация MSP (MCU Support Package) для UART
 * huart Указатель на структуру обработчика UART
 * 
 * Вызывается автоматически из HAL_UART_Init()
 * Выполняет низкоуровневую инициализацию периферии:
 * - Включает тактирование USART и GPIO
 * - Настраивает пины в alternate function mode
 * - Конфигурирует параметры GPIO
 * 
 * Не вызывать напрямую - вызывается HAL автоматически
 */

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // Проверка, что инициализируется именно USART2
    if (huart->Instance == USART2) {
        // 1. Включение тактирования USART2
        __HAL_RCC_USART2_CLK_ENABLE();
        // 2. Включение тактирования GPIOA (порт A)
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // 3. Настройка пинов PA2 (TX) и PA3 (RX) для USART2
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;      // PA2 и PA3
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;             // Alternate Function Push-Pull
        GPIO_InitStruct.Pull = GPIO_NOPULL;                 // No pull-up/pull-down
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  // Высокая скорость
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;        // Alternate Function 7 для USART2
        // 4. Применение настроек к GPIOA
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
 * Деинициализация MSP (MCU Support Package) для UART
 * huart Указатель на структуру обработчика UART
 * 
 * Вызывается автоматически из HAL_UART_DeInit()
 * Выполняет обратную инициализации:
 * - Отключает тактирование периферии
 * - Возвращает пины в исходное состояние
 * 
 * Не вызывать напрямую - вызывается HAL автоматически
 */

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {
    if (huart->Instance == USART2) {                        // Проверка, что деинициализируется именно USART2
        __HAL_RCC_USART2_CLK_DISABLE();                     // 1. Отключение тактирования USART2
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);    // 2. Деинициализация пинов PA2 (TX) и PA3 (RX)
    }
}
