#pragma once

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "Protocol.h"

namespace forgeui {

template <size_t MaxAssets, size_t MaxBytesPerAsset>
class AssetStore {
public:
    struct View {
        uint32_t id = 0;
        protocol::AssetFormat format = protocol::AssetFormat::Rgb888;
        uint16_t width = 0, height = 0;
        const uint8_t* data = nullptr;
        size_t size = 0;
    };

    bool applyChunk(const uint8_t* packet, size_t packetSize) {
        protocol::AssetChunkView chunk;
        if (!protocol::decodeAssetChunk(packet, packetSize, chunk) || chunk.totalSize > MaxBytesPerAsset)
            return false;
        Slot* slot = find(chunk.assetId);
        if (!slot) slot = allocate(chunk.assetId);
        if (!slot || (chunk.offset == 0 && !begin(*slot, chunk))) return false;
        if (chunk.offset != 0 && slot->expected != chunk.assetCrc) return false;
        if (chunk.offset + chunk.payloadSize > slot->totalSize) return false;
        for (size_t i = 0; i < chunk.payloadSize; ++i) slot->bytes[chunk.offset + i] = chunk.payload[i];
        slot->received = std::max(slot->received, static_cast<size_t>(chunk.offset + chunk.payloadSize));
        if (slot->received == slot->totalSize) {
            slot->committed = protocol::crc32(slot->bytes.data(), slot->totalSize) == slot->expected;
        }
        return true;
    }

    bool contains(uint32_t id) const { return findConst(id) && findConst(id)->committed; }
    View view(uint32_t id) const {
        const Slot* slot = findConst(id);
        if (!slot || !slot->committed) return {};
        return {slot->id, slot->format, slot->width, slot->height, slot->bytes.data(), slot->totalSize};
    }

private:
    struct Slot {
        uint32_t id = 0;
        protocol::AssetFormat format = protocol::AssetFormat::Rgb888;
        uint16_t width = 0, height = 0;
        size_t totalSize = 0, received = 0;
        uint32_t expected = 0;
        bool used = false, committed = false;
        std::array<uint8_t, MaxBytesPerAsset> bytes{};
    };

    bool begin(Slot& slot, const protocol::AssetChunkView& chunk) {
        slot.format = chunk.format; slot.width = chunk.width; slot.height = chunk.height;
        slot.totalSize = chunk.totalSize; slot.received = 0; slot.expected = chunk.assetCrc; slot.committed = false;
        return true;
    }
    Slot* find(uint32_t id) { for (auto& slot : slots_) if (slot.used && slot.id == id) return &slot; return nullptr; }
    const Slot* findConst(uint32_t id) const { for (const auto& slot : slots_) if (slot.used && slot.id == id) return &slot; return nullptr; }
    Slot* allocate(uint32_t id) {
        for (auto& slot : slots_) if (!slot.used) { slot = {}; slot.used = true; slot.id = id; return &slot; }
        return nullptr;
    }
    std::array<Slot, MaxAssets> slots_{};
};

} // namespace forgeui
