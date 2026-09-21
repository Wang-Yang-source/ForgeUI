#pragma once

#include <cstddef>
#include <cstdint>

namespace forgeui {

// HUB75 scan timing and DMA remain in the hardware driver. ForgeUI submits a
// complete RGB888 frame through this small platform-neutral boundary.
template <int Width, int Height>
class Hub75Backend {
public:
    using SubmitFrame = bool (*)(const uint8_t*, size_t, uint16_t, void*);
    explicit Hub75Backend(SubmitFrame submit, void* context = nullptr)
        : submit_(submit), context_(context) {}
    bool flush(const uint8_t* rgb888, uint16_t fadeMs = 0) const {
        return submit_ && submit_(rgb888, static_cast<size_t>(Width) * Height * 3u, fadeMs, context_);
    }
    static constexpr size_t frameBytes() { return static_cast<size_t>(Width) * Height * 3u; }
private:
    SubmitFrame submit_ = nullptr;
    void* context_ = nullptr;
};

} // namespace forgeui
