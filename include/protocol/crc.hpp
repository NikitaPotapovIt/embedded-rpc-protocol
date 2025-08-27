#pragma once
#include <cstdint>
#include "packet.hpp"

namespace protocol {

class Crc {
public:
    static bool validate(const Packet& packet);
    static std::uint8_t calculate(const std::uint8_t* data, std::size_t length, std::uint8_t crc = 0x00);
};

} // namespace protocol
