#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <tuple>

namespace rpc {

    class Serializer {
    public:
        // Сериализация одиночного значения
        template<typename T>
        static void serialize(uint8_t* buffer, T value) {
            static_assert(std::is_trivially_copyable_v<T>, 
                        "Type must be trivially copyable for serialization");
            std::memcpy(buffer, &value, sizeof(T));
        }

        // Десериализация одиночного значения
        template<typename T>
        static T deserialize(const uint8_t* buffer) {
            static_assert(std::is_trivially_copyable_v<T>,
                        "Type must be trivially copyable for deserialization");
            T value;
            std::memcpy(&value, buffer, sizeof(T));
            return value;
        }

        // Рекурсивная сериализация кортежа (для нескольких аргументов)
        template<typename... Args, size_t... I>
        static void serialize_tuple_impl(const std::tuple<Args...>& tuple, 
                                    uint8_t* buffer, 
                                    std::index_sequence<I...>) {
            (serialize(buffer + offset_of<I>(), std::get<I>(tuple)), ...);
        }

        // Рекурсивная десериализация кортежа
        template<typename... Args, size_t... I>
        static std::tuple<Args...> deserialize_tuple_impl(const uint8_t* buffer,
                                                        std::index_sequence<I...>) {
            return std::make_tuple(deserialize<Args>(buffer + offset_of<I>())...);
        }

    private:
        // Вычисление смещения для N-го элемента в буфере
        template<size_t N>
        static constexpr size_t offset_of() {
            return (offset_of<N-1>() + sizeof(std::tuple_element_t<N-1, std::tuple<Args...>>));
        }
        
        template<>static constexpr size_t offset_of<0>() { return 0; }
    };
    
} // namespace rpc
