#pragma once

#include <cstdint>

namespace forgeui {

struct Rect {
    int16_t x = 0, y = 0, width = 0, height = 0;
};

template <typename CanvasT>
class Component {
public:
    explicit Component(Rect bounds = {}) : bounds_(bounds) {}
    virtual ~Component() = default;

    virtual void update(uint32_t now) = 0;
    virtual void draw(CanvasT& canvas) const = 0;

    Rect bounds() const { return bounds_; }
    void setBounds(Rect bounds) { bounds_ = bounds; }

protected:
    Rect bounds_;
};

} // namespace forgeui
