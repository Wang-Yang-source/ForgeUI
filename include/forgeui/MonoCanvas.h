#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

namespace forgeui {

// Packed 1-bit canvas for OLED and small monochrome LED panels.
template <int Width, int Height>
class MonoCanvas {
public:
    static_assert(Width > 0 && Height > 0, "Canvas dimensions must be positive");
    static constexpr size_t byteCount = (static_cast<size_t>(Width) * Height + 7u) / 8u;

    void clear(bool on = false) { bytes_.fill(on ? 0xffu : 0u); }

    void pixel(int x, int y, bool on = true) {
        if (x < 0 || x >= Width || y < 0 || y >= Height) return;
        const size_t index = static_cast<size_t>(y) * Width + static_cast<size_t>(x);
        const uint8_t mask = static_cast<uint8_t>(1u << (index & 7u));
        if (on) bytes_[index >> 3u] |= mask;
        else bytes_[index >> 3u] &= static_cast<uint8_t>(~mask);
    }

    bool pixel(int x, int y) const {
        if (x < 0 || x >= Width || y < 0 || y >= Height) return false;
        const size_t index = static_cast<size_t>(y) * Width + static_cast<size_t>(x);
        return (bytes_[index >> 3u] & (1u << (index & 7u))) != 0;
    }

    void fill(int x0, int y0, int x1, int y1, bool on = true) {
        const int left = std::max(0, std::min(x0, x1));
        const int right = std::min(Width - 1, std::max(x0, x1));
        const int top = std::max(0, std::min(y0, y1));
        const int bottom = std::min(Height - 1, std::max(y0, y1));
        for (int y = top; y <= bottom; ++y)
            for (int x = left; x <= right; ++x) pixel(x, y, on);
    }

    void box(int x0, int y0, int x1, int y1, bool on = true) {
        fill(x0, y0, x1, y0, on);
        fill(x0, y1, x1, y1, on);
        fill(x0, y0, x0, y1, on);
        fill(x1, y0, x1, y1, on);
    }

    const uint8_t* data() const { return bytes_.data(); }
    uint8_t* data() { return bytes_.data(); }

private:
    std::array<uint8_t, byteCount> bytes_{};
};

} // namespace forgeui
