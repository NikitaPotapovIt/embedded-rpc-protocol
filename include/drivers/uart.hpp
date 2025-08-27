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
    void set_rx_callback(void (*callback)(std::uint8_t));
    bool send(const std::uint8_t* data, std::size_t length, TickType_t timeout);

private:
    friend void HAL_UART_RxCpltCallback(UART_HandleTypeDef*);
    static void rx_task(void* arg);
    UART_HandleTypeDef* m_huart;
    QueueHandle_t m_rx_queue;
    void (*m_rx_callback)(std::uint8_t);
    std::uint8_t m_rx_byte;
};

} // namespace drivers
