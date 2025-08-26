#include "../../include/protocol/crc.hpp"

namespace protocol {

std::uint8_t Crc8::calculate(const std::uint8_t* data, std::size_t length, std::uint8_t crc) {
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
