#include "main.h"
#include "stm32f4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

extern UART_HandleTypeDef huart2;

// Объявления функций FreeRTOS (из port.c)
extern void xPortSysTickHandler(void);
extern void xPortPendSVHandler(void);
extern void vPortSVCHandler(void);

void NMI_Handler(void) {}
void HardFault_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void) { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }

void DebugMon_Handler(void) {}

void USART2_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart2);
}
