# ForgeUI architecture

ForgeUI targets two different use cases: tiny pixel displays with strict RAM
limits, and richer embedded interfaces that benefit from LVGL's widget and
layout ecosystem. The project therefore uses layers with a small dependency
surface instead of making every target depend on the full stack.

## Layers

### Nano

`MonoCanvas` and the fixed-size `Canvas` are low-level render targets. They own
their storage and do not allocate. A display adapter only needs to consume
`data()` and implement the panel-specific flush operation.

### Core

`Component`, `InputEvent`, `Tween`, and `StaticContainer` provide a small
retained component model. Components have separate `update`, `draw`, and
`handle` phases. Containers borrow child objects; ownership stays with the
application, keeping lifetime and memory usage explicit on embedded targets.

### Full and editor integration

The editor should store a device-independent scene model. Exporters can map the
same scene to either a compact ForgeUI component tree or LVGL C/C++ calls. LVGL
is an optional integration target, not a dependency of the Nano/Core runtime.

## Performance rules

1. Use integer or fixed-point geometry on embedded targets.
2. Drive animation from a monotonic millisecond timestamp, never frame count.
3. Keep ownership explicit; use fixed-capacity containers for small targets.
4. Add dirty-region tracking before enabling expensive widgets.
5. Keep layout, data binding, charts, and themes optional at compile time.

## Planned interfaces

The next layers should add focus navigation, a dirty-region renderer, and an
LVGL exporter. These should be separate headers
so a 52×52 firmware build can include only Nano/Core pieces.
