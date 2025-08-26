#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "../include/drivers/uart.hpp"
#include "../include/protocol/parser.hpp"
#include "../include/rpc/service.hpp"

// Глобальные объекты
protocol::Parser* packet_parser = nullptr;
rpc::Service& rpc_service = rpc::Service::instance();

// Очередь для передачи байт из прерывания в задачу
QueueHandle_t uart_rx_queue;

void uart_rx_callback(std::uint8_t byte) {
    // Отправляем байт в очередь из прерывания
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(uart_rx_queue, &byte, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void packet_handler(const protocol::Packet& packet) {
    if (packet.valid) {
        // Здесь будет разбор RPC сообщения из packet.data
        // Пока просто выводим уведомление
        const char* msg = "Packet received!\n";
        drivers::Uart::instance().send(
            reinterpret_cast<const std::uint8_t*>(msg), 
            std::strlen(msg)
        );
    }
}

void uart_receive_task(void* arg) {
    std::uint8_t byte;
    while (true) {
        if (xQueueReceive(uart_rx_queue, &byte, portMAX_DELAY) == pdPASS) {
            if (packet_parser) {
                packet_parser->process_byte(byte);
            }
        }
    }
}

void rpc_process_task(void* arg) {
    while (true) {
        // Здесь будет обработка RPC сообщений из очереди
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    
    // Инициализация драйверов
    auto& uart = drivers::Uart::instance();
    uart.init();
    uart.set_rx_callback(uart_rx_callback);
    
    // Создаем очередь
    uart_rx_queue = xQueueCreate(64, sizeof(std::uint8_t));
    
    // Создаем парсер
    packet_parser = new protocol::Parser(packet_handler);
    
    // Создаем задачи FreeRTOS
    xTaskCreate(uart_receive_task, "UART_Rx", 256, nullptr, 4, nullptr);
    xTaskCreate(rpc_process_task, "RPC_Proc", 256, nullptr, 3, nullptr);
    
    // Запускаем планировщик
    vTaskStartScheduler();
    
    while (1) {}
}
