#pragma once
#include <cstdint>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "../utils/noncopyable.hpp"

namespace drivers {

class Uart : private utils::NonCopyable {
public:
    explicit Uart(UART_HandleTypeDef* huart);
    void start();
    void set_rx_callback(void (*callback)(std::uint8_t, void*), void* user_data);
    bool send(const std::uint8_t* data, std::size_t length, TickType_t timeout);

    // Геттеры для доступа из HAL_UART_RxCpltCallback
    static Uart* get_global_instance() { return global_uart_instance; }
    UART_HandleTypeDef* get_huart() { return m_huart; }
    QueueHandle_t get_rx_queue() { return m_rx_queue; }
    std::uint8_t& get_rx_byte() { return m_rx_byte; }

private:
    static void rx_task(void* arg);
    static Uart* global_uart_instance;
    UART_HandleTypeDef* m_huart;
    QueueHandle_t m_rx_queue;
    void (*m_rx_callback)(std::uint8_t, void*);
    void* m_rx_user_data;
    std::uint8_t m_rx_byte;
};

} // namespace drivers
