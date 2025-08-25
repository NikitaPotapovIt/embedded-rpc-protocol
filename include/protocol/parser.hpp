#pragma once
#include <cstdint>
#include "./packet.hpp"
#include "../utils/noncopyable.hpp"

namespace protocol {

    class Parser : private utils::NonCopyable {
        using PacketHandler = void(*)(const Packet&);

        explicit Parser(PacketHandler handler);
        void process_byte(std::uint8_t byte);
        void reset();

        private:
        enum class State {
            WaitSync,       // Ожидание стартового бита 0xFA
            GetLengthL,     // Получение младшего байта длины
            GetLengthH,     // Получение старшего байта доины
            GetHeaderCrc,   // Получение CRC заголовка
            WaitDataSync,   // Ожидание 0xFB
            GetData,        // Получение полезной нагрузки
            GetFooterCrc,   // Получение Crc данных
            GetStopByte,    // Ожидание стопового байта 0xFE
        };

        State m_state{State::WaitSync};
        Packet m_current_packet;
        PacketHandler m_handler;
        std::size_t m_bytes_remaining{0};
        std::uint8_t m_calculated_crc{0};

        void validate_packet();
    };
} // namespace protocol
