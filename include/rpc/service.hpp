#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "rpc/types.hpp"
#include "rpc/serializer.hpp"
#include "utils/noncopyable.hpp"
#include "utils/allocator.hpp"

namespace rpc {
    // Форвард декларация
    template<typename... Args>
    class HandlerWrapper;

    class Service : private utils::NonCopyable {
    public:
        // Базовый тип для стирания типа обработчика
        struct HandlerBase {
            virtual ~HandlerBase() = default;
            virtual void invoke(const uint8_t* args, size_t args_len, 
                            uint8_t* result, size_t* result_len) = 0;
        };

        // Шаблонная реализация для конкретных типов
        template<typename Result, typename... Args>
        class HandlerWrapper : public HandlerBase {
        public:
            using HandlerFunc = std::function<Result(Args...)>;
            
            HandlerWrapper(HandlerFunc func) : m_func(std::move(func)) {}
            
            void invoke(const uint8_t* args, size_t args_len,
                    uint8_t* result, size_t* result_len) override {
                // Десериализуем аргументы
                auto args_tuple = Serializer::deserialize_tuple_impl<Args...>(
                    args, std::index_sequence_for<Args...>{});
                
                // Вызываем функцию
                if constexpr (std::is_same_v<Result, void>) {
                    // Для void функций
                    std::apply(m_func, args_tuple);
                    *result_len = 0;
                } else {
                    // Для функций с возвращаемым значением
                    Result func_result = std::apply(m_func, args_tuple);
                    *result_len = sizeof(Result);
                    Serializer::serialize(result, func_result);
                }
            }
            
        private:
            HandlerFunc m_func;
        };

        static Service& instance() {
            static Service instance;
            return instance;
        }

        // Метод регистрации для типобезопасных обработчиков
        template<typename Result, typename... Args>
        bool register_handler(const std::string& name, 
                            std::function<Result(Args...)> handler) {
            auto wrapper = new HandlerWrapper<Result, Args...>(std::move(handler));
            return m_handlers.emplace(name, std::unique_ptr<HandlerBase>(wrapper)).second;
        }

        // Упрощенная регистрация (автоматически выводит типы)
        template<typename Function>
        bool register_handler(const std::string& name, Function&& func) {
            using traits = function_traits<std::decay_t<Function>>;
            return register_handler<typename traits::result_type, typename traits::args_type>(
                name, std::function(std::forward<Function>(func)));
        }

        void process_packet(const protocol::Packet& packet);

    private:
        Service() = default;
        
        // Вспомогательные метафункции для автоматического выведения типов
        template<typename T>
        struct function_traits;
        
        template<typename Result, typename... Args>
        struct function_traits<std::function<Result(Args...)>> {
            using result_type = Result;
            using args_type = std::tuple<Args...>;
        };
        
        std::unordered_map<
            std::string, 
            std::unique_ptr<HandlerBase>,
            std::hash<std::string>,
            std::equal_to<std::string>,
            utils::StaticAllocator<std::pair<const std::string, std::unique_ptr<HandlerBase>>, 32>
        > m_handlers;
    };
} // namespace rpc
