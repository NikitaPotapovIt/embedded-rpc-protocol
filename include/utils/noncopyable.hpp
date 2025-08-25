#pragma once

namespace utils {

    class NonCopyable {

    protected:
        NonCopyable() = default;
        ~NonCopyable() = default;
    
    NonCopyable(const NonCopyable&) = delete; // Удаляем конструктор копирования

    NonCopyable& operator=(const NonCopyable) = delete; // Удаляем оператор присваивания
    
    };
    
} // namespace utils
