#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "../include/drivers/uart.hpp"
#include "../include/protocol/parser.hpp"
#include "../include/rpc/service.hpp"
#include "../include/rpc/client.hpp"

drivers::Uart* global_uart_instance = nullptr;
protocol::Parser* global_parser_instance = nullptr;

int32_t rpc_add(int32_t a, int32_t b) { return a + b; }
void rpc_set_led(bool state) { HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET); }
float rpc_get_temperature() { return 25.0f; }

void packet_handler(const protocol::Packet& packet) {
    if (packet.valid) {
        rpc::Client* client = rpc::Client::global_client_instance;
        if (client && (packet.type == rpc::MessageType::Response || packet.type == rpc::MessageType::Error)) {
            xQueueSend(client->get_response_queue(), &packet, portMAX_DELAY);
        }
    }
}

void uart_receive_task(void* arg) {
    drivers::Uart* uart = static_cast<drivers::Uart*>(arg);
    (void)uart; // Избежать предупреждения о неиспользуемой переменной
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

void rx_callback(std::uint8_t byte) {
    extern protocol::Parser* global_parser_instance;
    global_parser_instance->process_byte(byte);
}

int main() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    drivers::Uart uart(&huart2);
    global_uart_instance = &uart;
    protocol::Parser parser(uart, packet_handler);
    global_parser_instance = &parser;
    rpc::Service rpc_service(parser);
    rpc::Client client(uart, parser);
    global_client_instance = &client;

    rpc_service.register_handler("add", std::function<int32_t(int32_t, int32_t)>(rpc_add));
    rpc_service.register_handler("set_led", std::function<void(bool)>(rpc_set_led));
    rpc_service.register_handler("get_temp", std::function<float()>(rpc_get_temperature));

    uart.set_rx_callback(rx_callback);
    uart.start();

    xTaskCreate(uart_receive_task, "UART_Rx", 256, &uart, 4, nullptr);
    xTaskCreate(rpc_process_task, "RPC_Proc", 256, &rpc_service, 3, nullptr);

    vTaskStartScheduler();
    while (true) {}
}

extern "C" void Error_Handler() {
    while (true) {}
}
