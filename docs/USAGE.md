# Using ForgeUI on an embedded display

ForgeUI intentionally stops at the framebuffer boundary. A display adapter
should copy or encode `Canvas::data()` into the target OLED or LED driver.
That keeps timing, DMA, bus locking, and panel-specific initialization out of
the UI core.

For a 1-bit OLED, map each pixel to a bit in the driver's page buffer. For an
RGB LED panel, map `Color` values into the driver's native order and submit a
frame at the panel's frame boundary. Keep `update()` separate from `draw()` so
input and animation state are deterministic and rendering remains repeatable.

Do not allocate from a render loop. Prefer one statically owned canvas and one
small component tree per screen.
