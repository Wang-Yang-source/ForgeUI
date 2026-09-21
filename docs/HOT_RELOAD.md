# Hot update design

ForgeUI separates two update mechanisms:

## UI hot reload

The editor sends a versioned scene package to the running ESP32 over USB
serial, Wi-Fi, or WebSocket. The device validates the package, builds the next
scene in a second buffer, and swaps it at a frame boundary. The current frame
continues rendering while the new scene is checked.

The first package format should contain:

```text
magic | protocol_version | scene_revision | payload_length | CRC32 | payload
```

The payload can start as JSON for editor convenience and move to CBOR or a
compact binary format for production devices. Images and fonts should live in
LittleFS/FATFS and be referenced by content hash, so changing a label does not
re-upload every asset.

The shared packet codec is available in `include/forgeui/Protocol.h`. ESP32
transport code can feed incoming UART, USB, or WebSocket bytes into the fixed-
capacity `forgeui::esp32::SceneReceiver` from
`examples/esp32/SceneReceiver.h`. It only exposes a scene after the complete
packet and CRC32 have been validated.

For rendering, wrap the receiver in `DoubleSceneStore`. The transport task
feeds bytes into the inactive slot; the display task calls
`commitAtFrameBoundary()` after the current frame has finished. This keeps the
active scene stable during upload and makes the swap atomic from the renderer's
point of view.

The current packet is little-endian and has this layout:

```text
4 bytes  FUI1
1 byte   protocol version
4 bytes  scene revision
4 bytes  payload length
4 bytes  CRC32(payload)
N bytes  JSON or compact scene payload
```

## Firmware OTA

Firmware replacement is a separate ESP32 OTA feature using the platform's
dual-partition update flow. It must verify the image before rebooting and keep
the previous partition available for rollback. UI hot reload must never rewrite
the firmware partition.

This separation gives the editor a fast preview loop while keeping field
updates recoverable and safe.
