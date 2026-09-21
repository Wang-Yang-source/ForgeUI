#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "Component.h"

namespace forgeui {

// Small, allocation-free layout helpers. Full layout engines stay optional.
template <typename CanvasT, size_t MaxChildren>
class Column;

template <typename CanvasT, size_t MaxChildren>
class Row;

template <typename CanvasT, size_t MaxChildren>
class StaticContainer : public Component<CanvasT> {
public:
    using Child = Component<CanvasT>;

    bool add(Child& child) {
        if (count_ == MaxChildren) return false;
        children_[count_++] = &child;
        return true;
    }

    void update(uint32_t now) override {
        for (size_t i = 0; i < count_; ++i) children_[i]->update(now);
    }

    void draw(CanvasT& canvas) const override {
        for (size_t i = 0; i < count_; ++i) children_[i]->draw(canvas);
    }

    bool handle(const InputEvent& event) override {
        bool handled = false;
        for (size_t i = 0; i < count_; ++i) handled = children_[i]->handle(event) || handled;
        return handled;
    }

    size_t count() const { return count_; }

protected:
    std::array<Child*, MaxChildren> children_{};
    size_t count_ = 0;
};

} // namespace forgeui
