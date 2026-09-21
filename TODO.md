# ForgeUI TODO

ForgeUI is being built as a pixel-first embedded GUI framework with a Qt
desktop editor. The default target is a colorful ESP32 display, while 52×52
and 128×64 panels must remain lightweight.

## 0. Current baseline

- [x] Public GitHub repository
- [x] Non-commercial license and usage notice
- [x] Fixed-size RGB canvas
- [x] Packed 1-bit `MonoCanvas`
- [x] Packed RGB565 `Rgb565Canvas`
- [x] Time-based `Tween`
- [x] Fixed-capacity concurrent `Timeline`
- [x] Input events and static component container
- [x] Qt desktop editor vertical slice
- [x] Custom canvas size and nearest-pixel preview
- [x] JSON scene save/load
- [x] Serial scene packet sender
- [x] Shared FUI1 packet codec and CRC32 validation
- [x] Allocation-free ESP32 transport receiver
- [x] ESP32 and hot-update architecture notes

## 1. Animation engine

- [ ] Add `AnimationTrack` types for position, size, color, opacity, and text progress
- [ ] Add keyframes and timeline markers
- [ ] Add `Sequence`, `Parallel`, `Delay`, `Repeat`, and `PingPong` composition helpers
- [ ] Add per-track completion state and cancellation
- [ ] Add deterministic integer/fixed-point easing curves
- [ ] Add typewriter animation with uneven, configurable character timing
- [ ] Add dirty-region reporting for animated components
- [ ] Add animation golden tests for 2.5-second boot sequences
- [ ] Add frame pacing independent from animation time

## 2. Embedded runtime

- [ ] Define a zero-dependency `DisplayBackend` implementation guide
- [ ] Add RGB565 byte-order and rotation helpers
- [ ] Add dirty rectangle collection and region merging
- [ ] Add optional double-buffer and partial-flush support
- [ ] Add bitmap and packed-pixel asset formats
- [ ] Add compact bitmap-font renderer
- [ ] Add image clipping and nearest-neighbor scaling
- [ ] Add focus navigation for buttons, lists, and encoder input
- [ ] Add optional layout helpers: row, column, grid, and anchor
- [ ] Add compile-time feature switches for Nano/Core/Full builds
- [ ] Measure RAM, flash, render time, and frame rate on ESP32-S3

## 3. ESP32 support

- [ ] Add an ESP-IDF component wrapper
- [ ] Add an Arduino-ESP32 example
- [ ] Add an `esp_lcd` RGB565 adapter
- [ ] Add SPI DMA flush support
- [ ] Add HUB75 scan/DMA backend boundary
- [ ] Add ESP32 input adapters for GPIO buttons and rotary encoders
- [ ] Add LittleFS asset loading
- [ ] Add watchdog-safe rendering and reconnect handling
- [ ] Document ESP32, ESP32-S3, and ESP32-C3 resource profiles

## 4. Scene model

- [ ] Define a versioned scene schema
- [ ] Add screen metadata: color format, rotation, safe area, refresh mode
- [ ] Add node z-order and parent/child relationships
- [ ] Add labels, images, panels, buttons, sliders, lists, and progress bars
- [ ] Add style tokens and reusable themes
- [ ] Add constraints and responsive layout properties
- [ ] Add scene migration between schema versions
- [ ] Add asset IDs and content hashes
- [ ] Add undo/redo command history

## 5. Desktop editor

- [x] Replace the prototype preview with a selectable pixel canvas
- [x] Add node selection, dragging, and deletion
- [ ] Add resize handles, snap-to-grid, and multi-select
- [ ] Add layers/tree panel
- [x] Add basic property inspector for geometry and text
- [x] Add property inspector for node colors
- [ ] Add property inspector for styles
- [ ] Add color palette optimized for RGB565 and LED panels
- [ ] Add font and image asset browser
- [ ] Add animation timeline and keyframe editing
- [ ] Add play, pause, scrub, loop, and frame-step controls
- [ ] Add device profile templates: 52×52, 128×64, HUB75, TFT
- [ ] Add desktop preview controls for brightness, rotation, and pixel scale
- [ ] Add scene validation and resource budget warnings
- [ ] Add export preview as PNG/GIF

## 6. Live preview and hot update

- [x] Implement ESP32 `FUI1` receiver
- [x] Validate packet version, payload length, and CRC32
- [ ] Add ACK/NACK and error messages
- [ ] Add scene revision conflict handling
- [ ] Add double-buffered scene replacement at frame boundaries
- [ ] Add serial auto-detection and reconnect
- [ ] Add Wi-Fi transport over WebSocket or HTTP
- [ ] Add LittleFS asset upload by content hash
- [ ] Add a safe-mode scene if an update fails
- [ ] Keep firmware OTA separate from UI scene hot reload

## 7. Exporters

- [ ] Export compact ForgeUI C++ scene code
- [ ] Export static RGB565 and monochrome assets
- [ ] Export LVGL C/C++ widget trees
- [ ] Export Slint scene files or generated bindings where licensing permits
- [ ] Export a standalone ESP-IDF example project
- [ ] Export an Arduino/PlatformIO example project
- [ ] Add generated-code version metadata
- [ ] Add generated resource size reports

## 8. Testing and quality

- [ ] Add CTest unit tests for canvas, timeline, scene model, and packet codec
- [ ] Add host-side golden-frame tests
- [ ] Add malformed JSON and malformed packet tests
- [ ] Add sanitizer builds for the desktop editor
- [ ] Add cross-compilation checks for ESP-IDF and Arduino
- [ ] Add CI for Linux, Windows, and macOS editor builds
- [ ] Add performance benchmark scenes
- [ ] Add documentation for the non-commercial license and third-party notices

## 9. Release milestones

### v0.1 — Pixel runtime

- RGB565/1-bit canvas
- timeline animations
- bitmap font and image support
- ESP32 example

### v0.2 — Usable editor

- selectable canvas nodes
- property inspector
- scene save/load
- timeline preview
- ForgeUI export

### v0.3 — Device workflow

- ESP32 receiver
- serial hot update
- asset upload
- safe rollback scene

### v0.4 — Multi-runtime export

- LVGL exporter
- Slint exporter evaluation
- resource budget reports
- device profile system

### v1.0 — Stable pixel GUI platform

- documented scene schema
- stable runtime API
- tested ESP32 backends
- reproducible editor packages
- complete examples and migration guide

## Licensing constraint

ForgeUI is public source-available software for non-commercial use only. Keep
third-party code and generated output licensing explicit. Slint embedded use
requires GPLv3 or a commercial license; do not copy Slint runtime code into
ForgeUI under the custom non-commercial license.
