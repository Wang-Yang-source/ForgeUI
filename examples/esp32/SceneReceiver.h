#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <forgeui/Protocol.h>

namespace forgeui::esp32 {

// Transport-independent receiver for UART, WebSocket, or USB CDC bytes.
// It never allocates and only publishes a scene after the complete packet and
// CRC have been validated.
template <size_t MaxPayload = protocol::maxScenePayload>
class SceneReceiver {
public:
    bool feed(const uint8_t* bytes, size_t size) {
        for (size_t i = 0; i < size; ++i) feedByte(bytes[i]);
        return ready_;
    }

    bool ready() const { return ready_; }
    uint32_t revision() const { return revision_; }
    const uint8_t* payload() const { return payload_.data(); }
    size_t payloadSize() const { return payloadSize_; }
    void consume() { reset(); }
    void reset() { resetPacket(); ready_ = false; }

private:
    void feedByte(uint8_t byte) {
        if (ready_) return;
        if (received_ == 0 && byte != 'F') return;
        if (received_ == 1 && byte != 'U') { resetPacket(); return; }
        if (received_ == 2 && byte != 'I') { resetPacket(); return; }
        if (received_ == 3 && byte != '1') { resetPacket(); return; }
        if (received_ == 4 && byte != protocol::version) { resetPacket(); return; }
        if (received_ < protocol::headerSize) header_[received_] = byte;
        else if (payloadSize_ <= MaxPayload && payloadReceived_ < payloadSize_) payload_[payloadReceived_++] = byte;
        ++received_;

        if (received_ == protocol::headerSize) {
            revision_ = protocol::readLe32(header_.data() + 5);
            payloadSize_ = protocol::readLe32(header_.data() + 9);
            if (payloadSize_ > MaxPayload) resetPacket();
        } else if (received_ >= protocol::headerSize && payloadReceived_ == payloadSize_) {
            const uint32_t expected = protocol::readLe32(header_.data() + 13);
            if (protocol::crc32(payload_.data(), payloadSize_) == expected) ready_ = true;
            else resetPacket();
        }
    }

    void resetPacket() {
        received_ = 0;
        payloadReceived_ = 0;
        payloadSize_ = 0;
    }

    std::array<uint8_t, protocol::headerSize> header_{};
    std::array<uint8_t, MaxPayload> payload_{};
    size_t received_ = 0;
    size_t payloadReceived_ = 0;
    size_t payloadSize_ = 0;
    uint32_t revision_ = 0;
    bool ready_ = false;
};

} // namespace forgeui::esp32

namespace forgeui::esp32 {

// Owns two validated scene slots. `commitAtFrameBoundary()` is the only point
// where the active scene changes, so the display task never sees a partial
// update while the transport task is receiving bytes.
template <size_t MaxPayload = protocol::maxScenePayload>
class DoubleSceneStore {
public:
    bool feed(const uint8_t* bytes, size_t size) {
        receiver_.feed(bytes, size);
        if (!receiver_.ready()) return false;
        const size_t next = active_ == 0 ? 1 : 0;
        if (receiver_.payloadSize() > MaxPayload) {
            receiver_.consume();
            return false;
        }
        for (size_t i = 0; i < receiver_.payloadSize(); ++i) slots_[next][i] = receiver_.payload()[i];
        pendingSize_ = receiver_.payloadSize();
        pendingRevision_ = receiver_.revision();
        pendingSlot_ = next;
        pending_ = true;
        receiver_.consume();
        return true;
    }

    bool commitAtFrameBoundary() {
        if (!pending_) return false;
        active_ = pendingSlot_;
        activeSize_ = pendingSize_;
        activeRevision_ = pendingRevision_;
        pending_ = false;
        return true;
    }

    const uint8_t* activePayload() const { return slots_[active_].data(); }
    size_t activeSize() const { return activeSize_; }
    uint32_t activeRevision() const { return activeRevision_; }
    bool pending() const { return pending_; }

private:
    SceneReceiver<MaxPayload> receiver_;
    std::array<std::array<uint8_t, MaxPayload>, 2> slots_{};
    size_t active_ = 0;
    size_t pendingSlot_ = 1;
    size_t activeSize_ = 0;
    size_t pendingSize_ = 0;
    uint32_t activeRevision_ = 0;
    uint32_t pendingRevision_ = 0;
    bool pending_ = false;
};

} // namespace forgeui::esp32
