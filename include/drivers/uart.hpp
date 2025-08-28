#pragma once
#include <cstdint>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "../utils/noncopyable.hpp"

namespace drivers {

/**
 * Обертка для работы с UART через FreeRTOS с обработкой в прерываниях
 * 
 * Механизм работы:
 * 1. HAL прерывание получает байт -> кладет в очередь
 * 2. Задача FreeRTOS читает из очереди -> вызывает пользовательский callback
 * 3. Отправка данных блокирующая с таймаутом
 * 
 * Для работы должен быть зарегистрирован в HAL_UART_RxCpltCallback
 */

class Uart : private utils::NonCopyable {
public:
    explicit Uart(UART_HandleTypeDef* huart);   // Должен быть вызван после создания объекта для начала приема данных
    void start();
    void set_rx_callback(void (*callback)(std::uint8_t, void*), void* user_data);
    bool send(const std::uint8_t* data, std::size_t length, TickType_t timeout);

    // Геттеры для доступа из HAL_UART_RxCpltCallback
    // HAL функции на C не могут работать с методами C++ напрямую
    static Uart* get_global_instance() { return global_uart_instance; }
    UART_HandleTypeDef* get_huart() { return m_huart; }
    QueueHandle_t get_rx_queue() { return m_rx_queue; }
    std::uint8_t& get_rx_byte() { return m_rx_byte; }

private:
    static void rx_task(void* arg);
    static Uart* global_uart_instance;  // Для доступа из HAL прерываний (C-контекст)
    UART_HandleTypeDef* m_huart;
    QueueHandle_t m_rx_queue;   // Очередь для передачи данных из прерывания в задачу
    void (*m_rx_callback)(std::uint8_t, void*);
    void* m_rx_user_data;
    std::uint8_t m_rx_byte; // Буфер для приема одного байта в прерывании
};

} // namespace drivers
