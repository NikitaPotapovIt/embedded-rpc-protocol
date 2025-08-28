#include "main.h"
#include "stm32f4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

// Внешнее объявление обработчика UART (определен в main)
extern UART_HandleTypeDef huart2;

// Объявления функций FreeRTOS (из port.c)
// Эти функции реализованы в порте FreeRTOS для Cortex-M4
extern void xPortSysTickHandler(void);
extern void xPortPendSVHandler(void);
extern void vPortSVCHandler(void);

void NMI_Handler(void) {}                       // Обработчик Non-Maskable Interrupt (NMI) / Вызывается при критических аппаратных сбоях
void HardFault_Handler(void) { while (1) {} }   // Обработчик Hard Fault Exception / Бесконечный цикл - система останавливается при Hard Fault
void MemManage_Handler(void) { while (1) {} }   // Обработчик Memory Management Fault / Вызывается при ошибках управления памятью (MPU violation, etc.)
void BusFault_Handler(void) { while (1) {} }    // Обработчик Bus Fault Exception / Вызывается при ошибках шины (bus errors)
void UsageFault_Handler(void) { while (1) {} }  // Обработчик Usage Fault Exception / Вызывается при ошибках использования инструкций

void DebugMon_Handler(void) {}                  // Обработчик Debug Monitor Exception / Вызывается при событиях отладки (hardware breakpoints, etc.)

void USART2_IRQHandler(void) {                  // Обработчик прерывания USART2 / Вызывается при событиях UART2: прием/передача данных, ошибки
    HAL_UART_IRQHandler(&huart2);
}
