#include "../../include/rpc/client.hpp"
#include "../../include/rpc/serializer.hpp"
#include "../../include/protocol/sender.hpp"
#include <cstring>

namespace rpc {

Client::Client(drivers::Uart& uart, protocol::Parser& parser) 
    : m_uart(uart), m_parser(parser), m_sequence(0), m_response_queue(xQueueCreate(10, sizeof(protocol::Packet))) {}

bool Client::wait_response(protocol::Packet& response, std::uint8_t seq, TickType_t timeout) {
    protocol::Packet packet;
    if (xQueueReceive(m_response_queue, &packet, timeout) == pdPASS && packet.seq == seq) {
        response = packet;
        return true;
    }
    return false;
}

bool Client::wait_response(Message& response, std::uint8_t seq, TickType_t timeout) {
    protocol::Packet packet;
    if (wait_response(packet, seq, timeout)) {
        response.type = packet.type;
        response.sequence_number = packet.seq;
        response.function_name = packet.func_name;
        response.arguments = packet.data + packet.func_name.size() + 2; // Skip type and name
        response.arguments_length = packet.data_length - (packet.func_name.size() + 2);
        return true;
    }
    return false;
}

template<typename Result, typename... Args>
Result Client::call(const std::string& function_name, Args... args) {
    protocol::Packet packet;
    packet.valid = true;
    packet.seq = m_sequence++;
    packet.func_name = function_name;
    packet.type = MessageType::Request;

    std::uint8_t buffer[protocol::Packet::MaxSize];
    buffer[0] = static_cast<std::uint8_t>(packet.type);
    buffer[1] = packet.seq;
    std::memcpy(buffer + 2, function_name.c_str(), function_name.size() + 1); // Include null terminator
    std::size_t offset = function_name.size() + 2;

    std::tuple<Args...> args_tuple{args...};
    Serializer::serialize_tuple(args_tuple, buffer + offset);
    packet.data_length = offset + Serializer::offset_of<sizeof...(Args), Args...>();
    std::memcpy(packet.data, buffer, packet.data_length);

    if (send_message(packet)) {
        protocol::Packet response;
        if (wait_response(response, packet.seq, pdMS_TO_TICKS(1000))) {
            if (response.type == MessageType::Response) {
                return Serializer::deserialize<Result>(response.data + response.func_name.size() + 2);
            } else if (response.type == MessageType::Error) {
                return Result{};
            }
        }
    }
    return Result{};
}

template<typename... Args>
void Client::stream_call(const std::string& function_name, Args... args) {
    protocol::Packet packet;
    packet.valid = true;
    packet.seq = m_sequence++;
    packet.func_name = function_name;
    packet.type = MessageType::Stream;

    std::uint8_t buffer[protocol::Packet::MaxSize];
    buffer[0] = static_cast<std::uint8_t>(packet.type);
    buffer[1] = packet.seq;
    std::memcpy(buffer + 2, function_name.c_str(), function_name.size() + 1); // Include null terminator
    std::size_t offset = function_name.size() + 2;

    std::tuple<Args...> args_tuple{args...};
    Serializer::serialize_tuple(args_tuple, buffer + offset);
    packet.data_length = offset + Serializer::offset_of<sizeof...(Args), Args...>();
    std::memcpy(packet.data, buffer, packet.data_length);

    send_message(packet);
}

bool Client::send_message(const protocol::Packet& msg) {
    protocol::Sender sender(m_uart);
    return sender.send_transport(msg.data, msg.data_length, msg.seq, msg.type);
}

} // namespace rpc

// Явное инстанцирование шаблонов
template int32_t rpc::Client::call<int32_t, int32_t, int32_t>(const std::string&, int32_t, int32_t);
template float rpc::Client::call<float>(const std::string&);
template void rpc::Client::call<void, bool>(const std::string&, bool);
template void rpc::Client::stream_call<bool>(const std::string&, bool);
