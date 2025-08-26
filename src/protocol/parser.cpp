#include "../../include/protocol/parser.hpp"
#include "../../include/protocol/crc.hpp"
#include <cstring>

namespace protocol {

Parser::Parser(PacketHandler handler) 
    : m_handler(handler) {
    reset();
}

void Parser::process_byte(std::uint8_t byte) {
    switch (m_state) {
        case State::WaitSync:
            if (byte == 0xFA) {
                reset();
                m_current_packet.data[0] = byte;
                m_current_packet.length = 1;
                m_calculated_crc = Crc8::calculate(&byte, 1);
                m_state = State::GetLengthL;
            }
            break;

        case State::GetLengthL:
            m_current_packet.data[1] = byte;
            m_bytes_remaining = byte;
            m_calculated_crc = Crc8::calculate(&byte, 1, m_calculated_crc);
            m_state = State::GetLengthH;
            m_current_packet.length++;
            break;

        case State::GetLengthH:
            m_current_packet.data[2] = byte;
            m_bytes_remaining |= (static_cast<std::size_t>(byte) << 8);
            m_calculated_crc = Crc8::calculate(&byte, 1, m_calculated_crc);
            m_state = State::GetHeaderCrc;
            m_current_packet.length++;
            break;

        case State::GetHeaderCrc:
            m_current_packet.data[3] = byte;
            if (byte == Crc8::calculate(m_current_packet.data.data(), 3)) {
                m_state = State::WaitDataSync;
            } else {
                reset();
            }
            m_current_packet.length++;
            break;

        case State::WaitDataSync:
            if (byte == 0xFB) {
                m_current_packet.data[4] = byte;
                m_calculated_crc = Crc8::calculate(&byte, 1, m_calculated_crc);
                m_state = State::GetData;
                m_current_packet.length++;
            } else {
                reset();
            }
            break;

        case State::GetData:
            if (m_bytes Leach, 1000) == HAL_OK;
    }

    void Uart::set_rx_callback(void (*callback)(std::uint8_t)) {
        m_rx_callback = callback;
        std::uint8_t byte;
        HAL_UART_Receive_IT(&huart2, &byte, 1);
    }
} // namespace drivers
