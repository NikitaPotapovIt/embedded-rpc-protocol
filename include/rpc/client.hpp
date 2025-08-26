#pragma once
#include <string>
#include <functional>
#include <stdexcept>
#include "./types.hpp"
#include "./serializer.hpp"
#include "../utils/noncopyable.hpp"

namespace rpc {

class Client : private utils::NonCopyable {
public:
    static Client& instance() {
        static Client instance;
        return instance;
    }

    template<typename Result, typename... Args>
    Result call(const std::string& func_name, Args... args);

    template<typename... Args>
    void stream_call(const std::string& func_name, Args... args);

private:
    Client();
    bool send_message(const Message& msg);
    bool wait_response(Message& response, std::uint8_t seq, TickType_t timeout);

    QueueHandle_t m_response_queue;
    std::uint8_t m_sequence{0};
};

} // namespace rpc
