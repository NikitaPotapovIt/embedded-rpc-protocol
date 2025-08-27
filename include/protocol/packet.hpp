#pragma once
#include <cstdint>
#include <string>

namespace protocol {
struct Packet {
    static constexpr std::size_t MaxSize = 64;
    bool valid{false};
    std::uint16_t length{0}; // 16-bit length (l_l | l_h << 8)
    std::uint8_t header_crc{0}; // CRC of header (0xFA, l_l, l_h)
    std::uint8_t seq{0};
    std::uint8_t data[MaxSize]{};
    std::size_t data_length{0};
    std::uint8_t crc{0}; // CRC of data
    std::string func_name;
    rpc::MessageType type;
};
} // namespace protocol
