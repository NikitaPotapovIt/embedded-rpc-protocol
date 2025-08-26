#pragma once
#include <cstdint>

namespace protocol {

class Crc8 {
public:
    static std::uint8_t calculate(const std::uint8_t* data, std::size_t length, std::uint8_t crc = 0x00);
};

} // namespace protocol
