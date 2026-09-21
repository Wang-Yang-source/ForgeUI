# Slint review for ForgeUI

The upstream Slint project is a useful reference for declarative properties,
transitions, dirty rendering, software rendering, and ESP-IDF integration. Its
ESP integration supports RGB565/RGB8, single or double buffers, line-by-line
rendering, rotation, and `esp_lcd` panel flushing.

Those concepts fit the target hardware, but directly embedding Slint is not a
good default for ForgeUI:

1. A 52×52 panel often needs deterministic pixel control more than a complete
   widget runtime.
2. HUB75 scanning is not the same as an `esp_lcd` panel flush and still needs a
   dedicated backend.
3. The Slint runtime uses Rust/`alloc` internally and adds a larger toolchain
   and memory surface than ForgeUI Nano.
4. Slint's embedded framework use is available under GPLv3 or a commercial
   license; its royalty-free option excludes embedded systems. That does not
   fit ForgeUI's current custom non-commercial license without a licensing
   change.

ForgeUI therefore adopts the architecture ideas, not the implementation:
property tracks, time-based transitions, dirty-region boundaries, RGB565
buffers, and an explicit display backend.
