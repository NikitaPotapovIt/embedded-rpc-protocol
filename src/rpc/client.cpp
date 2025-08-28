#include "../../include/rpc/client.hpp"
#include "../../include/rpc/serializer.hpp"
#include "../../include/protocol/sender.hpp"
#include <cstring>

namespace rpc {

/**
 * Конструктор RPC клиента
 * uart Ссылка на UART драйвер для отправки запросов
 * parser Ссылка на парсер для приема ответов
 * 
 * Создает очередь FreeRTOS для приема ответных пакетов
 * Парсер должен быть настроен для отправки пакетов в эту очередь
 */

Client::Client(drivers::Uart& uart, protocol::Parser& parser)
    : m_uart(uart), m_parser(parser), m_sequence(0), m_response_queue(xQueueCreate(10, sizeof(protocol::Packet))) {}

/**
 * Ожидание ответа по порядковому номеру (сырой пакет)
 * response Ссылка для сохранения полученного пакета
 * seq Ожидаемый порядковый номер ответа
 * timeout Таймаут ожидания в тиках FreeRTOS
 * true если ответ получен и номер совпадает, false при таймауте или несовпадении
 * 
 * Блокирует вызывающую задачу до получения ответа
 * Не проверяет валидность пакета - только порядковый номер
 */

bool Client::wait_response(protocol::Packet& response, std::uint8_t seq, TickType_t timeout) {
    protocol::Packet packet;
    if (xQueueReceive(m_response_queue, &packet, timeout) == pdPASS && packet.seq == seq) {
        response = packet;
        return true;
    }
    return false;
}

/**
 * Ожидание ответа по порядковому номеру (распарсенное сообщение)
 * response Ссылка для сохранения распарсенного сообщения
 * seq Ожидаемый порядковый номер ответа
 * timeout Таймаут ожидания в тиках FreeRTOS
 * true если ответ получен и номер совпадает, false при таймауте
 * 
 * Преобразует сырой пакет в структуру Message для удобства использования
 */

bool Client::wait_response(Message& response, std::uint8_t seq, TickType_t timeout) {
    protocol::Packet packet;
    if (wait_response(packet, seq, timeout)) {
        response.type = packet.type;
        response.sequence_number = packet.seq;
        response.function_name = packet.func_name;
        response.arguments = packet.data + packet.func_name.size() + 2; // Skip type and name
        response.arguments_length = packet.data_length - (packet.func_name.size() + 2);
        return true;
    }
    return false;
}

/**
 * Синхронный вызов RPC функции с ожиданием результата
 * Result Тип возвращаемого значения (может быть void)
 * Args Типы аргументов функции
 * function_name Имя вызываемой RPC функции
 * args Аргументы функции
 * Результат выполнения функции или значение по умолчанию при ошибке
 * 
 * Блокирует задачу на время выполнения RPC вызова
 * Для void функций возвращает void, для остальных - значение по умолчанию при ошибке
 */

template<typename Result, typename... Args>
Result Client::call(const std::string& function_name, Args... args) {
    protocol::Packet packet;
    packet.valid = true;
    packet.seq = m_sequence++;
    packet.func_name = function_name;
    packet.type = MessageType::Request;

    std::uint8_t buffer[protocol::Packet::MaxSize];                                 // Формирование бинарного буфера сообщения
    buffer[0] = static_cast<std::uint8_t>(packet.type);                             // Тип сообщения
    buffer[1] = packet.seq;                                                         // Порядковый номер
    std::memcpy(buffer + 2, function_name.c_str(), function_name.size() + 1);       // Копирование имени функции с null terminator
    std::size_t offset = function_name.size() + 2;

    std::tuple<Args...> args_tuple{args...};                                        // Сериализация аргументов функции
    Serializer::serialize_tuple(args_tuple, buffer + offset);
    packet.data_length = offset + Serializer::tuple_size<Args...>();
    std::memcpy(packet.data, buffer, packet.data_length);

    if (send_message(packet)) {                                                     // Отправка запроса и ожидание ответа
        protocol::Packet response;
        if (wait_response(response, packet.seq, pdMS_TO_TICKS(1000))) {             // Ожидание ответа с таймаутом 1 секунда
            if (response.type == MessageType::Response) {
                if constexpr (!std::is_void_v<Result>) {                            // Успешный ответ - десериализация результата
                    return Serializer::deserialize<Result>(response.data + response.func_name.size() + 2);
                }
            } else if (response.type == MessageType::Error) {
                if constexpr (!std::is_void_v<Result>) {                            // Ошибка выполнения - возврат значения по умолчанию
                    return Result{};
                }
            }
        }
    }
    if constexpr (!std::is_void_v<Result>) {                                        // Таймаут или ошибка отправки - возврат значения по умолчанию
        return Result{};
    }
}

/**
 * Асинхронный вызов RPC функции без ожидания результата
 * Args Типы аргументов функции
 * function_name Имя вызываемой RPC функции
 * args Аргументы функции
 * 
 * Отправляет запрос и немедленно возвращает управление
 * Не возвращает результат и не обрабатывает ошибки
 */

template<typename... Args>
void Client::stream_call(const std::string& function_name, Args... args) {
    protocol::Packet packet;
    packet.valid = true;
    packet.seq = m_sequence++;                                                      // Автоинкремент порядкового номера
    packet.func_name = function_name;
    packet.type = MessageType::Stream;                                              // Stream сообщение

    std::uint8_t buffer[protocol::Packet::MaxSize];                                 // Формирование бинарного буфера сообщения
    buffer[0] = static_cast<std::uint8_t>(packet.type);
    buffer[1] = packet.seq;
    std::memcpy(buffer + 2, function_name.c_str(), function_name.size() + 1);       // Включение null terminator
    std::size_t offset = function_name.size() + 2;

    std::tuple<Args...> args_tuple{args...};                                        // Сериализация аргументов функции
    Serializer::serialize_tuple(args_tuple, buffer + offset);
    packet.data_length = offset + Serializer::tuple_size<Args...>();
    std::memcpy(packet.data, buffer, packet.data_length);

    send_message(packet);                                                           // Отправка без ожидания ответа
}

// Отправка сообщения через транспортный протокол / true если отправка успешна, false при ошибке
bool Client::send_message(const protocol::Packet& msg) {
    protocol::Sender sender(m_uart);
    return sender.send_transport(msg.data, msg.data_length, msg.seq, msg.type);
}

} // namespace rpc

// Явное инстанцирование шаблонов
template int32_t rpc::Client::call<int32_t, int32_t, int32_t>(const std::string&, int32_t, int32_t);
template float rpc::Client::call<float>(const std::string&);
template void rpc::Client::call<void, bool>(const std::string&, bool);
template void rpc::Client::stream_call<bool>(const std::string&, bool);
