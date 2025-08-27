#include "main.h"
#include "rpc/client.hpp"
#include "rpc/service.hpp"
#include "drivers/uart.hpp"
#include "protocol/parser.hpp"
#include "protocol/sender.hpp"
#include <string>

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);

// Функции для RPC
int32_t add(int32_t a, int32_t b) { return a + b; }
float get_temperature() { return 25.5f; }
void set_led(bool state) { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, state ? GPIO_PIN_SET : GPIO_PIN_RESET); }

// Обработчик пакетов для Parser
void packet_handler(const protocol::Packet& packet, void* user_data) {
    auto* service = static_cast<rpc::Service*>(user_data);
    service->handle_packet(packet);
}

extern "C" int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    drivers::Uart uart(&huart2);
    protocol::Parser parser(uart, packet_handler, nullptr); // Передаём UART, handler и user_data
    rpc::Client client(uart, parser); // Корректная инициализация Client
    rpc::Service service(parser);     // Корректная инициализация Service

    // Регистрация обработчиков с правильными типами
    service.register_handler("add", &add);
    service.register_handler("get_temperature", &get_temperature);
    service.register_handler("set_led", &set_led);

    xTaskCreate([](void* param) {
        auto* s = static_cast<rpc::Service*>(param);
        while (true) {
            s->process(); // Используем process() из Service
            vTaskDelay(1);
        }
    }, "Service", 256, &service, 1, nullptr);

    vTaskStartScheduler();
    while (1) {}
}

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

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}
