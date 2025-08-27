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

    template<typename Result, typename... Args>
    bool register_handler(const std::string& name, std::function<Result(Args...)> func) {
        m_handlers[name] = Handler<Result, std::tuple<Args...>>{func};
        return true;
    }

private:
    template<typename Result, typename... Args>
    struct Handler {
        std::function<Result(Args...)> func;

        void invoke(const std::uint8_t* args, std::size_t args_length, std::uint8_t* res, std::size_t* res_length) {
            auto args_tuple = Serializer::deserialize_tuple<Args...>(args);
            auto result = std::apply(func, args_tuple);
            Serializer::serialize(result, res);
            *res_length = sizeof(result);
        }
    };

    void process_packet(const protocol::Packet& packet);
    protocol::Parser& m_parser;
    std::map<std::string, std::function<void(const std::uint8_t*, std::size_t, std::uint8_t*, std::size_t*)>> m_handlers;
};

} // namespace rpc
