#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Tween.h"

namespace forgeui {

enum class RepeatMode : uint8_t { Once, Loop, PingPong };

inline uint32_t easingValue(uint32_t t, Easing easing) {
    if (easing == Easing::Smoothstep) return (t * t * (765u - 2u * t)) / 65025u;
    if (easing == Easing::EaseOut) return 255u - ((255u - t) * (255u - t) / 255u);
    return t;
}

// A fixed-capacity property timeline. Tracks run in parallel; delay composes
// staggered effects without a heap or a callback allocation.
template <size_t MaxTracks>
class Timeline {
public:
    bool add(int32_t& property, int32_t from, int32_t to, uint32_t delay,
             uint32_t duration, Easing easing = Easing::Smoothstep,
             RepeatMode repeat = RepeatMode::Once) {
        if (count_ == MaxTracks) return false;
        Track& track = tracks_[count_++];
        track.property = &property;
        track.from = from;
        track.to = to;
        track.delay = delay;
        track.duration = duration == 0 ? 1 : duration;
        track.easing = easing;
        track.repeat = repeat;
        track.finished = false;
        property = from;
        return true;
    }

    void start(uint32_t now) {
        started_ = now;
        running_ = true;
        for (size_t i = 0; i < count_; ++i) tracks_[i].finished = false;
    }

    bool update(uint32_t now) {
        if (!running_) return false;
        bool anyActive = false;
        for (size_t i = 0; i < count_; ++i) {
            if (!tracks_[i].update(now - started_)) continue;
            anyActive = true;
        }
        if (!anyActive) running_ = false;
        return running_;
    }

    void stop() { running_ = false; }
    bool active() const { return running_; }
    size_t count() const { return count_; }

private:
    struct Track {
        int32_t* property = nullptr;
        int32_t from = 0;
        int32_t to = 0;
        uint32_t delay = 0;
        uint32_t duration = 1;
        Easing easing = Easing::Smoothstep;
        RepeatMode repeat = RepeatMode::Once;
        bool finished = false;

        bool update(uint32_t elapsed) {
            if (finished || property == nullptr) return false;
            if (elapsed < delay) {
                *property = from;
                return true;
            }

            uint32_t local = elapsed - delay;
            const uint32_t cycle = duration;
            uint32_t cycleIndex = local / cycle;
            uint32_t position = local % cycle;
            if (repeat == RepeatMode::Once && cycleIndex > 0) {
                *property = to;
                finished = true;
                return false;
            }
            if (repeat == RepeatMode::Loop) {
                position = local % cycle;
            } else if (repeat == RepeatMode::PingPong) {
                if ((cycleIndex & 1u) != 0u) position = cycle - position;
            } else if (position == 0 && cycleIndex > 0) {
                position = cycle;
            }

            const uint32_t t = static_cast<uint32_t>((static_cast<uint64_t>(position) * 255u) / cycle);
            const int64_t delta = static_cast<int64_t>(to) - from;
            *property = static_cast<int32_t>(from + (delta * easingValue(t, easing)) / 255);
            return true;
        }
    };

    std::array<Track, MaxTracks> tracks_{};
    size_t count_ = 0;
    uint32_t started_ = 0;
    bool running_ = false;
};

} // namespace forgeui
