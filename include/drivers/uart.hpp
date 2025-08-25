#pragma once
#include "../utils/noncopyable.hpp"
#include "../utils/allocator.hpp"

namespace drivers {

    class Uart : private utils::NonCopyable {
        public:
        static Uart& instance() {
            static Uart instance;
            return instance;
        }

        bool init();
        bool send(const std::uint8_t* data, std::size_t size);
        void set_rx_callback(void (*callback)(std::uint8_t));

        private:
        Uart() = default;
        void (*m_rx_callback)(std::uint8_t){nullptr};

        friend void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);
    };
} // namespase drivers
