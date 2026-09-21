# ESP32 integration

ForgeUI keeps ESP32 support platform-neutral. The core headers do not include
Arduino or ESP-IDF headers, so the same scene code can run in an ESP-IDF task,
an Arduino sketch, or the desktop preview.

For color displays, use `Rgb565Canvas<W, H>`. It stores exactly two bytes per
pixel and exposes a contiguous `data()` buffer suitable for SPI DMA or an
`esp_lcd` draw-bitmap call:

```cpp
#include <forgeui/ForgeUI.h>

forgeui::Rgb565Canvas<240, 240> canvas;

void render() {
    canvas.clear(forgeui::Color::black());
    canvas.box(8, 8, 231, 231, forgeui::Color::cyan());
    // The backend owns the actual esp_lcd_panel_handle_t or TFT_eSPI object.
    // backend.flush({0, 0, 240, 240}, canvas.data(), 240);
}
```

Recommended ESP32 targets:

- ESP-IDF for production firmware and `esp_lcd`/SPI DMA;
- Arduino-ESP32 for quick prototypes and board bring-up;
- LittleFS or FATFS for UI scenes, fonts, and images;
- a monotonic millisecond clock passed into `update()` and `Tween`.

The display backend should own byte order, DMA alignment, rotation, and panel
specific color settings. Those details should not leak into the UI widgets.
