#pragma once
#include <cstddef>
#include <cstdint>
#include <new>

namespace utils {

/**
 * Статический аллокатор с pre-allocated памятью на стеке
 * T Тип объектов для аллокации
 * Size Максимальное количество объектов типа T
 * 
 * Особенности:
 * - Вся память выделяется на стеке при создании аллокатора
 * - Не освобождает память (deallocate no-op)
 * - Быстрый (нет системных вызовов)
 * - Детерминированное время выполнения
 * 
 * Не подходит для scenarios где требуется освобождение памяти
 * Временные объекты, буферы фиксированного размера, embedded systems
 */

template<typename T, std::size_t Size>
class StaticAllocator {
public:
    // Конструктор инициализирует аллокатор
    StaticAllocator() : m_index(0) {}

    // Выделение памяти для n объектов типа T
    T* allocate(std::size_t n) {
        if (m_index + n > Size) {
            return nullptr; // Переполнение буфера
        }
        T* ptr = reinterpret_cast<T*>(&m_storage[m_index * sizeof(T)]);
        m_index += n;
        return ptr;
    }

    // Освобождение памяти
    void deallocate(T*, std::size_t) {
        // Статический аллокатор не освобождает память
    }

private:
    alignas(T) std::uint8_t m_storage[Size * sizeof(T)];    // Статический буфер памяти
    std::size_t m_index;                                    // Текущий индекс аллокации
};

} // namespace utils
