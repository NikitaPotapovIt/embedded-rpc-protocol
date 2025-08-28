#pragma once
#include <cstdint>
#include "packet.hpp"
#include "../utils/noncopyable.hpp"
#include "../drivers/uart.hpp"
#include "../rpc/types.hpp"

namespace protocol {

 /**
 * Конечный автомат для разбора бинарных пакетов протокола из UART
 * 
 * Реализует парсинг пакетов в формате:
 *       [0xFA][length_low][length_high][header_crc][seq][data...][data_crc][0xFE]
 * 
 * Не потокобезопасен - должен вызываться из одного контекста
 */

class Parser : private utils::NonCopyable {
public:
    // Тип callback-функции для обработки распарсенных пакетов
    using PacketHandler = void (*)(const Packet&, void*);

    // Конструктор парсера
    explicit Parser(drivers::Uart& uart, PacketHandler handler, void* user_data = nullptr);
    void process_byte(std::uint8_t byte);
    // Возвращает ссылку на UART драйвер
    drivers::Uart& get_uart() { return m_uart; }
    // Устанавливает обработчик пакетов
    void set_handler(PacketHandler handler, void* user_data = nullptr) {
        m_handler = handler;
        m_user_data = user_data;
    }

private:
    enum class State {
        GetHeader,          // Ожидание стартового байта 0xFA
        GetLengthLow,       // Получение младшего байта
        GetLengthHigh,      // Получение старшего байта
        GetHeaderCrc,       // Получение CRC заголовка
        GetDataStart,       // Получение стартовой нагрузки
        GetData,            // Получение полезной нагрузки
        GetFooterCrc,       // Получение CRC данных
        GetStopByte         // Ожидание стопового байта 0xFE
    };

    drivers::Uart& m_uart;              // Ссылка на драйвер UART для приема данных
    PacketHandler m_handler;            // Callback для обработки готовых пакетов
    void* m_user_data;                  // Пользовательские данные для callback
    State m_state{State::GetHeader};    // Текущее состояние парсера
    Packet m_packet;                    // Текущий обрабатываемый пакет
    std::size_t m_index{0};             // Индекс для накопления данных
};

} // namespace protocol
