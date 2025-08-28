#pragma once
#include <cstdint>
#include "packet.hpp"
#include "../drivers/uart.hpp"
#include "../utils/noncopyable.hpp"
#include "../rpc/types.hpp"

namespace protocol {

/**
 * Класс для формирования и отправки бинарных пакетов протокола через UART
 * 
 * Формирует пакеты в формате:
 *       [0xFA][length_low][length_high][header_crc][seq][data...][data_crc][0xFE]
 * 
 * Автоматически рассчитывает CRC заголовка и данных,
 *          управляет порядковыми номерами пакетов
 */

class Sender : private utils::NonCopyable {
public:
    // Конструктор отправителя
    explicit Sender(drivers::Uart& uart);
    // Отправка данных через транспортный протокол
    bool send_transport(const std::uint8_t* data, std::size_t length, std::uint8_t seq, rpc::MessageType type);

private:
    drivers::Uart& m_uart;          // Ссылка на драйвер UART для отправки данных
    std::uint8_t m_sequence{0};     // Текущий порядковый номер пакета
};

} // namespace protocol
