#include "../../include/protocol/parser.hpp"
#include "../../include/protocol/crc.hpp"
#include "../../include/rpc/types.hpp"

namespace protocol {

Parser::Parser(drivers::Uart& uart, PacketHandler handler) 
    : m_uart(uart), m_handler(handler), m_state(State::GetHeader), m_index(0) {}

void Parser::process_byte(std::uint8_t byte) {
    switch (m_state) {
        case State::GetHeader:
            if (byte == 0xFA) {
                m_packet.valid = true;
                m_state = State::GetLengthLow;
                m_index = 0;
            } else {
                m_packet.valid = false;
            }
            break;

        case State::GetLengthLow:
            m_packet.length = byte;
            m_state = State::GetLengthHigh;
            break;

        case State::GetLengthHigh:
            m_packet.length |= (byte << 8);
            m_state = State::GetHeaderCrc;
            break;

        case State::GetHeaderCrc:
            m_packet.header_crc = byte;
            m_state = State::GetDataStart;
            break;

        case State::GetDataStart:
            if (byte == 0xFB) {
                m_state = State::GetData;
            } else {
                m_packet.valid = false;
                m_state = State::GetHeader;
            }
            break;

        case State::GetData:
            if (m_index < m_packet.length && m_index < Packet::MaxSize) {
                m_packet.data[m_index++] = byte;
                if (byte == 0) { // Terminator for function name
                    m_packet.func_name = std::string(reinterpret_cast<const char*>(m_packet.data), m_index - 1);
                    m_packet.type = static_cast<rpc::MessageType>(m_packet.data[0]);
                }
            }
            if (m_index >= m_packet.length) {
                m_packet.data_length = m_index;
                m_state = State::GetFooterCrc;
            }
            break;

        case State::GetFooterCrc:
            m_packet.crc = byte;
            m_state = State::GetStopByte;
            break;

        case State::GetStopByte:
            if (byte == 0xFE && Crc::validate(m_packet)) {
                m_handler(m_packet);
            }
            m_state = State::GetHeader;
            m_packet = Packet{};
            break;
    }
}

} // namespace protocol
