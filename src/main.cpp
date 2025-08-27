#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "../include/drivers/uart.hpp"
#include "../include/protocol/parser.hpp"
#include "../include/rpc/service.hpp"
#include "../include/rpc/client.hpp"

int32_t rpc_add(int32_t a, int32_t b) { return a + b; }
void rpc_set_led(bool state) { HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET); }
float rpc_get_temperature() { return 25.0f; }

void packet_handler(const protocol::Packet& packet) {
    if (packet.valid) {
        extern rpc::Client* global_client_instance;
        if (global_client_instance && (packet.type == rpc::MessageType::Response || packet.type == rpc::MessageType::Error)) {
            xQueueSend(global_client_instance->get_response_queue(), &packet, portMAX_DELAY);
        }
    }
}

void uart_receive_task(void* arg) {
    drivers::Uart* uart = static_cast<drivers::Uart*>(arg);
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void rpc_process_task(void* arg) {
    rpc::Service* service = static_cast<rpc::Service*>(arg);
    while (true) {
        service->process();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

rpc::Client* global_client_instance = nullptr;

int main() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    drivers::Uart uart(&huart2);
    protocol::Parser parser(uart, packet_handler);
    rpc::Service rpc_service(parser);
    rpc::Client client(uart, parser);
    global_client_instance = &client;

    rpc_service.register_handler("add", std::function<int32_t(int32_t, int32_t)>(rpc_add));
    rpc_service.register_handler("set_led", std::function<void(bool)>(rpc_set_led));
    rpc_service.register_handler("get_temp", std::function<float()>(rpc_get_temperature));

    uart.set_rx_callback([&parser](std::uint8_t byte) { parser.process_byte(byte); });
    uart.start();

    xTaskCreate(uart_receive_task, "UART_Rx", 256, &uart, 4, nullptr);
    xTaskCreate(rpc_process_task, "RPC_Proc", 256, &rpc_service, 3, nullptr);

    vTaskStartScheduler();
    while (true) {}
}

extern "C" void Error_Handler() {
    while (true) {}
}
