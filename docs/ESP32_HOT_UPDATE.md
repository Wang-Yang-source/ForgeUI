# ESP32-S3 hot update path

ForgeUI Studio sends two binary packet types over the same serial link:

- `FUI1`: a versioned JSON scene with revision and payload CRC32.
- `FUA1`: an asset chunk with asset id, format, dimensions, chunk CRC32, and
  complete-asset CRC32.

The PixelPad FM6373C firmware owns UART0 from its display task. It validates
both CRCs, commits scene data only after JSON parsing succeeds, stores up to
four 8 KiB assets in PSRAM, and renders RGB888, RGB565, 1-bit, and bitmap-mask
assets. Scene animation properties currently supported on-device are `x`, `y`,
`width`, `height`, `opacity`, and `textProgress`.

The HUB75 timing and DMA remain in the FM6373C driver. ForgeUI supplies a
complete RGB888 frame through `Hub75Backend`, so the GUI layer does not depend
on a particular scan-chain implementation.

## Verification

On the 52×52 ESP32-S3 target:

```text
FM6373C ready: 52x52, 26 scan, 4 IC/lane, ... frame=60.08 Hz
FORGEUI ACK asset id=7 bytes=12
FORGEUI ACK scene revision=45 bytes=493
```

The scene and asset were sent without reflashing after boot. Firmware upload
uses PlatformIO's `pixelpad_s3_fm6373c_rtos` environment and `/dev/ttyUSB0`.
