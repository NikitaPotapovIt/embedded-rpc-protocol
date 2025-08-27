#include "../../include/protocol/sender.hpp"
#include "../../include/protocol/crc.hpp"
#include "../../include/drivers/uart.hpp"
#include "../../include/rpc/types.hpp"

namespace protocol {

Sender::Sender(drivers::Uart& uart) : m_uart(uart) {}

bool Sender::send_transport(const std::uint8_t* data, std::size_t length, std::uint8_t seq, rpc::MessageType type) {
    std::uint8_t packet[Packet::MaxSize + 7]; // 0xFA, l_l, l_h, crc8, 0xFB, data, crc8, 0xFE
    packet[0] = 0xFA; // Header
    packet[1] = length & 0xFF; // l_l
    packet[2] = length >> 8; // l_h
    packet[3] = Crc::calculate(packet, 3); // Header CRC
    packet[4] = 0xFB; // Data start
    for (std::size_t i = 0; i < length; ++i) {
        packet[5 + i] = data[i];
    }
    packet[5 + length] = Crc::calculate(data, length); // Data CRC
    packet[6 + length] = 0xFE; // Stop byte
    return m_uart.send(packet, length + 7, pdMS_TO_TICKS(100));
}

} // namespace protocol
