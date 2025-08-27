#pragma once
#include <cstdint>
#include "packet.hpp"
#include "../utils/noncopyable.hpp"
#include "../drivers/uart.hpp"
#include "../rpc/types.hpp"

namespace protocol {

class Parser : private utils::NonCopyable {
public:
    using PacketHandler = void (*)(const Packet&, void*);

    explicit Parser(drivers::Uart& uart, PacketHandler handler, void* user_data = nullptr);
    void process_byte(std::uint8_t byte);
    drivers::Uart& get_uart() { return m_uart; }
    void set_handler(PacketHandler handler, void* user_data = nullptr) {
        m_handler = handler;
        m_user_data = user_data;
    }

private:
    enum class State {
        GetHeader,
        GetLengthLow,
        GetLengthHigh,
        GetHeaderCrc,
        GetDataStart,
        GetData,
        GetFooterCrc,
        GetStopByte
    };

    drivers::Uart& m_uart;
    PacketHandler m_handler;
    void* m_user_data;
    State m_state{State::GetHeader};
    Packet m_packet;
    std::size_t m_index{0};
};

} // namespace protocol
