#pragma once
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <cstring>

namespace rpc {

/**
 * Статический класс для бинарной сериализации и десериализации данных
 * 
 * Использует простое копирование памяти (memcpy) для POD-типов
 * Работает только с тривиально копируемыми типами (POD)
 * Поддерживает сериализацию кортежей с автоматическим вычислением смещений
 */

class Serializer {
public:
    // Сериализация значения в буфер
    template<typename T>
    static void serialize(const T& value, std::uint8_t* buffer) {
        static_assert(!std::is_void_v<T>, "Cannot serialize void type");
        std::memcpy(buffer, &value, sizeof(T));
    }

    // Десериализация значения из буфера
    template<typename T>
    static T deserialize(const std::uint8_t* buffer) {
        static_assert(!std::is_void_v<T>, "Cannot deserialize void type");
        T value;
        std::memcpy(&value, buffer, sizeof(T));
        return value;
    }

    // Сериализация кортежа в буфер
    template<typename... Args>
    static void serialize_tuple(const std::tuple<Args...>& tuple, std::uint8_t* buffer) {
        serialize_tuple_impl(tuple, buffer, std::index_sequence_for<Args...>{});
    }

    // Десериализация кортежа из буфера
    template<typename... Args>
    static std::tuple<Args...> deserialize_tuple(const std::uint8_t* buffer) {
        return deserialize_tuple_impl<Args...>(buffer, std::index_sequence_for<Args...>{});
    }

    // Вычисление размера буфера для кортежа
    template<typename... Args>
    static constexpr std::size_t tuple_size() {
        return (sizeof(Args) + ... + 0);
    }

private:
    // Вспомогательная функция для сериализации кортежа
    template<typename... Args, std::size_t... I>
    static void serialize_tuple_impl(const std::tuple<Args...>& tuple, std::uint8_t* buffer, std::index_sequence<I...>) {
        (serialize(std::get<I>(tuple), buffer + offset_of<I, Args...>()), ...);
    }

    // Вспомогательная функция для десериализации кортежа
    template<typename... Args, std::size_t... I>
    static std::tuple<Args...> deserialize_tuple_impl(const std::uint8_t* buffer, std::index_sequence<I...>) {
        return std::tuple<Args...>{deserialize<std::tuple_element_t<I, std::tuple<Args...>>>(buffer + offset_of<I, Args...>())...};
    }

    // Вычисление смещения элемента в сериализованном кортеже
    template<std::size_t N, typename... Args>
    static constexpr std::size_t offset_of() {
        if constexpr (N == 0) {
            return 0;
        } else {
            return offset_of<N - 1, Args...>() + sizeof(std::tuple_element_t<N - 1, std::tuple<Args...>>);
        }
    }
};

} // namespace rpc
