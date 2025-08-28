#include "main.h"
#include "rpc/client.hpp"
#include "rpc/service.hpp"
#include "drivers/uart.hpp"
#include "protocol/parser.hpp"
#include "protocol/sender.hpp"
#include <string>

// Глобальный обработчик UART (инициализируется CubeMX)
UART_HandleTypeDef huart2;

// Прототипы функций инициализации (генерируются CubeMX)
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);

int32_t add(int32_t a, int32_t b) { return a + b; }     // RPC функция сложения двух чисел
float get_temperature() { return 25.5f; }               // RPC функция получения температуры (заглушка)
void set_led(bool state) {                              // RPC функция управления светодиодом (true - вкл, false - выкл)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, state ? GPIO_PIN_SET : GPIO_PIN_RESET); 
}

// Обработчик входящих пакетов для парсера
void packet_handler(const protocol::Packet& packet, void* user_data) {
    auto* service = static_cast<rpc::Service*>(user_data);
    service->handle_packet(packet);
}

/**
 * Основная функция приложения
 * Код возврата (никогда не возвращает управление)
 * 
 * Выполняет инициализацию системы в следующем порядке:
 * 1. Инициализация HAL и тактирования
 * 2. Инициализация периферии (GPIO, UART)
 * 3. Создание объектов драйверов и сервисов
 * 4. Регистрация RPC обработчиков
 * 5. Запуск FreeRTOS планировщика
 */

extern "C" int main(void) {
    HAL_Init();                 // 1. Инициализация HAL библиотеки
    SystemClock_Config();       // 2. Настройка системной частоты и тактирования
    // 3. Инициализация периферии
    MX_GPIO_Init();             // Настройка GPIO (светодиоды, кнопки)
    MX_USART2_UART_Init();      // Настройка UART2 для коммуникации

    // 4. Создание объектов системы
    drivers::Uart uart(&huart2);                            // Драйвер UART
    protocol::Parser parser(uart, packet_handler, nullptr); // Парсер протокола. Передаём UART, handler и user_data
    rpc::Client client(uart, parser);                       // RPC клиент (для отправки запросов)
    rpc::Service service(parser);                           // RPC сервис (для обработки запросов)

    // 5. Регистрация RPC обработчиков функций
    service.register_handler("add", &add);                          // Функция сложения
    service.register_handler("get_temperature", &get_temperature);  // Получение температуры
    service.register_handler("set_led", &set_led);                  // Управление светодиодом

    // 6. Создание задачи для обработки RPC сервиса
    xTaskCreate([](void* param) {
        auto* s = static_cast<rpc::Service*>(param);
        while (true) {
            s->process();   // Обработка фоновых задач сервиса
            vTaskDelay(1);  // Задержка 10ms
        }
    }, "Service", 256, &service, 1, nullptr);

    // 7. Запуск планировщика FreeRTOS (не возвращает управление)
    vTaskStartScheduler();
    // 8. Бесконечный цикл на случай остановки планировщика
    while (1) {}
}

/**
 * Конфигурация системной частоты
 * 
 * Настраивает PLL для генерации 100MHz из HSE 8MHz:
 * - PLLM = 8, PLLN = 200, PLLP = 2 → 8MHz * 200 / 2 = 800MHz / 2 = 400MHz?
 * - На самом деле: HSE / PLLM * PLLN / PLLP = 8MHz / 8 * 200 / 2 = 100MHz
 * 
 * Точные параметры зависят от конкретного STM32 и тактовой схемы
 */

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * Инициализация UART2
 * 
 * Настройки: 115200 baud, 8 бит данных, 1 стоп-бит, без контроля четности
 * Используется для отладочного вывода и RPC коммуникации
 */

void MX_USART2_UART_Init(void) {
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * Инициализация GPIO
 * 
 * Настраивает пины:
 * - PA5: выход для светодиода (User LED)
 * - Другие пины согласно конфигурации CubeMX
 */

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Настройка пина PA5 как выхода для светодиода
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * Обработчик критических ошибок
 * 
 * Вызывается при фатальных ошибках инициализации
 * Отключает прерывания и останавливает систему
 */

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}

/**
 * Callback системного таймера (SysTick)
 * 
 * Вызывается каждую миллисекунду
 * Используется HAL для отсчета времени и таймаутов
 */

extern "C" void HAL_SYSTICK_Callback(void) {
    // Может быть использован для периодических задач
}
