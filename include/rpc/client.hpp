#pragma once
#include <cstdint>
#include <string>
#include "FreeRTOS.h"
#include "queue.h"
#include "../protocol/packet.hpp"
#include "../protocol/parser.hpp"
#include "../drivers/uart.hpp"
#include "../rpc/types.hpp"

namespace rpc {

/**
 * RPC клиент для удаленного вызова процедур через бинарный протокол
 * 
 * Обеспечивает синхронные и асинхронные вызовы с обработкой ответов
 * Для работы требует предварительно инициализированные UART и Parser
 */

class Client {
public:
    // Конструктор RPC клиента
    Client(drivers::Uart& uart, protocol::Parser& parser);
    
    // Ожидание ответа по порядковому номеру (raw packet)
    bool wait_response(protocol::Packet& response, std::uint8_t seq, TickType_t timeout);
    // Ожидание ответа по порядковому номеру (parsed message)
    bool wait_response(Message& response, std::uint8_t seq, TickType_t timeout);

    // Синхронный вызов RPC функции с ожиданием результата
    template<typename Result, typename... Args>
    Result call(const std::string& func_name, Args... args);

    // Асинхронный вызов RPC функции без ожидания результата
    template<typename... Args>
    void stream_call(const std::string& func_name, Args... args);

    // Отправка сырого пакета сообщения
    bool send_message(const protocol::Packet& msg);

    // Возвращает очередь для приема ответов
    QueueHandle_t get_response_queue() const { return m_response_queue; }

private:
    drivers::Uart& m_uart;              // Драйвер UART для отправки запросов
    protocol::Parser& m_parser;         // Парсер для обработки ответов
    std::uint8_t m_sequence{0};         // Текущий порядковый номер
    QueueHandle_t m_response_queue;     // Очередь для приема ответов
};

} // namespace rpc
