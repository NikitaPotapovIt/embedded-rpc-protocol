#pragma once
#include <cstdint>
#include "./packet.hpp"
#include "../utils/noncopyable.hpp"

namespace protocol {

class Sender : private utils::NonCopyable {
public:
    static Sender& instance() {
        static Sender instance;
        return instance;
    }

    bool send_transport(const std::uint8_t* data, std::size_t length);

private:
    Sender() = default;
};

} // namespace protocol
