#include "../../include/drivers/uart.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace drivers {

Uart* Uart::global_uart_instance = nullptr; // Инициализация статического указателя на глобальный экземпляр UART

// Конструктор UART драйвера
Uart::Uart(UART_HandleTypeDef* huart) : m_huart(huart), m_rx_queue(nullptr), m_rx_callback(nullptr), m_rx_user_data(nullptr), m_rx_byte(0) {}

void Uart::start() {                                                                                    // Запуск UART драйвера
    m_rx_queue = xQueueCreate(64, sizeof(std::uint8_t));                                                // Создание очереди для передачи данных из прерывания в задачу
    HAL_UART_Receive_IT(m_huart, &m_rx_byte, 1);                                                        // Запуск приема данных в прерывании (один байт)
    xTaskCreate(rx_task, "UartRx", configMINIMAL_STACK_SIZE, this, tskIDLE_PRIORITY + 1, nullptr);      // Создание задачи для обработки принятых данных
}

// Установка callback функции для обработки принятых данных
void Uart::set_rx_callback(void (*callback)(std::uint8_t, void*), void* user_data) {
    m_rx_callback = callback;
    m_rx_user_data = user_data;
}

// Отправка данных через UART
bool Uart::send(const std::uint8_t* data, std::size_t length, TickType_t timeout) {
    if (HAL_UART_Transmit(m_huart, const_cast<std::uint8_t*>(data), length, timeout) == HAL_OK) {
        return true;
    }
    return false;
}

// Задача FreeRTOS для обработки принятых данных
void Uart::rx_task(void* arg) {
    Uart* uart = static_cast<Uart*>(arg);
    std::uint8_t byte;
    while (true) {
        if (xQueueReceive(uart->m_rx_queue, &byte, portMAX_DELAY) == pdPASS) {
            if (uart->m_rx_callback) {
                uart->m_rx_callback(byte, uart->m_rx_user_data);
            }
        }
    }
}

} // namespace drivers

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {                                // Callback обработки завершения приема UART (HAL)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    drivers::Uart* uart = drivers::Uart::get_global_instance();                                     // Получение глобального экземпляра UART
    if (uart && uart->get_huart() == huart) {
        xQueueSendFromISR(uart->get_rx_queue(), &uart->get_rx_byte(), &xHigherPriorityTaskWoken);   // Отправка принятого байта в очередь (из прерывания)
        HAL_UART_Receive_IT(huart, &uart->get_rx_byte(), 1);                                        // Перезапуск приема следующего байта
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);                                                   // Переключение контекста если необходимо
}
