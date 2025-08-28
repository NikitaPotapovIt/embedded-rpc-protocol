#include "../../include/protocol/crc.hpp"

namespace protocol {

/**
 * Проверка целостности пакета с помощью CRC
 * packet Пакет для проверки
 * true если оба CRC (заголовка и данных) корректны, false если есть ошибки
 * 
 * Проверяет два CRC: заголовка и полезных данных
 * Возвращает false при любой ошибке CRC
 * 
 * Алгоритм проверки:
 * 1. Вычисляет CRC заголовка (байты: 0xFA, length_low, length_high)
 * 2. Сравнивает с packet.header_crc
 * 3. Вычисляет CRC данных (packet.data[0..data_length-1])
 * 4. Сравнивает с packet.crc
 */

bool Crc::validate(const Packet& packet) {
    std::uint8_t header[4] = {0xFA,                                                             // Стартовый байт
        static_cast<std::uint8_t>(packet.length & 0xFF),                                        // Младший байт длины
        static_cast<std::uint8_t>(packet.length >> 8),                                          // Старший байт длины
        0xFB};                                                                                  // Фиктивный байт
    std::uint8_t header_crc = calculate(header, 3);                                             // Расчет CRC заголовка (только первые 3 байта)
    if (header_crc != packet.header_crc) {                                                      // Проверка CRC заголовка
        return false;                                                                           // Ошибка CRC заголовка
    }
    std::uint8_t calculated_crc = calculate(packet.data, packet.data_length);                   // Расчет CRC полезных данных
    return calculated_crc == packet.crc;                                                        // Проверка CRC данных
}

std::uint8_t Crc::calculate(const std::uint8_t* data, std::size_t length, std::uint8_t crc) {   // Вычисление CRC-8 для данных
    constexpr std::uint8_t poly = 0x07;                                                         // CRC-8-CCITT polynomial: x^8 + x^2 + x^1 + 1
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= data[i];                                                                         // XOR с текущим байтом данных
        for (int j = 0; j < 8; ++j) {                                                           // Обработка каждого бита (8 бит на байт)
            crc = (crc & 0x80) ? (crc << 1) ^ poly : crc << 1;                                  // Если старший бит установлен - сдвигаем и XOR с полиномом
        }
    }
    return crc;
}

} // namespace protocol
