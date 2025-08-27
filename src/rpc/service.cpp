#include "../../include/rpc/service.hpp"
#include "../../include/protocol/sender.hpp"
#include <cstring>
#include "FreeRTOS.h"
#include "queue.h"

namespace rpc {

void service_packet_handler(const protocol::Packet& packet, void* service_ptr) {
    auto* service = static_cast<Service*>(service_ptr);
    service->handle_packet(packet);
}

Service::Service(protocol::Parser& parser) : m_parser(parser) {
    m_parser.set_handler([](const protocol::Packet& packet, void* arg) { service_packet_handler(packet, arg); }, this);
}

void Service::handle_packet(const protocol::Packet& packet) {
    if (!packet.valid || packet.func_name.empty()) {
        return;
    }

    auto it = m_handlers.find(packet.func_name);
    if (it == m_handlers.end()) {
        protocol::Packet error_packet;
        error_packet.valid = true;
        error_packet.seq = packet.seq;
        error_packet.type = MessageType::Error;
        error_packet.func_name = packet.func_name;
        error_packet.data[0] = static_cast<std::uint8_t>(MessageType::Error);
        error_packet.data_length = 1;

        protocol::Sender sender(m_parser.get_uart());
        sender.send_transport(error_packet.data, error_packet.data_length, error_packet.seq, error_packet.type);
        return;
    }

    std::uint8_t response[protocol::Packet::MaxSize];
    std::size_t response_length = 0;
    it->second(packet.data + packet.func_name.size() + 2, packet.data_length - (packet.func_name.size() + 2), response, &response_length);

    protocol::Packet response_packet;
    response_packet.valid = true;
    response_packet.seq = packet.seq;
    response_packet.type = MessageType::Response;
    response_packet.func_name = packet.func_name;
    response_packet.data_length = response_length + packet.func_name.size() + 2;
    response_packet.data[0] = static_cast<std::uint8_t>(MessageType::Response);
    response_packet.data[1] = packet.seq;
    std::memcpy(response_packet.data + 2, packet.func_name.c_str(), packet.func_name.size() + 1);
    std::memcpy(response_packet.data + packet.func_name.size() + 2, response, response_length);

    protocol::Sender sender(m_parser.get_uart());
    sender.send_transport(response_packet.data, response_packet.data_length, response_packet.seq, response_packet.type);
}

void Service::process() {
    // Реализация метода process (если требуется)
}

} // namespace rpc
