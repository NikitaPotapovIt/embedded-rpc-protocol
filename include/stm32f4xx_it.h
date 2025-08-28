#ifndef __STM32F4XX_IT_H
#define __STM32F4XX_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Заголовочный файл обработчиков прерываний и исключений Cortex-M4
 * 
 * Содержит объявления всех обработчиков прерываний, используемых в проекте
 */

void NMI_Handler(void);             // Не маскируемое прерывание - highest priority hardware interrupt
void HardFault_Handler(void);       // Вызывается при критических ошибках: access to invalid memory, etc.
void MemManage_Handler(void);       // Вызывается при ошибках управления памятью (MPU violation, etc.)
void BusFault_Handler(void);        // Вызывается при ошибках шины (bus errors)
void UsageFault_Handler(void);      // Вызывается при ошибках использования инструкций
void SVC_Handler(void);             // Вызывается инструкцией SVC для перехода в privileged mode
void DebugMon_Handler(void);        // Вызывается при событиях отладки (hardware breakpoints, etc.)
void PendSV_Handler(void);          // Вызывается по запросу для отложенного обслуживания
void SysTick_Handler(void);         // Вызывается каждую миллисекунду системным таймером / Важнейший обработчик для системного времени и RTOS
void USART2_IRQHandler(void);       // Вызывается при событиях UART2: прием/передача данных, ошибки

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4XX_IT_H */
