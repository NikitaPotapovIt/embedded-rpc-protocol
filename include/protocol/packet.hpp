#pragma once
#include <cstdint>
#include <array>
#include "../utils/allocator.hpp"

namespace protocol {

    struct Packet {
        static constexpr std::size_t MaxSize = 128;

        std::array<std::uint8_t, MaxSize> data;
        std::size_t lengh{0};

        bool valiid{false};
    };
} // namesoace protocol
