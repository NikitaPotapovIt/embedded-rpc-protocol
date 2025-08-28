#include "../../include/protocol/parser.hpp"
#include "../../include/protocol/crc.hpp"

namespace protocol {

/**
 * Конструктор парсера протокола
 * uart Ссылка на UART драйвер для приема данных
 * handler Функция-обработчик собранных пакетов
 * user_data Пользовательские данные для callback
 * 
 * Регистрирует callback на прием байтов из UART
 * UART должен быть инициализирован до создания парсера
 */

Parser::Parser(drivers::Uart& uart, PacketHandler handler, void* user_data)
    : m_uart(uart), m_handler(handler), m_user_data(user_data) {
    m_uart.set_rx_callback([](std::uint8_t byte, void* arg) {
        static_cast<Parser*>(arg)->process_byte(byte);
    }, this);
}

/**
 * Обработка очередного принятого байта
 * byte Принятый байт данных
 * 
 * Реализует конечный автомат для разбора пакетов протокола
 * Вызывается из контекста прерывания/задачи UART - должен быть быстрым
 * 
 * Формат пакета:
 * [0xFA][length_low][length_high][header_crc][seq][data...][data_crc][0xFB]
 */

void Parser::process_byte(std::uint8_t byte) {
    switch (m_state) {
        case State::GetHeader:                      // Ожидание стартового байта пакета
            if (byte == 0xFA) {
                m_packet.valid = false;
                m_state = State::GetLengthLow;
            }
            break;

        case State::GetLengthLow:                   // Получение младшего байта длины данных
            m_packet.length = byte;
            m_state = State::GetLengthHigh;
            break;

        case State::GetLengthHigh:                  // Получение старшего байта длины данных
            m_packet.length |= (byte << 8);
            m_state = State::GetHeaderCrc;
            break;

        case State::GetHeaderCrc:                   // Получение CRC заголовка (0xFA + length_low + length_high)
            m_packet.header_crc = byte;
            m_index = 0;
            m_state = State::GetDataStart;
            break;

        case State::GetDataStart:                   // Получение порядкового номера пакета (первый байт данных)
            m_packet.seq = byte;
            m_packet.data[m_index++] = byte;
            m_state = State::GetData;
            break;

        case State::GetData:                        // Накопление полезных данных пакета
            if (m_index < m_packet.length) {
                m_packet.data[m_index++] = byte;
            }
            if (m_index >= m_packet.length) {       // Проверка завершения приема данных
                m_state = State::GetFooterCrc;
            }
            break;

        case State::GetFooterCrc:                   // Получение CRC полезных данных
            m_packet.crc = byte;
            m_state = State::GetStopByte;
            break;

        case State::GetStopByte:                    // Ожидание стопового байта пакета
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
