#pragma once
#include <cstdint>
#include <string>

namespace rpc {

enum class MessageType : std::uint8_t {
    Request = 0x0B,
    Stream = 0x0C,
    Response = 0x16,
    Error = 0x21
};

struct Message {
    MessageType type;
    std::uint8_t sequence_number;
    std::string function_name;
    const std::uint8_t* arguments;
    std::size_t arguments_length;
};

} // namespace rpc
