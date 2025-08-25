#pragma once
#include <cstdint>

namespace rpc {

    enum class MessageType : uint8_t {
        Request = 0x0B,      // Запрос
        Stream = 0x0C,       // Поток
        Response = 0x16,     // Ответ
        Error = 0x21         // Ошибка
    };

    struct Message {
        MessageType type;
        uint8_t sequence_number;
        const char* function_name;
        const uint8_t* arguments;
        uint16_t arguments_length;
    };
} // namespace rpc
