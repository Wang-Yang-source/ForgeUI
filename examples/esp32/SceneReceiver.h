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
    void consume() { ready_ = false; }
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
