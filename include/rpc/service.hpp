#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <map>
#include "types.hpp"
#include "../protocol/parser.hpp"
#include "serializer.hpp"

namespace rpc {

/**
 * RPC сервер для регистрации и выполнения удаленных процедур
 * 
 * Принимает входящие RPC запросы, выполняет зарегистрированные handlers
 *       и возвращает результаты обратно через протокол
 * Все handlers должны быть thread-safe (вызываются из контекста парсера)
 */

class Service {
public:
    // Конструктор RPC сервиса
    explicit Service(protocol::Parser& parser);
    // Основной цикл обработки входящих запросов
    void process();
    // Обработчик входящего пакета
    void handle_packet(const protocol::Packet& packet);

    // Регистрация handler'а RPC функции
    template<typename Result, typename... Args>
    bool register_handler(const std::string& name, Result (*func)(Args...)) {
        m_handlers[name] = [func](const std::uint8_t* args, std::size_t args_length, std::uint8_t* res, std::size_t* res_length) {
            // Десериализация аргументов из бинарных данных
            auto args_tuple = Serializer::deserialize_tuple<Args...>(args);
            if constexpr (std::is_void_v<Result>) {
                // Для void-функций: только выполняем, не возвращаем результат
                std::apply(func, args_tuple);
                *res_length = 0;
            } else {
                // Для не-void функций: выполняем и сериализуем результат
                auto result = std::apply(func, args_tuple);
                Serializer::serialize(result, res);
                *res_length = sizeof(result);
            }
        };
        return true;
    }

private:
    protocol::Parser& m_parser; // Парсер для получения входящих пакетов
    /**
     * Map зарегистрированных обработчиков RPC функций
     * Имя RPC функции (std::string)
     * Функтор обработки: void(const uint8_t* args, size_t args_length, 
     *                                uint8_t* res, size_t* res_length)
     */
    std::map<std::string, std::function<void(const std::uint8_t*, std::size_t, std::uint8_t*, std::size_t*)>> m_handlers;
};

} // namespace rpc
