# Embedded RPC over UART for STM32F411RE

This project implements a **Remote Procedure Call (RPC)** protocol over UART, enabling seamless function calls between a host (client) and an STM32F411RE device (server) running FreeRTOS. Designed for the **Nucleo-F411RE** board, it leverages **C++17** with FreeRTOS integration, providing a modular, type-safe, and extensible architecture.

---

## Table of Contents

- [Architectural Overview](#-architectural-overview)
- [Key Features](#-key-features)
- [Quick Start](#-quick-start)
- [Project Structure](#-project-structure)
- [Implementation Details](#-implementation-details)
- [Protocol Analysis and Improvements](#-protocol-analysis-and-improvements)
- [Limitations](#-limitations)
- [Future Improvements](#-future-improvements)
- [Build Requirements](#-build-requirements)
- [Conclusion](#-conclusion)

---

## Architectural Overview

The project implements a layered RPC protocol over UART for reliable and efficient communication. The architecture separates concerns into distinct layers, ensuring modularity and scalability.

```
┌─────────────────┐    Serialized Data    ┌─────────────────┐
│   Host/Client   │ ◄───────────────────► │  Device/Server  │
└─────────────────┘                       └─────────────────┘
            │                                      │
            │ UART Stream                          │ UART Stream
            ▼                                      ▼
┌─────────────────────────┐              ┌─────────────────────────┐
│ Application Layer       │              │ Application Layer       │
│  - Function Registry    │              │  - Task-based Processing│
│  - Type-Safe Wrappers   │              │  - Dynamic Dispatch     │
├─────────────────────────┤              ├─────────────────────────┤
│ Transport Layer         │              │ Transport Layer         │
│  - Message Serialization│              │  - Message Parsing      │
│  - Request/Response     │              │  - Error Handling       │
├─────────────────────────┤              ├─────────────────────────┤
│ Data Link Layer         │              │ Data Link Layer         │
│  - Packet Framing       │              │  - Stream Deframing     │
│  - CRC8 Validation      │              │  - Byte-Stuffing (opt.) │
├─────────────────────────┤              ├─────────────────────────┤
│ Physical Layer          │              │ Physical Layer          │
│  - UART Driver (HAL)    │              │  - Async UART (IT)      │
└─────────────────────────┘              └─────────────────────────┘
```

---

## Key Features

- **RPC by Function Name**: Call functions remotely using string identifiers (e.g., `add(1, 2)`).
- **Return Value Support**: Results are sent back to the client.
- **FreeRTOS Integration**: Request processing runs in a dedicated FreeRTOS task.
- **FPU Compatibility**: Supports `float` operations using the Cortex-M4F's hardware FPU.
- **Modular Architecture**: Separates protocol, RPC logic, and drivers for maintainability.
- **C++17 Features**: Utilizes `std::tuple`, `std::function`, lambdas, and `constexpr`.

---

## Quick Start

### 1. Register Handlers
Register functions on the server side to handle incoming RPC requests.

```cpp
// In main.cpp
service.register_handler("add", &add);
service.register_handler("get_temperature", &get_temperature);
service.register_handler("set_led", &set_led);
```

**Example Functions**:
```cpp
int32_t add(int32_t a, int32_t b);
float get_temperature();
void set_led(bool state);
```

### 2. Call Remote Functions
Invoke functions from the client and retrieve results.

```cpp
int32_t result;
if (client.call("add", std::make_tuple(5, 3), result)) {
    // Success: result = 8
} else {
    // Error: timeout or function not found
}
```

### 3. Start Processing Task
Run the server-side processing loop in a FreeRTOS task.

```cpp
xTaskCreate([](void* param) {
    auto* s = static_cast<rpc::Service*>(param);
    while (true) {
        s->process();
        vTaskDelay(1); // Yield to other tasks
    }
}, "RPC_Service", 256, &service, 1, nullptr);
```

---

## Project Structure

```
.
├── include/                 # Header files
│   ├── drivers/             # Driver abstractions
│   │   └── uart.hpp         # UART interface
│   ├── protocol/            # Data link and transport layers
│   │   ├── parser.hpp       # Byte stream to packet parser
│   │   ├── sender.hpp       # Packet formation and transmission
│   │   └── crc.hpp          # CRC8 computation
│   └── rpc/                 # RPC logic
│       ├── client.hpp       # Client-side function calls
│       └── service.hpp      # Server-side function dispatching
├── src/                     # Source files
│   ├── drivers/
│   │   └── uart.cpp         # UART driver implementation (HAL)
│   ├── protocol/
│   │   ├── parser.cpp
│   │   ├── sender.cpp
│   │   └── crc.cpp
│   ├── rpc/
│   │   ├── client.cpp
│   │   └── service.cpp
│   └── main.cpp             # Entry point
├── lib/                     # External libraries
│   └── FreeRTOS/            # FreeRTOS with ARM_CM4F port
├── platformio.ini           # PlatformIO build configuration
└── README.md                # This file
```

---

## Implementation Details

### Data Link Layer: Reliable Transmission
- **Packet Format**:
  ```cpp
  // 0xFA | l_l | l_h | crc8_hdr | 0xFB | payload | crc8_full | 0xFE
  ```
- **Synchronization**: Markers `0xFA`, `0xFB`, `0xFE` ensure packet boundary detection.
- **Packet Length**: 2 bytes (up to 65,535 bytes).
- **CRC8**: Validates header and full packet integrity.
- **Parser**: Finite state machine for stream-to-packet conversion.

### Transport Layer: RPC Logic
- **Message Format**:
  ```cpp
  // type | seq | name\0 | args...
  ```
- **Message Types**: `0x0B` (request), `0x0C` (response), `0x21` (error).
- **Sequence Number**: Matches requests to responses.
- **Function Names**: Null-terminated strings.
- **Arguments**: Serialized as raw bytes.

### FreeRTOS Integration
- Uses `HAL_SYSTICK_Callback()` for `HAL_Delay()` compatibility.
- Avoids duplicate handlers (e.g., `vPortSVCHandler`, `xPortPendSVHandler`) by relying solely on `port.c`.

---

## Protocol Analysis and Improvements

### Current Limitations
- **No Timeout**: Client may hang if response is lost.
- **String-Based Names**: Slow `strcmp` lookup with high overhead.
- **No Argument Type Validation**: Risk of undefined behavior (e.g., `add("hello", 5)`).
- **Weak CRC8**: 1/256 chance of missing errors.
- **No Flow Control**: Risk of buffer overflow under heavy load.
- **Blocking `call()`**: May starve other tasks.

### Implemented Improvements
- **FPU Support**: Enabled via `build_flags` and `extra_scripts` for `float` operations.
- **No Duplicate Handlers**: Removed `SVC_Handler`, `PendSV_Handler` from `stm32f4xx_it.c`.
- **Cross-Platform**: Builds on macOS, Windows, and Linux with PlatformIO.
- **Warning Suppression**: Uses `-specs=nosys.specs` to eliminate `_write`, `_close` warnings.

---

## Limitations
- **No Timeout in `call()`**: May block indefinitely.
- **O(n) Function Lookup**: Slow for 100+ functions.
- **No Argument Validation**: Type mismatches cause undefined behavior.
- **Packet Size Limit**: 64KB may be insufficient for large payloads.
- **No Array Support**: Only fixed-size structures.
- **Half-Duplex UART**: No flow control, risking data loss.

---

## Future Improvements
- **Response Timeout**: Add `client.call(..., timeout_ms)` for reliability.
- **Hashed Function Names**: Use `crc16("add")` for O(1) lookups.
- **Argument Type Validation**: Include type byte (`int32`, `float`, `bool`).
- **Asynchronous Calls**: Implement `call_async(name, args, callback)`.
- **Packet Buffering**: Use an outgoing packet queue.
- **Stronger CRC**: Upgrade to CRC16 for better error detection.
- **Stream Support**: Add `0x16` for streaming data (e.g., logs).
- **Serialization**: Use FlatBuffers or Cap'n Proto for structured data.
- **Array Support**: Include length prefix in payloads.

---

## Build Requirements

```ini
[env:nucleo_f411re]
platform = ststm32
board = nucleo_f411re
framework = stm32cube

board_build.link_flags =
    --specs=nosys.specs
    --specs=nano.specs

build_flags =
    -DUSE_HAL_DRIVER
    -DSTM32F411xE
    -std=c++17
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -I$PROJECT_DIR/include
    -I$PROJECT_DIR/lib/FreeRTOS/include
    -I$PROJECT_DIR/lib/FreeRTOS/portable/GCC/ARM_CM4F

build_unflags =
    -mfloat-abi=soft
    -mfloat-abi=softfp
    -mfloat-abi=hard
    -mfpu=*

lib_extra_dirs = $PROJECT_DIR/lib
lib_archive = no
```

---

## Conclusion

This project delivers a robust **RPC-over-UART** implementation for the STM32F411RE, fully integrated with **FreeRTOS** and built using **C++17**:
- Functional RPC protocol over UART.
- FreeRTOS task-based processing.
- Cross-platform build support via PlatformIO.
- Comprehensive documentation and source code.

The design emphasizes modern embedded development principles: **type safety**, **modularity**, and **low-level control** (e.g., FPU, linker flags). Future enhancements can address current limitations to make the protocol even more robust and versatile.