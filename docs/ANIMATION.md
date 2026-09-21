# Pixel animation model

ForgeUI borrows the useful idea behind Slint's property animations: animation
should describe how a property changes, while the renderer decides when the
next frame is flushed. ForgeUI implements this independently with a fixed-
capacity `Timeline` so the same model works on small ESP32 and HUB75 targets.

```cpp
int32_t logoY = -12;
int32_t typedChars = 0;
forgeui::Timeline<8> intro;
intro.add(logoY, -12, 18, 0, 800);
intro.add(typedChars, 0, 7, 250, 1200, forgeui::Easing::EaseOut);
intro.start(now);

// Called from the display task using a monotonic timestamp.
intro.update(now);
```

Tracks are concurrent by default. `delay` creates a staggered sequence, and
`RepeatMode::Loop` or `RepeatMode::PingPong` handles repeating effects. There
are no per-frame allocations or callbacks. A future editor can generate these
tracks from a timeline panel and export them to ForgeUI, LVGL, or Slint.

The implementation intentionally does not copy Slint source code. It is a
small, integer-friendly animation layer designed for pixel displays.
