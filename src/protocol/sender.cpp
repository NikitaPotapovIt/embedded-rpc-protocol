#include "../../include/protocol/sender.hpp"
#include "../../include/protocol/crc.hpp"
#include "../../include/drivers/uart.hpp"
#include <cstring>

namespace protocol {

bool Sender::send_transport(const std::uint8_t* data, std::size_t length) {
    if (length > Packet::MaxSize - 7) { // 7 bytes for header+footer
        return false;
    }

    Packet packet;
    packet.length = 7 + length; // FA, l_l, l_h, crc8, FB, data, crc8, FE
    packet.data[0] = 0xFA; // Start byte
    packet.data[1] = static_cast<std::uint8_t>(length & 0xFF); // l_l
    packet.data[2] = static_cast<std::uint8_t>(length >> 8);   // l_h
    packet.data[3] = Crc8::calculate(packet.data.data(), 3);   // Header CRC
    packet.data[4] = 0xFB; // Data start
    std::memcpy(&packet.data[5], data, length); // Transport data
    packet.data[5 + length] = Crc8::calculate(packet.data.data(), 5 + length); // Full CRC
    packet.data[6 + length] = 0xFE; // Stop byte

    return drivers::Uart::instance().send(packet.data.data(), packet.length);
}

} // namespace protocol
