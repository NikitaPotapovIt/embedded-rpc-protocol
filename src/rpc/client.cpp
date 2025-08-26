#include "../../include/rpc/client.hpp"
#include "../../include/protocol/sender.hpp"
#include <cstring>
#include "FreeRTOS.h"
#include "queue.h"

namespace rpc {

Client::Client() {
    m_response_queue = xQueueCreate(16, sizeof(Message));
}

template<typename Result, typename... Args>
Result Client::call(const std::string& func_name, Args... args) {
    // Serialize arguments
    std::tuple<Args...> args_tuple(args...);
    std::array<std::uint8_t, 64> args_buffer;
    std::size_t args_length = 0;
    Serializer::serialize_tuple_impl(args_tuple, args_buffer.data(), std::index_sequence_for<Args...>{});
    args_length = Serializer::offset_of<sizeof...(Args)>();

    // Form message
    Message msg;
    msg.type = MessageType::Request;
    msg.sequence_number = m_sequence++;
    msg.function_name = func_name.c_str();
    msg.arguments = args_buffer.data();
    msg.arguments_length = args_length;

    // Send
    if (!send_message(msg)) {
        throw std::runtime_error("Failed to send request");
    }

    // Wait response
    Message response;
    if (!wait_response(response, msg.sequence_number, pdMS_TO_TICKS(1000))) {
        throw std::runtime_error("Response timeout");
    }

    if (response.type == MessageType::Error) {
        throw std::runtime_error("Remote error: " + std::string(reinterpret_cast<const char*>(response.arguments), response.arguments_length));
    }

    // Deserialize result
    return Serializer::deserialize<Result>(response.arguments);
}

template<typename... Args>
void Client::stream_call(const std::string& func_name, Args... args) {
    std::tuple<Args...> args_tuple(args...);
    std::array<std::uint8_t, 64> args_buffer;
    std::size_t args_length = 0;
    Serializer::serialize_tuple_impl(args_tuple, args_buffer.data(), std::index_sequence_for<Args...>{});
    args_length = Serializer::offset_of<sizeof...(Args)>();

    Message msg;
    msg.type = MessageType::Stream;
    msg.sequence_number = m_sequence++;
    msg.function_name = func_name.c_str();
    msg.arguments = args_buffer.data();
    msg.arguments_length = args_length;

    send_message(msg);
}

bool Client::send_message(const Message& msg) {
    std::array<std::uint8_t, Packet::MaxSize> buffer;
    std::size_t offset = 0;

    buffer[offset++] = static_cast<std::uint8_t>(msg.type);
    buffer[offset++] = msg.sequence_number;
    std::strcpy(reinterpret_cast<char*>(&buffer[offset]), msg.function_name);
    offset += std::strlen(msg.function_name) + 1;
    std::memcpy(&buffer[offset], msg.arguments, msg.arguments_length);
    offset += msg.arguments_length;

    return protocol::Sender::instance().send_transport(buffer.data(), offset);
}

bool Client::wait_response(Message& response, std::uint8_t seq, TickType_t timeout) {
    Message msg;
    while (xQueueReceive(m_response_queue, &msg, timeout) == pdPASS) {
        if (msg.sequence_number == seq && (msg.type == MessageType::Response || msg.type == MessageType::Error)) {
            response = msg;
            return true;
        }
    }
    return false;
}

// Explicit template instantiations (adjust based on expected types)
template int32_t Client::call<int32_t, int32_t, int32_t>(const std::string&, int32_t, int32_t);
template float Client::call<float>(const std::string&);
template void Client::call<void, bool>(const std::string&, bool);
template void Client::stream_call<bool>(const std::string&, bool);

} // namespace rpc
