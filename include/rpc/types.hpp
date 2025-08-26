#pragma once
#include <cstdint>

namespace rpc {

    enum class MessageType : std::uint8_t {
        Request = 0x0B,   // Запрос
        Stream = 0x0C,    // Поток
        Response = 0x16,  // Ответ
        Error = 0x21      // Ошибка
    };

    /**
     * @brief Базовая структура RPC сообщения
     */
    struct Message {
        MessageType type;
        std::uint8_t sequence_number;
        const char* function_name;
        const std::uint8_t* arguments;
        std::size_t arguments_length;
    };
} // namespace rpc
