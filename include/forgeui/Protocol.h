#pragma once

#include <cstddef>
#include <cstdint>

namespace forgeui {

namespace protocol {
constexpr uint8_t version = 1;
constexpr size_t headerSize = 17;
constexpr uint32_t maxScenePayload = 64u * 1024u;

struct PacketView {
    uint32_t revision = 0;
    const uint8_t* payload = nullptr;
    size_t payloadSize = 0;
};

inline uint32_t crc32(const uint8_t* data, size_t size) {
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1u) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

inline void writeLe32(uint8_t* out, uint32_t value) {
    out[0] = static_cast<uint8_t>(value);
    out[1] = static_cast<uint8_t>(value >> 8u);
    out[2] = static_cast<uint8_t>(value >> 16u);
    out[3] = static_cast<uint8_t>(value >> 24u);
}

inline uint32_t readLe32(const uint8_t* in) {
    return static_cast<uint32_t>(in[0]) |
           (static_cast<uint32_t>(in[1]) << 8u) |
           (static_cast<uint32_t>(in[2]) << 16u) |
           (static_cast<uint32_t>(in[3]) << 24u);
}

inline size_t encodedSize(size_t payloadSize) { return headerSize + payloadSize; }

inline bool encodeScene(uint8_t* out, size_t capacity, uint32_t revision,
                        const uint8_t* payload, size_t payloadSize) {
    if (out == nullptr || payload == nullptr || payloadSize > maxScenePayload ||
        capacity < encodedSize(payloadSize)) return false;
    out[0] = 'F'; out[1] = 'U'; out[2] = 'I'; out[3] = '1';
    out[4] = version;
    writeLe32(out + 5, revision);
    writeLe32(out + 9, static_cast<uint32_t>(payloadSize));
    writeLe32(out + 13, crc32(payload, payloadSize));
    for (size_t i = 0; i < payloadSize; ++i) out[headerSize + i] = payload[i];
    return true;
}

inline bool decodeScene(const uint8_t* packet, size_t packetSize, PacketView& view) {
    if (packet == nullptr || packetSize < headerSize || packet[0] != 'F' ||
        packet[1] != 'U' || packet[2] != 'I' || packet[3] != '1' ||
        packet[4] != version) return false;
    const uint32_t payloadSize = readLe32(packet + 9);
    if (payloadSize > maxScenePayload || packetSize != encodedSize(payloadSize)) return false;
    const uint8_t* payload = packet + headerSize;
    if (crc32(payload, payloadSize) != readLe32(packet + 13)) return false;
    view.revision = readLe32(packet + 5);
    view.payload = payload;
    view.payloadSize = payloadSize;
    return true;
}

} // namespace protocol
} // namespace forgeui
