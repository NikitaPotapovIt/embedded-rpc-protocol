#include "../../include/drivers/uart.hpp"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart == &huart2) {
        std::uint8_t byte = huart->Instance->DR;
        auto& uart = drivers::Uart::instance(); // Получение байт из регистра

        if (uart.m_rx_callback) {
            uart.m_rx_callback(byte); // Вызов callback при прирывании
        }

        HAL_UART_Receive_IT(huart, &byte. 1); // Перезапуск
    }
}

namespace drivers {

    bool Uart::init() {
        return HAL_UART_GetState(huart&) == HAL_UART_STATE_READY;
    }

    bool Uart::send(const std uint8_t* data, std::size_t size) {
        return HAL_UART_Transmit(&huart, const_cast<std::uint8_t*>(data), size, 1000) == HAL_OK;
    }

    void Uart::set_rx_callback(void (*callback)(std::uint8_t)) {
        m_rx_callback = callback;

        std::uint8_t byte;
        HAL_UART_Receive_IT(&huart2, &byte, 1);
    }
} // namespase drivers
