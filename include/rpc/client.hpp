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

class Client {
public:
    Client(drivers::Uart& uart, protocol::Parser& parser);
    
    bool wait_response(protocol::Packet& response, std::uint8_t seq, TickType_t timeout);
    bool wait_response(Message& response, std::uint8_t seq, TickType_t timeout);

    template<typename Result, typename... Args>
    Result call(const std::string& func_name, Args... args);

    template<typename... Args>
    void stream_call(const std::string& func_name, Args... args);

    bool send_message(const protocol::Packet& msg);

    QueueHandle_t get_response_queue() const { return m_response_queue; }

private:
    drivers::Uart& m_uart;
    protocol::Parser& m_parser;
    std::uint8_t m_sequence{0};
    QueueHandle_t m_response_queue;
};

} // namespace rpc
