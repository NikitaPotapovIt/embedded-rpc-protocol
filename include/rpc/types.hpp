#pragma once
#include <cstdint>
#include <string>

namespace rpc {

/**
 * MessageType
 * Типы сообщений в RPC протоколе
 * 
 * Значения выбраны для удобства отладки (видны в HEX дампе)
 */

enum class MessageType : std::uint8_t {
    Request = 0x0B,     // Запрос на выполнение RPC функции (клиент → сервер)
    Stream = 0x0C,      // Stream-сообщение (одностороннее, без ответа)
    Response = 0x16,    // Ответ на RPC запрос (сервер → клиент)
    Error = 0x21        // Сообщение об ошибке выполнения
};

// Структура RPC сообщения для внутренней обработки. Распарсенное представление сообщения
struct Message {
    MessageType type;                   // Тип сообщения
    std::uint8_t sequence_number;       // Порядковый номер для сопоставления запросов-ответов
    std::string function_name;          // Имя вызываемой RPC функции
    const std::uint8_t* arguments;      // Указатель на бинарные аргументы функции
    std::size_t arguments_length;       // Длина аргументов в байтах
};

} // namespace rpc
