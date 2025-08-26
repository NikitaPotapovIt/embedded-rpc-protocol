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
                    m_state = State::GetLengthL;
                    reset();
                    m_current_packet.data[0] = byte; // Сохраняем стартовый байт
                    m_current_packet.length = 1;
                }
                break;
                
            case State::GetLengthL:
                m_current_packet.data[1] = byte;
                m_bytes_remaining = byte; // Пока только младший байт
                m_state = State::GetLengthH;
                m_current_packet.length++;
                break;
                
            case State::GetLengthH:
                m_current_packet.data[2] = byte;
                m_bytes_remaining |= (byte << 8); // Объединяем с старшим байтом
                m_state = State::GetHeaderCrc;
                m_current_packet.length++;
                break;
                
            case State::GetHeaderCrc:
                m_current_packet.data[3] = byte;
                // Проверяем CRC заголовка (первые 4 байта)
                if (byte == Crc8::calculate(m_current_packet.data.data(), 3)) {
                    m_state = State::WaitDataSync;
                } else {
                    reset(); // CRC не совпал - начинаем сначала
                }
                m_current_packet.length++;
                break;
                
            case State::WaitDataSync:
                if (byte == 0xFB) {
                    m_state = State::GetData;
                    m_current_packet.data[4] = byte;
                    m_current_packet.length++;
                } else {
                    reset(); // Ожидали 0xFB, но получили другой байт
                }
                break;
                
            case State::GetData:
                if (m_bytes_remaining > 0) {
                    m_current_packet.data[m_current_packet.length++] = byte;
                    m_calculated_crc = Crc8::calculate(&byte, 1, m_calculated_crc);
                    m_bytes_remaining--;
                    
                    if (m_bytes_remaining == 0) {
                        m_state = State::GetFooterCrc;
                    }
                }
                break;
                
            case State::GetFooterCrc:
                // Сравниваем расчетный CRC с полученным
                if (byte == m_calculated_crc) {
                    m_state = State::GetStopByte;
                } else {
                    reset(); // CRC данных не совпал
                }
                break;
                
            case State::GetStopByte:
                if (byte == 0xFE) {
                    m_current_packet.valid = true;
                    if (m_handler) {
                        m_handler(m_current_packet); // Вызываем обработчик
                    }
                }
                reset(); // Всегда сбрасываемся после стопового байта
                break;
        }
    }

    void Parser::reset() {
        m_state = State::WaitSync;
        m_current_packet.length = 0;
        m_current_packet.valid = false;
        m_bytes_remaining = 0;
        m_calculated_crc = 0;
        std::memset(m_current_packet.data.data(), 0, m_current_packet.data.size());
    }
} // namespace protocol
