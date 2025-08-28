#include "../../include/protocol/sender.hpp"
#include "../../include/protocol/crc.hpp"
#include "../../include/drivers/uart.hpp"
#include "../../include/rpc/types.hpp"

namespace protocol {

/**
 * Конструктор отправителя протокола
 * uart Ссылка на UART драйвер для отправки данных
 * 
 * Инициализирует ссылку на UART драйвер
 * UART должен быть инициализирован до использования отправителя
 */

Sender::Sender(drivers::Uart& uart) : m_uart(uart) {}

// Буфер для формирования пакета: заголовок(4) + стартер данных(1) + данные + CRC(1) + стоп(1)
bool Sender::send_transport(const std::uint8_t* data, std::size_t length, std::uint8_t seq, rpc::MessageType type) {
    std::uint8_t packet[Packet::MaxSize + 7];                           // Максимальный размер пакета
    packet[0] = 0xFA;                                                   // Стартовый байт заголовка
    packet[1] = length & 0xFF;                                          // Младший байт длины данных (LSB)
    packet[2] = length >> 8;                                            // Старший байт длины данных (MSB)
    packet[3] = Crc::calculate(packet, 3);                              // CRC заголовка (байты 0-2: 0xFA + l_l + l_h)
    packet[4] = 0xFB;                                                   // Начало данных / Маркер начала полезных данных
    for (std::size_t i = 0; i < length; ++i) {                          // Копирование полезных данных
        packet[5 + i] = data[i];                                        // Полезные данные
    }
    packet[5 + length] = Crc::calculate(data, length);                  // CRC только полезных данных
    packet[6 + length] = 0xFE;                                          // Стоповый байт
    return m_uart.send(packet, length + 7, pdMS_TO_TICKS(100));         // Отправка через UART с таймаутом 100ms
}

} // namespace protocol
