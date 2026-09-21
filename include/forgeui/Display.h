#pragma once

#include <cstdint>

namespace forgeui {

struct DisplayRect {
    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 0;
    int16_t height = 0;
};

// Hardware-neutral flush boundary. ESP-IDF, Arduino, SDL, and custom drivers
// can implement this without pulling platform headers into the ForgeUI core.
template <typename PixelT>
class DisplayBackend {
public:
    virtual ~DisplayBackend() = default;
    virtual void flush(DisplayRect region, const PixelT* pixels, uint32_t stride) = 0;
};

} // namespace forgeui
