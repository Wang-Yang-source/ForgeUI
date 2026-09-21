#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace forgeui {

template <size_t MaxCharacters>
class Typewriter {
public:
    bool configure(const uint16_t* delays, size_t count, uint32_t start, uint16_t hold = 0) {
        if (!delays || count > MaxCharacters) return false;
        count_ = count; start_ = start; hold_ = hold;
        uint32_t accumulated = 0;
        for (size_t i = 0; i < count_; ++i) { accumulated += delays[i]; revealAt_[i] = accumulated; }
        return true;
    }
    size_t visible(uint32_t now) const {
        if (now < start_) return 0;
        const uint32_t elapsed = now - start_;
        size_t result = 0;
        while (result < count_ && elapsed >= revealAt_[result]) ++result;
        return result;
    }
    bool finished(uint32_t now) const {
        return count_ == 0 || (now >= start_ && now - start_ >= revealAt_[count_ - 1] + hold_);
    }
    size_t count() const { return count_; }
private:
    std::array<uint32_t, MaxCharacters> revealAt_{};
    size_t count_ = 0;
    uint32_t start_ = 0;
    uint16_t hold_ = 0;
};

} // namespace forgeui
