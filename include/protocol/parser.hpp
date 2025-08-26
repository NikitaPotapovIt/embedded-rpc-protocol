#pragma once
#include <cstdint>
#include "./packet.hpp"
#include "../utils/noncopyable.hpp"

namespace protocol {

class Parser : private utils::NonCopyable {
public:
    using PacketHandler = void(*)(const Packet&);

    explicit Parser(PacketHandler handler);
    void process_byte(std::uint8_t byte);
    void reset();

private:
    enum class State {
        WaitSync,       // Waiting for 0xFA
        GetLengthL,     // Low byte of length
        GetLengthH,     // High byte of length
        GetHeaderCrc,   // Header CRC
        WaitDataSync,   // Waiting for 0xFB
        GetData,        // Payload
        GetFooterCrc,   // Full packet CRC
        GetStopByte     // Waiting for 0xFE
    };

    State m_state{State::WaitSync};
    Packet m_current_packet;
    PacketHandler m_handler;
    std::size_t m_bytes_remaining{0};
    std::uint8_t m_calculated_crc{0};
};

} // namespace protocol
