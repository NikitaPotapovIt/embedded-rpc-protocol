#include "../../include/rpc/service.hpp"
#include "../../include/drivers/uart.hpp"
#include <cstring>

namespace rpc {
    bool Service::register_handler(const std::string& name, Handler handler) {
        return m_handlers.emplace(name, std::move(handler)).second;
    }

    void Service::process_message(const Message& request) {
        Message response;
        response.type = MessageType::Response;
        response.sequence_number = request.sequence_number;
        
        auto it = m_handlers.find(request.function_name);
        if (it != m_handlers.end()) {
            // Вызываем обработчик
            it->second(request, response);
        } else {
            // Функция не найдена
            response.type = MessageType::Error;
            response.arguments = reinterpret_cast<const std::uint8_t*>("Function not found");
            response.arguments_length = 15; // strlen("Function not found")
        }
        
        send_response(response);
    }

    void Service::send_response(const Message& response) {
        // Здесь будет код формирования и отправки пакета ответа
        // Пока просто заглушка для демонстрации
        auto& uart = drivers::Uart::instance();
        const char* msg = "Response sent\n";
        uart.send(reinterpret_cast<const std::uint8_t*>(msg), std::strlen(msg));
    }
} // namespace rpc
