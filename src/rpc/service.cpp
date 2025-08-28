#include "../../include/rpc/service.hpp"
#include "../../include/protocol/sender.hpp"
#include <cstring>
#include "FreeRTOS.h"
#include "queue.h"

namespace rpc {

/**
 * Обработчик входящих пакетов для сервиса
 * packet Принятый пакет для обработки
 * service_ptr Указатель на экземпляр Service (передается как void*)
 * 
 * Статическая функция-обертка для преобразования типов
 * Преобразует void* в Service* и делегирует обработку
 */

void service_packet_handler(const protocol::Packet& packet, void* service_ptr) {
    auto* service = static_cast<Service*>(service_ptr);
    service->handle_packet(packet);
}

/**
 * Конструктор RPC сервиса
 * parser Ссылка на парсер протокола для приема пакетов
 * 
 * Регистрирует обработчик пакетов в парсере
 * Парсер должен быть инициализирован до создания сервиса
 */

Service::Service(protocol::Parser& parser) : m_parser(parser) {
    m_parser.set_handler([](const protocol::Packet& packet, void* arg) { service_packet_handler(packet, arg); }, this);
}

/**
 * Обработка входящего RPC пакета
 * packet Принятый пакет для обработки
 * 
 * Выполняет следующие действия:
 * 1. Проверяет валидность пакета и наличие имени функции
 * 2. Ищет зарегистрированный обработчик по имени функции
 * 3. Если обработчик не найден - отправляет ошибку
 * 4. Если найден - выполняет его и отправляет результат
 * 
 * Вызывается из контекста парсера - должен быть быстрым
 */

void Service::handle_packet(const protocol::Packet& packet) {
    if (!packet.valid || packet.func_name.empty()) {            // Проверка валидности пакета и наличия имени функции
        return;                                                 // Игнорирование невалидных пакетов
    }

    auto it = m_handlers.find(packet.func_name);                // Поиск зарегистрированного обработчика по имени функции
    if (it == m_handlers.end()) {
        protocol::Packet error_packet;                          // Обработчик не найден - отправка сообщения об ошибке
        error_packet.valid = true;
        error_packet.seq = packet.seq;                          // Сохранение порядкового номера запроса
        error_packet.type = MessageType::Error;
        error_packet.func_name = packet.func_name;
        error_packet.data[0] = static_cast<std::uint8_t>(MessageType::Error);
        error_packet.data_length = 1;

        protocol::Sender sender(m_parser.get_uart());           // Отправка ошибки через транспортный протокол
        sender.send_transport(error_packet.data, error_packet.data_length, error_packet.seq, error_packet.type);
        return;
    }

    // Обработчик найден - выполнение RPC функции
    std::uint8_t response[protocol::Packet::MaxSize];           // Буфер для результата
    std::size_t response_length = 0;
    // Вызов зарегистрированного обработчика
    // Аргументы начинаются после имени функции + 2 байта (тип и seq)
    it->second(packet.data + packet.func_name.size() + 2, packet.data_length - (packet.func_name.size() + 2), response, &response_length);

    protocol::Packet response_packet;                           // Формирование пакета ответа
    response_packet.valid = true;
    response_packet.seq = packet.seq;                           // Тот же порядковый номер, что в запросе
    response_packet.type = MessageType::Response;
    response_packet.func_name = packet.func_name;
    response_packet.data_length = response_length + packet.func_name.size() + 2;

    // Заполнение данных ответа
    response_packet.data[0] = static_cast<std::uint8_t>(MessageType::Response);                     // Тип ответа
    response_packet.data[1] = packet.seq;                                                           // Порядковый номер
    std::memcpy(response_packet.data + 2, packet.func_name.c_str(), packet.func_name.size() + 1);   // Имя функции с null terminator
    std::memcpy(response_packet.data + packet.func_name.size() + 2, response, response_length);     // Результат выполнения

    // Отправка ответа через транспортный протокол
    protocol::Sender sender(m_parser.get_uart());
    sender.send_transport(response_packet.data, response_packet.data_length, response_packet.seq, response_packet.type);
}

/**
 * Основной цикл обработки сервиса
 * В текущей реализации не выполняет действий
 * Может быть использован для фоновой обработки или чистки ресурсов
 */

void Service::process() {
    // Реализация метода process (если требуется)
    // Может использоваться для:
    // - Очистки устаревших соединений
    // - Фоновых задач обслуживания
    // - Мониторинга состояния сервис
}

} // namespace rpc
