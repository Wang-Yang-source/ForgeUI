# ForgeUI

ForgeUI is a tiny, pixel-first GUI foundation for editable embedded OLED and
LED displays. It is designed for small monochrome, grayscale, and RGB panels
where predictable memory use and allocation-free rendering matter more than a
large widget toolkit.

The first release focuses on a clean core that can be connected to any display
driver:

- fixed-size frame canvas with no heap allocation;
- packed 1-bit canvas for low-memory monochrome panels;
- packed RGB565 canvas for ESP32 and small color displays;
- primitive drawing for pixels, rectangles, boxes, and lines;
- lightweight easing and time-based animation helpers;
- composable components with explicit `update()` and `draw()` phases;
- allocation-free input events and fixed-capacity component containers;
- no dependency on Arduino, FreeRTOS, LVGL, or a particular MCU;
- easy to adapt to OLED, HUB75, RGB LED, and custom framebuffer backends.

ForgeUI is source-available for non-commercial use only. It may be used for
personal projects, education, research, and maker projects. Commercial use,
commercial distribution, paid products, and business deployment require prior
written permission. See [LICENSE](LICENSE) and [NOTICE](NOTICE).

Because commercial use is restricted, this is a public source-available project
rather than an OSI-approved open-source license.

## Quick start

```cpp
#include <forgeui/ForgeUI.h>

forgeui::Canvas<52, 52> canvas;
forgeui::Tween progress;

void begin(uint32_t now) {
    progress.start(now, 1200, forgeui::Easing::Smoothstep);
}

void render(uint32_t now) {
    canvas.clear();
    canvas.box(4, 4, 44, 44, forgeui::Color::white());
    canvas.fill(8, 8, 8 + progress.value(now) * 28 / 255, 12,
                forgeui::Color::cyan());
    // Send canvas.data() to the display driver here.
}
```

## Build the host demo

```sh
cmake -S . -B build
cmake --build build
./build/forgeui_host_demo
```

## Relationship to oled-ui-astra

ForgeUI takes inspiration from the supplied OLED UI project at the level of
ideas: small embedded primitives, frame-based animation, and direct display
control. It is an independent implementation with a different API and source
layout. The upstream project remains under its own GPL-3.0 license.

## Layered runtime

ForgeUI is designed as a tiered runtime instead of one mandatory widget stack:

- **Nano**: `MonoCanvas`, static components, direct drawing, and time-based animation for 52×52 or 128×64 displays.
- **Core**: input events, retained components, containers, menus, and optional layout helpers.
- **Full**: richer widgets, data binding, themes, and an LVGL export/adapter layer for larger displays.

Features should be opt-in so a small target does not pay for the Full layer. The editor can use one scene model and export either a compact ForgeUI runtime or LVGL C/C++ code.

See [the architecture notes](docs/ARCHITECTURE.md) for the layer boundaries.

ESP32 setup notes are in [docs/ESP32.md](docs/ESP32.md). The planned editor
hot-update protocol is described in [docs/HOT_RELOAD.md](docs/HOT_RELOAD.md).

## Roadmap

- 1-bit and RGB565 canvas adapters;
- compact bitmap and tiny-font helpers;
- retained-mode widgets for menus and status panels;
- input focus/navigation primitives;
- LVGL scene exporter and desktop preview;
- optional Arduino and PlatformIO adapters;
- host-side golden-frame tests.
