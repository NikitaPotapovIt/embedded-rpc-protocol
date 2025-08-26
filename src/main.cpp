#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "../include/drivers/uart.hpp"
#include "../include/protocol/parser.hpp"
#include "../include/rpc/service.hpp"
#include "../include/rpc/client.hpp"

protocol::Parser* packet_parser = nullptr;
rpc::Service& rpc_service = rpc::Service::instance();
rpc::Client& rpc_client = rpc::Client::instance();
QueueHandle_t uart_rx_queue;

int32_t rpc_add(int32_t a, int32_t b) {
    return a + b;
}

void rpc_set_led(bool state) {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

float rpc_get_temperature() {
    return 25.5f;
}

void uart_rx_callback(std::uint8_t byte) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(uart_rx_queue, &byte, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void packet_handler(const protocol::Packet& packet) {
    rpc_service.process_packet(packet);
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
        // Demo client call (commented for build-only)
        /*
        try {
            int32_t sum = rpc_client.call<int32_t, int32_t, int32_t>("add", 5, 3);
            float temp = rpc_client.call<float>("get_temp");
            rpc_client.stream_call<bool>("set_led", true);
        } catch (const std::exception& e) {
            const char* msg = "Client error\n";
            drivers::Uart::instance().send(reinterpret_cast<const std::uint8_t*>(msg), std::strlen(msg));
        }
        */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    auto& uart = drivers::Uart::instance();
    uart.init();
    uart.set_rx_callback(uart_rx_callback);

    rpc_service.register_handler("add", rpc_add);
    rpc_service.register_handler("set_led", rpc_set_led);
    rpc_service.register_handler("get_temp", rpc_get_temperature);

    uart_rx_queue = xQueueCreate(64, sizeof(std::uint8_t));
    packet_parser = new protocol::Parser(packet_handler);

    xTaskCreate(uart_receive_task, "UART_Rx", 256, nullptr, 4, nullptr);
    xTaskCreate(rpc_process_task, "RPC_Proc", 256, nullptr, 3, nullptr);

    vTaskStartScheduler();

    while (1) {}
}
