#include once
#include <functional>
#include <string>
#include <unordered_map>
#include "./rpc/types.hpp"
#include "../utils/noncopyable.hpp"
#include "../utils/allocator.hpp"


namespace rcp {

    class Service : utils::NonCopyable {
        
        public:
        using Handler = std::function<void(const uint8_t* args, uint16_t len, 
            uint8_t* result, uint16_t* result_len)>;

        static Service& instance() {
            static Service instance;
            return instance;
        }

        // Регистрация обработчика по имени функции
        void register_handler(const std::string& name, Handler handler);

        // Оновной метод обработки входящего пакета
        void handle_packet(const protocol::Packet& packet);

        // Метод для отправки отчета
        void send_response(uint8_t sequence_number, const uint8_t* data, uint16_t length);

        private:
        Service() = default;

        std::unordered_map<std::string, Handler> m_handlers;

    };
} // namespace rpc
