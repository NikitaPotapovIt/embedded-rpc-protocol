#pragma once
#include <cstddef>
#include <cstdint>
#include <new>

namespace utils {

template<typename T, std::size_t Size>
class StaticAllocator {
public:
    StaticAllocator() : m_index(0) {}

    T* allocate(std::size_t n) {
        if (m_index + n > Size) {
            return nullptr;
        }
        T* ptr = reinterpret_cast<T*>(&m_storage[m_index * sizeof(T)]);
        m_index += n;
        return ptr;
    }

    void deallocate(T*, std::size_t) {
        // Статический аллокатор не освобождает память
    }

private:
    alignas(T) std::uint8_t m_storage[Size * sizeof(T)];
    std::size_t m_index;
};

} // namespace utils
