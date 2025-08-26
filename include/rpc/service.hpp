#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "rpc/types.hpp"
#include "../utils/noncopyable.hpp"
#include "../utils/allocator.hpp"

namespace rpc {

    class Service : private utils::NonCopyable {
    public:
        using Handler = std::function<void(const Message&, Message&)>;
        
        static Service& instance() {
            static Service instance;
            return instance;
        }
        
        bool register_handler(const std::string& name, Handler handler);
        void process_message(const Message& request);
        void send_response(const Message& response);
        
    private:
        Service() = default;
        
        std::unordered_map<
            std::string, 
            Handler,
            std::hash<std::string>,
            std::equal_to<std::string>,
            utils::StaticAllocator<std::pair<const std::string, Handler>, 32>
        > m_handlers;
    };
} // namespace rpc
