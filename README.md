# Embedded RPC over UART на C++

**Язык:** C++17/20 (с ограничениями для embedded)
**Платформа:** Любой микроконтроллер (ARM Cortex-M, ESP32, RISC-V)
**Интерфейс:** UART (Universal Asynchronous Receiver-Transmitter)
**RTOS:** FreeRTOS (с C++ обертками)

## Оглавление

1.  [Архитектурный обзор](#архитектурный-обзор)
2.  [Ключевые возможности](#ключевые-возможности)
3.  [Быстрый старт](#быстрый-старт)
4.  [Структура проекта](#структура-проекта)
5.  [Детали реализации](#детали-реализации)
6.  [Анализ и улучшения протокола](#анализ-и-улучшения-протокола)

## Архитектурный обзор

Проект реализует многоуровневый RPC (Remote Procedure Call) протокол, использующий современные возможности C++ для создания типобезопасного и выразительного API. Архитектура спроектирована с учетом ограничений embedded-систем.

```
┌─────────────────┐    Serialized Data    ┌─────────────────┐
│   Host/Client   │ ◄───────────────────► │  Device/Server  │
└─────────────────┘                       └─────────────────┘
            │                                      │
            │ UART Stream                          │ UART Stream
            ▼                                      ▼
┌─────────────────────────┐              ┌─────────────────────────┐
│ Application Layer       │              │ Application Layer       │
│  - RPC Client API       │              │  - RPC Service Registry │
│  - Type-Safe Calls      │              │  - Handler Management   │
├─────────────────────────┤              ├─────────────────────────┤
│ Transport Layer         │              │ Transport Layer         │
│  - Message Serialization│              │  - Message Parsing      │
│  - Timeout Management   │              │  - Handler Dispatch     │
├─────────────────────────┤              ├─────────────────────────┤
│ Data Link Layer         │              │ Data Link Layer         │
│  - Packet Framing       │              │  - Packet Deframing     │
│  - CRC Calculation      │              │  - CRC Validation       │
├─────────────────────────┤              ├─────────────────────────┤
│ Physical Layer          │              │ Physical Layer          │
│  - UART Driver          │              │  - UART Driver          │
└─────────────────────────┘              └─────────────────────────┘
```

## Ключевые возможности

- **Type-Safe RPC**: Вызов удаленных функций с проверкой типов на этапе компиляции
- **Zero-Cost Abstractions**: Минимизация накладных расходов через шаблонное метапрограммирование
- **Memory Safety**: Исключение небезопасных преобразований типов и ручной работы с памятью
- **Modular Design**: Четкое разделение на уровни с легкой заменяемостью компонентов
- **Real-Time Ready**: Детерминированное выполнение с предсказуемым потреблением памяти

## Быстрый старт

### 1. Регистрация RPC обработчика

```cpp
// Объявляем обработчик с типобезопасной сигнатурой
auto temperature_handler = [](SensorId id, Precision prec) -> Temperature {
    return read_temperature(id, prec);
};

// Регистрируем в сервисе
RPCService::instance().register_handler(
    "get_temperature", // имя метода
    temperature_handler // любой callable объект
);
```

### 2. Вызов удаленной процедуры

```cpp
// Клиентский код
auto result = RPCClient::call<Temperature>(
    "get_temperature", // имя метода
    SensorId::CORE,    // аргумент 1
    Precision::HIGH    // аргумент 2
);

if (result) {
    Temperature temp = *result;
    // работаем с результатом
}
```

### 3. Конфигурация оборудования

```cpp
int main() {
    // Инициализация HAL
    HAL_Init();
    SystemClock_Config();
    
    // Настройка UART
    UART::Config uart_cfg {
        .baudrate = 115200,
        .word_length = DataLength::B8,
        .stop_bits = StopBits::ONE,
        .parity = Parity::NONE
    };
    auto& uart = UART::get_instance(UART_ID);
    uart.init(uart_cfg);
    
    // Создание задач FreeRTOS
    xTaskCreate(
        task_uart_receive,  "UART_Rx",  512, nullptr, 4, nullptr
    );
    xTaskCreate(
        task_process_rpc,   "RPC_Proc", 1024, nullptr, 3, nullptr
    );
    
    vTaskStartScheduler();
    while (1);
}
```

## Структура проекта

```
include/
├── rpc/
│   ├── service.hpp      # RPC сервис (синглтон)
│   ├── client.hpp       # RPC клиент
│   ├── serializer.hpp   # Сериализация типов
│   └── types.hpp        # Basic types
├── protocol/
│   ├── packet.hpp       # Пакет канального уровня
│   ├── parser.hpp       # Конечный автомат парсера
│   └── crc.hpp          # CRC calculator
├── drivers/
│   ├── uart.hpp         # Абстракция UART
│   └── stm32f4xx_uart.hpp # Специфичная реализация
└── utils/
    ├── queue.hpp        # Очередь сообщений
    ├── allocator.hpp    # Custom allocator
    └── noncopyable.hpp  # Mixin class

src/
├── rpc/
│   ├── service.cpp      # Реализация RPC сервиса
│   └── serializer.cpp   # Специализации сериализации
├── protocol/
│   ├── parser.cpp       # Реализация парсера
│   └── crc.cpp          # Реализация CRC
├── drivers/
│   └── uart.cpp         # Драйвер UART
└── main.cpp             # Точка входа
```

## Детали реализации

### Безопасная сериализация

```cpp
// Специализация сериализатора для пользовательских типов
template<>
struct Serializer<Temperature> {
    static std::array<uint8_t, sizeof(Temperature)> serialize(const Temperature& temp) {
        std::array<uint8_t, sizeof(Temperature)> data;
        std::memcpy(data.data(), &temp, sizeof(Temperature));
        return data;
    }
    
    static Temperature deserialize(const std::span<const uint8_t>& data) {
        Temperature temp;
        std::memcpy(&temp, data.data(), sizeof(Temperature));
        return temp;
    }
};
```

### Статическая диспетчеризация

```cpp
// Вызов обработчика с автоматической десериализацией
template<typename Handler, typename... Args>
auto invoke_handler(Handler&& handler, const Packet& packet) {
    auto args_tuple = deserialize_args<std::tuple<Args...>>(packet.data());
    return std::apply(handler, args_tuple);
}
```

### Управление памятью

```cpp
// Static allocator для исключения динамической памяти
template<typename T, size_t Size>
class StaticAllocator {
public:
    using value_type = T;
    
    T* allocate(size_t n) {
        if (n > (Size - m_index)) {
            return nullptr; // Better error handling
        }
        T* ptr = &m_buffer[m_index];
        m_index += n;
        return ptr;
    }
    
    void deallocate(T* p, size_t n) noexcept {
        // Memory is reused only on reset
    }

private:
    std::array<T, Size> m_buffer;
    size_t m_index{0};
};

// Использование статического аллокатора
using PacketQueue = Queue<Packet, 32, StaticAllocator<Packet, 32>>;
```

## Анализ и улучшения протокола

### Недостатки исходного протокола

1.  **Низкая эффективность**: Высокие накладные расходы (14+ байт на пакет)
2.  **Уязвимость к сбоям**: Отсутствие экранирования служебных байт
3.  **Слабый CRC**: CRC8 недостаточен для надежного контроля целостности
4.  **Строковые идентификаторы**: Неэффективный поиск и большой overhead

### Реализованные улучшения

1.  **Бинарные идентификаторы**:
    ```cpp
    // Вместо строки - 16-битный идентификатор
    enum class FunctionID : uint16_t {
        GET_TEMPERATURE = 0x01,
        SET_CONFIG      = 0x02,
        // ...
    };
    ```

2.  **Усиленный контроль целостности**:
    ```cpp
    // CRC16 вместо CRC8
    class CRC16 {
    public:
        static constexpr uint16_t calculate(std::span<const uint8_t> data) {
            uint16_t crc = 0xFFFF;
            for (uint8_t byte : data) {
                crc ^= byte;
                for (uint8_t i = 0; i < 8; ++i) {
                    crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
                }
            }
            return crc;
        }
    };
    ```

3.  **Оптимизированный формат пакета**:
    ```
    [SYNC][LEN][FID][TYPE][SEQ][DATA...][CRC16]
    ```

### Требования к сборке

```cmake
# Минимальные требования компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Флаги для embedded
add_compile_options(
    -ffunction-sections
    -fdata-sections
    -fno-exceptions
    -fno-rtti
    -fno-use-cxa-atexit
)

# Линковка
add_link_options(
    -Wl,--gc-sections
    -specs=nano.specs
    -specs=nosys.specs
)
```

Проект демонстрирует современный подход к embedded-разработке на C++ с акцентом на безопасность типов, нулевые overheads и модульность архитектуры.
