#include "../../include/protocol/parser.hpp"
#include "../../include/protocol/crc.hpp"

namespace protocol {

Parser::Parser(drivers::Uart& uart, PacketHandler handler, void* user_data)
    : m_uart(uart), m_handler(handler), m_user_data(user_data) {
    m_uart.set_rx_callback([](std::uint8_t byte, void* arg) {
        static_cast<Parser*>(arg)->process_byte(byte);
    }, this);
}

void Parser::process_byte(std::uint8_t byte) {
    switch (m_state) {
        case State::GetHeader:
            if (byte == 0xFA) {
                m_packet.valid = false;
                m_state = State::GetLengthLow;
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
            m_index = 0;
            m_state = State::GetDataStart;
            break;

        case State::GetDataStart:
            m_packet.seq = byte;
            m_packet.data[m_index++] = byte;
            m_state = State::GetData;
            break;

        case State::GetData:
            if (m_index < m_packet.length) {
                m_packet.data[m_index++] = byte;
            }
            if (m_index >= m_packet.length) {
                m_state = State::GetFooterCrc;
            }
            break;

        case State::GetFooterCrc:
            m_packet.crc = byte;
            m_state = State::GetStopByte;
            break;

        case State::GetStopByte:
            if (byte == 0xFB) {
                m_packet.valid = Crc::validate(m_packet);
                m_packet.data_length = m_index;
                if (m_packet.valid && m_handler) {
                    m_handler(m_packet, m_user_data);
                }
            }
            m_state = State::GetHeader;
            break;
    }
}

} // namespace protocol
