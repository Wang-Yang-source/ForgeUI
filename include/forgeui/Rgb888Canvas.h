#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "Canvas.h"

namespace forgeui {

template <int Width, int Height>
class Rgb888Canvas {
public:
    static_assert(Width > 0 && Height > 0, "Canvas dimensions must be positive");
    static constexpr size_t byteCount = static_cast<size_t>(Width) * Height * 3u;

    void clear(Color color = Color::black()) {
        for (size_t i = 0; i < byteCount; i += 3) setAt(i, color);
    }
    void pixel(int x, int y, Color color) {
        if (!inside(x, y)) return;
        setAt((static_cast<size_t>(y) * Width + static_cast<size_t>(x)) * 3u, color);
    }
    void fill(int x0, int y0, int x1, int y1, Color color) {
        const int left = std::max(0, std::min(x0, x1));
        const int right = std::min(Width - 1, std::max(x0, x1));
        const int top = std::max(0, std::min(y0, y1));
        const int bottom = std::min(Height - 1, std::max(y0, y1));
        for (int y = top; y <= bottom; ++y)
            for (int x = left; x <= right; ++x) pixel(x, y, color);
    }
    void box(int x0, int y0, int x1, int y1, Color color) {
        fill(x0, y0, x1, y0, color); fill(x0, y1, x1, y1, color);
        fill(x0, y0, x0, y1, color); fill(x1, y0, x1, y1, color);
    }
    const uint8_t* data() const { return pixels_.data(); }
    uint8_t* data() { return pixels_.data(); }

private:
    static constexpr bool inside(int x, int y) { return x >= 0 && x < Width && y >= 0 && y < Height; }
    void setAt(size_t index, Color color) {
        pixels_[index] = color.r; pixels_[index + 1] = color.g; pixels_[index + 2] = color.b;
    }
    std::array<uint8_t, byteCount> pixels_{};
};

} // namespace forgeui
