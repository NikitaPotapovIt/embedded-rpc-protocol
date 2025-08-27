#include "../../include/protocol/crc.hpp"

namespace protocol {

bool Crc::validate(const Packet& packet) {
    std::uint8_t header[4] = {0xFA, static_cast<std::uint8_t>(packet.length & 0xFF), static_cast<std::uint8_t>(packet.length >> 8), 0xFB};
    std::uint8_t header_crc = calculate(header, 3);
    if (header_crc != packet.header_crc) {
        return false;
    }
    std::uint8_t calculated_crc = calculate(packet.data, packet.data_length);
    return calculated_crc == packet.crc;
}

std::uint8_t Crc::calculate(const std::uint8_t* data, std::size_t length, std::uint8_t crc) {
    constexpr std::uint8_t poly = 0x07; // CRC-8-CCITT polynomial
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc & 0x80) ? (crc << 1) ^ poly : crc << 1;
        }
    }
    return crc;
}

} // namespace protocol
