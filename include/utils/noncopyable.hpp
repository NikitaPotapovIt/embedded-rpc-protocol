#pragma once

namespace utils {

/**
 * Базовый класс для запрета копирования и присваивания
 * 
 * Наследование от этого класса автоматически делает дочерний класс
 *       non-copyable и non-assignable
 * Реализует идиому "Explicitly deleted copy operations"
 */

class NonCopyable {
protected:
    NonCopyable() = default;                                    // Защищенный чтобы предотвратить создание экземпляров NonCopyable
    ~NonCopyable() = default;                                   // Виртуальный не требуется - класс не предназначен для полиморфного использования

    NonCopyable(const NonCopyable&) = delete;                   // Явно запрещает копирование объектов
    NonCopyable& operator=(const NonCopyable&) = delete;        // Явно запрещает присваивание объектов
};

} // namespace utils
