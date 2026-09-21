#include <cassert>
#include <cstdint>
#include <cstring>

#include <forgeui/Protocol.h>
#include <forgeui/AssetStore.h>
#include <examples/esp32/SceneReceiver.h>

int main() {
    const char text[] = "{\"width\":52}";
    uint8_t packet[128]{};
    assert(forgeui::protocol::encodeScene(packet, sizeof(packet), 42,
                                          reinterpret_cast<const uint8_t*>(text), sizeof(text) - 1));

    forgeui::protocol::PacketView view;
    assert(forgeui::protocol::decodeScene(packet, forgeui::protocol::encodedSize(sizeof(text) - 1), view));
    assert(view.revision == 42 && view.payloadSize == sizeof(text) - 1);
    assert(std::memcmp(view.payload, text, view.payloadSize) == 0);

    forgeui::esp32::SceneReceiver<128> receiver;
    for (size_t i = 0; i < forgeui::protocol::encodedSize(sizeof(text) - 1); ++i)
        receiver.feed(packet + i, 1);
    assert(receiver.ready() && receiver.revision() == 42);
    assert(std::memcmp(receiver.payload(), text, receiver.payloadSize()) == 0);
    receiver.consume();
    assert(!receiver.ready());

    forgeui::esp32::DoubleSceneStore<128> store;
    assert(store.feed(packet, forgeui::protocol::encodedSize(sizeof(text) - 1)));
    assert(store.pending() && store.activeSize() == 0);
    assert(store.commitAtFrameBoundary());
    assert(!store.pending() && store.activeRevision() == 42);
    assert(std::memcmp(store.activePayload(), text, sizeof(text) - 1) == 0);

    packet[forgeui::protocol::headerSize] ^= 1u;
    assert(!forgeui::protocol::decodeScene(packet, forgeui::protocol::encodedSize(sizeof(text) - 1), view));

    const uint8_t asset[] = {1, 2, 3, 4, 5, 6};
    const uint32_t assetCrc = forgeui::protocol::crc32(asset, sizeof(asset));
    uint8_t a[128]{};
    assert(forgeui::protocol::encodeAssetChunk(a, sizeof(a), 7, 0, sizeof(asset),
        forgeui::protocol::AssetFormat::Rgb888, 2, 1, assetCrc, asset, 3));
    uint8_t b[128]{};
    assert(forgeui::protocol::encodeAssetChunk(b, sizeof(b), 7, 3, sizeof(asset),
        forgeui::protocol::AssetFormat::Rgb888, 2, 1, assetCrc, asset + 3, 3));
    forgeui::AssetStore<2, 16> assets;
    assert(assets.applyChunk(a, forgeui::protocol::encodedAssetSize(3)));
    assert(!assets.contains(7));
    assert(assets.applyChunk(b, forgeui::protocol::encodedAssetSize(3)));
    assert(assets.contains(7) && assets.view(7).size == sizeof(asset));
    assert(std::memcmp(assets.view(7).data, asset, sizeof(asset)) == 0);
    b[forgeui::protocol::assetHeaderSize] ^= 1u;
    assert(!assets.applyChunk(b, forgeui::protocol::encodedAssetSize(3)));
    return 0;
}
