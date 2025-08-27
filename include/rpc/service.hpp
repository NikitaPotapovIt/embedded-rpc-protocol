#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <map>
#include "types.hpp"
#include "../protocol/parser.hpp"
#include "serializer.hpp"

namespace rpc {

class Service {
public:
    explicit Service(protocol::Parser& parser);
    void process();
    void handle_packet(const protocol::Packet& packet);

    template<typename Result, typename... Args>
    bool register_handler(const std::string& name, Result (*func)(Args...)) {
        m_handlers[name] = [func](const std::uint8_t* args, std::size_t args_length, std::uint8_t* res, std::size_t* res_length) {
            auto args_tuple = Serializer::deserialize_tuple<Args...>(args);
            if constexpr (std::is_void_v<Result>) {
                std::apply(func, args_tuple);
                *res_length = 0;
            } else {
                auto result = std::apply(func, args_tuple);
                Serializer::serialize(result, res);
                *res_length = sizeof(result);
            }
        };
        return true;
    }

private:
    protocol::Parser& m_parser;
    std::map<std::string, std::function<void(const std::uint8_t*, std::size_t, std::uint8_t*, std::size_t*)>> m_handlers;
};

} // namespace rpc
