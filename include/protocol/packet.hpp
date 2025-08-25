#pragma once
#include <cstdint>
#include <array>
#include "../utils/allocator.hpp"

namespace protocol {

    struct Packet {
        static constexpr std::size_t MaxSize = 128;

        std::array<std::uint8_t, MaxSize> data;
        std::size_t length{0};

        bool valid{false};
    };
} // namesoace protocol
