#pragma once
#include <cstddef>
#include <new>


namespace utils {

    template<typename T, std::size_t Size>

    class StaticAllocator {

        public:
        using value_type = T;

        StaticAllocator() = default;

        template<typename U>
        StaticAllocator(const StaticAllocator<U, Size>&) noexcept {}

        T* allocate(std::size_t n) {
            if (m_index + n > Size) {
                return nullptr;
            }
            T* ptr = reinterpret_cast<T*>(&m_buffer[m_index * sizeof(T)]);
            m_index += n;
            return ptr;
        }

        void deallocator(T* p, std::size_t n) noexcept {}

        private:
        alignas(T) std::byte m_buffer[Size * sizeof(T)];
        std::size_t m_index{0};

    };
} // namespace utils
