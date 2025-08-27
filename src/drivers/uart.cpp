#include "../../include/drivers/uart.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace drivers {

Uart* global_uart_instance = nullptr;

Uart::Uart(UART_HandleTypeDef* huart) : m_huart(huart), m_rx_queue(nullptr), m_rx_callback(nullptr), m_rx_byte(0) {
    global_uart_instance = this;
}

void Uart::start() {
    m_rx_queue = xQueueCreate(64, sizeof(std::uint8_t));
    HAL_UART_Receive_IT(m_huart, &m_rx_byte, 1);
    xTaskCreate(rx_task, "UartRx", configMINIMAL_STACK_SIZE, this, tskIDLE_PRIORITY + 1, nullptr);
}

void Uart::set_rx_callback(void (*callback)(std::uint8_t)) {
    m_rx_callback = callback;
}

bool Uart::send(const std::uint8_t* data, std::size_t length, TickType_t timeout) {
    if (HAL_UART_Transmit(m_huart, const_cast<std::uint8_t*>(data), length, timeout) == HAL_OK) {
        return true;
    }
    return false;
}

void Uart::rx_task(void* arg) {
    Uart* uart = static_cast<Uart*>(arg);
    std::uint8_t byte;
    while (true) {
        if (xQueueReceive(uart->m_rx_queue, &byte, portMAX_DELAY) == pdPASS) {
            if (uart->m_rx_callback) {
                uart->m_rx_callback(byte);
            }
        }
    }
}

} // namespace drivers

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    drivers::Uart* uart = drivers::Uart::global_uart_instance;
    if (uart && uart->m_huart == huart) {
        xQueueSendFromISR(uart->m_rx_queue, &uart->m_rx_byte, &xHigherPriorityTaskWoken);
        HAL_UART_Receive_IT(huart, &uart->m_rx_byte, 1);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
