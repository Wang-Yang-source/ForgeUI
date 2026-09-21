#pragma once

#include <algorithm>
#include <cstdint>

namespace forgeui {

enum class Easing { Linear, Smoothstep, EaseOut };

class Tween {
public:
    void start(uint32_t now, uint32_t duration, Easing easing = Easing::Smoothstep,
               uint8_t from = 0, uint8_t to = 255) {
        started_ = now;
        duration_ = duration == 0 ? 1 : duration;
        easing_ = easing;
        from_ = from;
        to_ = to;
        active_ = true;
    }

    uint8_t value(uint32_t now) const {
        if (!active_) return to_;
        const uint32_t elapsed = now - started_;
        if (elapsed >= duration_) return to_;
        const uint32_t t = (elapsed * 255u) / duration_;
        uint32_t eased = t;
        if (easing_ == Easing::Smoothstep) eased = (t * t * (765u - 2u * t)) / 65025u;
        if (easing_ == Easing::EaseOut) eased = 255u - ((255u - t) * (255u - t) / 255u);
        const int32_t delta = static_cast<int32_t>(to_) - static_cast<int32_t>(from_);
        const int32_t result = static_cast<int32_t>(from_) + (delta * static_cast<int32_t>(eased)) / 255;
        return static_cast<uint8_t>(std::clamp(result, 0, 255));
    }

    bool active(uint32_t now) const { return active_ && now - started_ < duration_; }
    void cancel() { active_ = false; }

private:
    uint32_t started_ = 0, duration_ = 1;
    uint8_t from_ = 0, to_ = 255;
    Easing easing_ = Easing::Smoothstep;
    bool active_ = false;
};

} // namespace forgeui
