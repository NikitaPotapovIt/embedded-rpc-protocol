#pragma once
#include <cstdint>
#include <string>
#include "../rpc/types.hpp"

namespace protocol {

/**
 * Структура бинарного пакета протокола с заголовком и контрольными суммами
 * 
 * Структура пакета в бинарном виде:
 * [0]    = 0xFA              - стартовый байт (предположительно)
 * [1]    = length_low        - младший байт длины данных
 * [2]    = length_high       - старший байт длины данных  
 * [3]    = header_crc        - CRC заголовка (байты 0-2)
 * [4]    = sequence          - порядковый номер пакета
 * [5..N] = data              - полезные данные
 * [N+1]  = data_crc          - CRC полезных данных
 * 
 * Порядок байтов: little-endian (l_l | l_h << 8)
 */

struct Packet {
    static constexpr std::size_t MaxSize = 64;
    bool valid{false};
    std::uint16_t length{0}; // 16-bit length (l_l | l_h << 8)
    std::uint8_t header_crc{0}; // CRC of header (0xFA, l_l, l_h)
    std::uint8_t seq{0};
    std::uint8_t data[MaxSize]{};
    std::size_t data_length{0};
    std::uint8_t crc{0}; // CRC of data
    std::string func_name;
    rpc::MessageType type;
};
} // namespace protocol
