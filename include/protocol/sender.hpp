#pragma once
#include <cstdint>
#include "packet.hpp"
#include "../drivers/uart.hpp"
#include "../utils/noncopyable.hpp"
#include "../rpc/types.hpp"

namespace protocol {

class Sender : private utils::NonCopyable {
public:
    explicit Sender(drivers::Uart& uart);
    bool send_transport(const std::uint8_t* data, std::size_t length, std::uint8_t seq, rpc::MessageType type);

private:
    drivers::Uart& m_uart;
    std::uint8_t m_sequence{0};
};

} // namespace protocol
