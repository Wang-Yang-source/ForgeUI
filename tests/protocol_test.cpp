#include <cassert>
#include <cstdint>
#include <cstring>

#include <forgeui/Protocol.h>
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
    return 0;
}
