#pragma once
#include <cstdint>
#include "packet.hpp"

namespace protocol {

/**
 * Калькулятор и валидатор контрольной суммы для протокольных пакетов
 * 
 * Использует алгоритм CRC-8 с определенными параметрами
 *       (уточнить полином, начальное значение и финальный XOR если известно)
 */

class Crc {
public:
    static bool validate(const Packet& packet);
     /**
     * Вычисляет контрольную сумму для данных
     * data Указатель на данные для расчета
     * length Длина данных в байтах
     * crc Начальное значение CRC (по умолчанию 0x00)
     * return Вычисленное значение CRC-8
     * 
     * std::uint8_t crc = Crc::calculate(data, length - 1); // Все кроме последнего байта (CRC)
     * bool valid = (crc == data[length - 1]); // Сравнение с последним байтом
     */
    static std::uint8_t calculate(const std::uint8_t* data, std::size_t length, std::uint8_t crc = 0x00);
};

} // namespace protocol
