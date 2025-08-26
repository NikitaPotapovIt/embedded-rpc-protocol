#include "../../include/rpc/service.hpp"
#include "../../include/protocol/sender.hpp"
#include "../../include/drivers/uart.hpp"
#include <cstring>

namespace rpc {

void Service::process_packet(const protocol::Packet& packet) {
    if (!packet.valid || packet.length < 5) return;

    Message request;
    request.type = static_cast<MessageType>(packet.data[5]); // After header
    request.sequence_number = packet.data[6];

    size_t name_offset = 7;
    request.function_name = reinterpret_cast<const char*>(&packet.data[name_offset]);
    size_t name_length = std::strlen(request.function_name);
    size_t args_offset = name_offset + name_length + 1;
    request.arguments = &packet.data[args_offset];
    request.arguments_length = packet.length - args_offset - 2; // Exclude footer CRC, stop byte

    if (request.type == MessageType::Response || request.type == MessageType::Error) {
        // Push to response queue
        xQueueSend(Client::instance().m_response_queue, &request, portMAX_DELAY);
        return;
    }

    auto it = m_handlers.find(std::string(request.function_name, name_length));
    if (it == m_handlers.end()) {
        Message response;
        response.type = MessageType::Error;
        response.sequence_number = request.sequence_number;
        response.function_name = "";
        const char* error_msg = "Function not found";
        response.arguments = reinterpret_cast<const std::uint8_t*>(error_msg);
        response.arguments_length = std::strlen(error_msg);
        send_response(response);
        return;
    }

    if (request.type == MessageType::Request) {
        std::array<std::uint8_t, 64> result_buffer;
        size_t result_length = 0;
        it->second->invoke(request.arguments, request.arguments_length, result_buffer.data(), &result_length);

        Message response;
        response.type = MessageType::Response;
        response.sequence_number = request.sequence_number;
        response.function_name = "";
        response.arguments = result_buffer.data();
        response.arguments_length = result_length;
        send_response(response);
    } else if (request.type == MessageType::Stream) {
        std::array<std::uint8_t, 64> result_buffer;
        size_t result_length = 0;
        it->second->invoke(request.arguments, request.arguments_length, result_buffer.data(), &result_length);
    }
}

void Service::send_response(const Message& response) {
    std::array<std::uint8_t, Packet::MaxSize> buffer;
    std::size_t offset = 0;

    buffer[offset++] = static_cast<std::uint8_t>(response.type);
    buffer[offset++] = response.sequence_number;
    buffer[offset++] = 0; // Empty function name
    std::memcpy(&buffer[offset], response.arguments, response.arguments_length);
    offset += response.arguments_length;

    protocol::Sender::instance().send_transport(buffer.data(), offset);
}

} // namespace rpc
