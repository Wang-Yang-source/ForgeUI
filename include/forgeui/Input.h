#pragma once

#include <cstdint>

namespace forgeui {

enum class InputType : uint8_t { Press, Release, Click, FocusNext, FocusPrevious, Encoder };

struct InputEvent {
    InputType type = InputType::Click;
    int16_t value = 0;
    uint32_t timestamp = 0;
};

} // namespace forgeui
