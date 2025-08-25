#include "../../include/protocol/parser.hpp"
#include "../../include/protocol/crc.hpp"
#include <cstring>

namespace protocol {

    constexpr uint8_t SYNC_BYTE = 0xFA;
    constexpr uint8_t DATA_SYNC_BYTE = 0xFB;
    constexpr uint8_t STOP_BYTE = 0xFE;

    Parser::Parser(PacketHandler handler) : m_handler(handler) {
        reset();
    }

    void Parser::process_byte(uint8_t byte) {
        switch (m_state) {
            case State::WaitSync:
                if (byte == SYNC_BYTE) {
                    m_state == State::GetLengthL;
                    reset();
                    m_current_packet.data[0] = byte; // Save 1 byte
                    m_current_packet.length = 1;
                }
                break;
            case State::GetLengthL:
                m_current_packet.data[1] = byte;
                m_current_packet.length++;
                m_bytes_remaining = byte;
                m_state = State::GetLengthH;
                break;

            case State::GetLengthH:
                m_current_packet.data[2] = byte;
                m_current_packet.length++;
                // Формировка длины
                m_bytes_remaining |= (static_cast<uint16_t>(byte) << 8);
                m_state = State::GetHeaderCrc;
                break;

            case State::GetHeaderCrc: {
                m_current_packet.data[3] = byte;
                m_current_packet.length++;
                // Проверка CRC заголовка
                uint8_t expected_crc = Crc8::calculate(m_current_packet.data.data(), 3);
                if (byte == expected_crc) {
                    m_state = State::WaitDataSync;
                } else {
                    reset(); // CRC fail - reboot
                }
                break;
            }

            case State::WaitDataSync:
                if (byte == DATA_SYNC_BYTE) {
                    m_current_packet.data[4] = byte;
                    m_current_packet.length++;
                    m_state = State::GetData;
                } else {
                    reset(); // fail byte - reset
                }
                break;
            
            case State::GetData:
                m_current_packet.data[m_current_packet.length++] = byte;
                if (--m_bytes_remaining == 0) {
                    m_state = State::GetFooterCrc;
                }
                break;
            
            case State::GetFooterCrc:
                // TODO: Check CRC data
                m_state = State::GetStopByte;
                break;

            case State::GetStopByte:
                if (byte == STOP_BYTE) {
                    m_current_packet.valid = true;
                    if (m_handler) {
                        m_handler(m_current_packet); // Отправка готового пакета
                    }
                }
                reset(); // Сброс после стопового байта (всегда)
                break;
        }
    }

    // Сброс парсера в исходное состояние.

    void Parser::reset() {
        m_state = State::WaitSync;
        m_current_packet.length = 0;
        m_current_packet.valid = false;
        m_bytes_remaining = 0;
        m_calculated_crc = 0;
    }
} // namespace protocol
