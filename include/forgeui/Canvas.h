#pragma once

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>

namespace forgeui {

struct Color {
    uint8_t r, g, b;
    static constexpr Color black() { return {0, 0, 0}; }
    static constexpr Color white() { return {255, 255, 255}; }
    static constexpr Color cyan() { return {0, 220, 255}; }
};

template <int Width, int Height>
class Canvas {
public:
    static_assert(Width > 0 && Height > 0, "Canvas dimensions must be positive");
    static constexpr size_t pixelCount = static_cast<size_t>(Width) * Height;

    void clear(Color color = Color::black()) {
        pixels_.fill(color);
    }

    void pixel(int x, int y, Color color) {
        if (!inside(x, y)) return;
        pixels_[static_cast<size_t>(y) * Width + x] = color;
    }

    void fill(int x0, int y0, int x1, int y1, Color color) {
        const int left = std::max(0, std::min(x0, x1));
        const int right = std::min(Width - 1, std::max(x0, x1));
        const int top = std::max(0, std::min(y0, y1));
        const int bottom = std::min(Height - 1, std::max(y0, y1));
        if (left > right || top > bottom) return;
        for (int y = top; y <= bottom; ++y)
            for (int x = left; x <= right; ++x) pixel(x, y, color);
    }

    void box(int x0, int y0, int x1, int y1, Color color) {
        fill(x0, y0, x1, y0, color);
        fill(x0, y1, x1, y1, color);
        fill(x0, y0, x0, y1, color);
        fill(x1, y0, x1, y1, color);
    }

    void line(int x0, int y0, int x1, int y1, Color color) {
        const int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        const int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;
        for (;;) {
            pixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            const int twice = 2 * error;
            if (twice >= dy) { error += dy; x0 += sx; }
            if (twice <= dx) { error += dx; y0 += sy; }
        }
    }

    const Color* data() const { return pixels_.data(); }
    Color* data() { return pixels_.data(); }

private:
    static constexpr bool inside(int x, int y) {
        return x >= 0 && x < Width && y >= 0 && y < Height;
    }
    std::array<Color, pixelCount> pixels_{};
};

} // namespace forgeui
