# ForgeUI Studio desktop editor

## Decision

The first desktop editor uses Qt 6 Widgets with a custom `QPainter` preview.
This matches the current C++ library, runs well on Linux, and gives us native
serial support through `QSerialPort`. The preview intentionally uses nearest-
pixel rendering and a visible grid so a 52×52 design looks like the target
panel rather than a smoothed web image.

Qt Quick/QML remains a possible later presentation layer if the property
inspector and timeline become more elaborate. The scene model, serial protocol,
and exporter must stay independent of the presentation layer.

Tauri/Web is not the first implementation because it would add a second
language/runtime and make a native LVGL/ForgeUI preview bridge more involved.
It remains a good option for a future web-based editor or documentation viewer.

## Current vertical slice

The `forgeui_studio` target currently provides:

- configurable canvas dimensions;
- nearest-pixel preview with optional grid;
- box and pixel-label scene nodes;
- JSON save/load;
- serial port discovery and connection;
- versioned scene push using the `FUI1` packet described in `HOT_RELOAD.md`.

Build it when Qt 5 or Qt 6 is installed:

```sh
cmake -S . -B build -DFORGEUI_BUILD_STUDIO=ON
cmake --build build --target forgeui_studio
./build/forgeui_studio
```

Without Qt, the embedded library and host demo continue to build and the
optional editor is skipped.

## Next editor milestones

1. Select, move, resize, duplicate, and delete scene nodes.
2. Add a property inspector and color/font/image asset panels.
3. Add a timeline for the existing `Tween` model and typewriter effects.
4. Add a real LVGL simulator/exporter target.
5. Add ESP32 receiver code for `FUI1`, CRC validation, double-buffered scene
   replacement, and LittleFS asset caching.
