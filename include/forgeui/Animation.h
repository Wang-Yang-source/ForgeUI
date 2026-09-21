#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Canvas.h"
#include "Timeline.h"

namespace forgeui {

// Keyframe timeline for scene properties. It deliberately stores all state in
// caller-selected fixed-capacity arrays: embedded builds never allocate while
// an animation is running.
enum class AnimatedProperty : uint8_t { Position, Size, Color, Opacity, TextProgress };

// A heap-free composition cursor. Its returned start times can be added to
// keyframe timestamps, making sequences and parallel groups deterministic.
template <size_t MaxClips>
class AnimationSchedule {
public:
    struct Clip { uint32_t start = 0, duration = 0; RepeatMode repeat = RepeatMode::Once; };
    size_t sequence(uint32_t duration) { return append(cursor_, duration, RepeatMode::Once, true); }
    size_t parallel(uint32_t duration) { return append(cursor_, duration, RepeatMode::Once, false); }
    void delay(uint32_t duration) { cursor_ += duration; }
    bool repeat(size_t index) { return setRepeat(index, RepeatMode::Loop); }
    bool pingPong(size_t index) { return setRepeat(index, RepeatMode::PingPong); }
    uint32_t duration() const { return cursor_; }
    const Clip& clip(size_t index) const { return clips_[index]; }
    size_t count() const { return count_; }

private:
    size_t append(uint32_t start, uint32_t duration, RepeatMode repeatMode, bool advance) {
        if (count_ == MaxClips) return invalid;
        clips_[count_] = {start, duration, repeatMode};
        const size_t result = count_++;
        if (advance) cursor_ += duration;
        return result;
    }
    bool setRepeat(size_t index, RepeatMode mode) { if (index >= count_) return false; clips_[index].repeat = mode; return true; }
    static constexpr size_t invalid = static_cast<size_t>(-1);
    std::array<Clip, MaxClips> clips_{};
    size_t count_ = 0;
    uint32_t cursor_ = 0;
};

template <size_t MaxTracks, size_t MaxKeyframes = 8, size_t MaxMarkers = 16>
class AnimationTimeline {
public:
    struct Marker { uint32_t time = 0; uint16_t id = 0; };

    size_t addInt(int32_t& target, AnimatedProperty property = AnimatedProperty::Position,
                 RepeatMode repeat = RepeatMode::Once) {
        if (trackCount_ == MaxTracks) return invalid;
        Track& track = tracks_[trackCount_];
        track = {};
        track.target = &target; track.property = property; track.repeat = repeat;
        return trackCount_++;
    }

    size_t addColor(Color& target, RepeatMode repeat = RepeatMode::Once) {
        if (trackCount_ == MaxTracks) return invalid;
        Track& track = tracks_[trackCount_];
        track = {};
        track.target = &target; track.property = AnimatedProperty::Color; track.kind = ValueKind::Color; track.repeat = repeat;
        return trackCount_++;
    }

    bool addKeyframe(size_t index, uint32_t time, int32_t value, Easing easing = Easing::Smoothstep) {
        if (index >= trackCount_ || tracks_[index].kind != ValueKind::Int || tracks_[index].keyframeCount == MaxKeyframes) return false;
        Keyframe& key = tracks_[index].keyframes[tracks_[index].keyframeCount++];
        key.time = time; key.value = value; key.easing = easing;
        return true;
    }

    bool addKeyframe(size_t index, uint32_t time, Color value, Easing easing = Easing::Smoothstep) {
        if (index >= trackCount_ || tracks_[index].kind != ValueKind::Color || tracks_[index].keyframeCount == MaxKeyframes) return false;
        Keyframe& key = tracks_[index].keyframes[tracks_[index].keyframeCount++];
        key.time = time; key.color = value; key.easing = easing;
        return true;
    }

    bool addMarker(uint32_t time, uint16_t id) {
        if (markerCount_ == MaxMarkers) return false;
        markers_[markerCount_++] = {time, id};
        return true;
    }

    void start(uint32_t now) { started_ = now; running_ = true; for (size_t i = 0; i < trackCount_; ++i) tracks_[i].finished = false; }
    void cancel() { running_ = false; }
    void cancel(size_t index) { if (index < trackCount_) tracks_[index].finished = true; }
    bool active() const { return running_; }
    bool finished(size_t index) const { return index >= trackCount_ || tracks_[index].finished; }
    size_t count() const { return trackCount_; }
    static constexpr size_t invalid = static_cast<size_t>(-1);

    bool update(uint32_t now) {
        if (!running_) return false;
        const uint32_t elapsed = now - started_;
        bool any = false;
        for (size_t i = 0; i < trackCount_; ++i) any |= updateTrack(tracks_[i], elapsed);
        if (!any) running_ = false;
        return running_;
    }

private:
    enum class ValueKind : uint8_t { Int, Color };
    struct Keyframe { uint32_t time = 0; int32_t value = 0; Color color{}; Easing easing = Easing::Linear; };
    struct Track {
        void* target = nullptr;
        AnimatedProperty property = AnimatedProperty::Position;
        ValueKind kind = ValueKind::Int;
        RepeatMode repeat = RepeatMode::Once;
        std::array<Keyframe, MaxKeyframes> keyframes{};
        size_t keyframeCount = 0;
        bool finished = false;
    };

    static uint32_t sample(uint32_t t, Easing easing) {
        return easingValue(t, easing);
    }
    static bool updateTrack(Track& track, uint32_t elapsed) {
        if (track.finished || track.keyframeCount == 0) return false;
        const uint32_t end = track.keyframes[track.keyframeCount - 1].time;
        uint32_t local = elapsed;
        if (track.repeat == RepeatMode::Once && elapsed >= end) { local = end; track.finished = true; }
        else if (track.repeat != RepeatMode::Once && end != 0) {
            const uint32_t cycle = elapsed / end;
            local = elapsed % end;
            if (track.repeat == RepeatMode::PingPong && (cycle & 1u)) local = end - local;
        }
        if (local < track.keyframes[0].time) local = track.keyframes[0].time;
        size_t right = 1;
        while (right < track.keyframeCount && track.keyframes[right].time < local) ++right;
        if (right == track.keyframeCount) right = track.keyframeCount - 1;
        const size_t left = right == 0 ? 0 : right - 1;
        const Keyframe& a = track.keyframes[left];
        const Keyframe& b = track.keyframes[right];
        const uint32_t span = b.time > a.time ? b.time - a.time : 1;
        const uint32_t t = local <= a.time ? 0 : local >= b.time ? 255 : uint32_t((uint64_t(local - a.time) * 255u) / span);
        const uint32_t eased = sample(t, b.easing);
        if (track.kind == ValueKind::Int) {
            const int64_t value = int64_t(a.value) + (int64_t(b.value) - a.value) * eased / 255;
            *static_cast<int32_t*>(track.target) = static_cast<int32_t>(value);
        } else {
            Color& out = *static_cast<Color*>(track.target);
            out.r = uint8_t(int(a.color.r) + (int(b.color.r) - a.color.r) * eased / 255);
            out.g = uint8_t(int(a.color.g) + (int(b.color.g) - a.color.g) * eased / 255);
            out.b = uint8_t(int(a.color.b) + (int(b.color.b) - a.color.b) * eased / 255);
        }
        return !track.finished;
    }

    std::array<Track, MaxTracks> tracks_{};
    std::array<Marker, MaxMarkers> markers_{};
    size_t trackCount_ = 0, markerCount_ = 0;
    uint32_t started_ = 0;
    bool running_ = false;
};

} // namespace forgeui
