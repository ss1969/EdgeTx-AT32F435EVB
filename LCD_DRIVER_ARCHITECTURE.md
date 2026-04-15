# LCD Driver Architecture

## Purpose

This document summarizes the current QSPI2 LCD driver structure for the
`TXW430038B0 + NV3041A` panel and is intended as a quick reference for future
AI or human maintainers.

The code is intentionally split into:

- a low-level QSPI2 bus transport layer
- a panel driver layer
- an EdgeTX-facing adapter layer
- a temporary EdgeTX compatibility type shim

## File Layout

### Public / integration-facing headers

- `project/inc/edgetx_lcd_compat.h`
  - Temporary local compatibility definitions for `coord_t`, `BitmapBuffer`,
    `LcdFlags`, and bitmap formats.
  - Replace this header with real EdgeTX display/UI headers during integration.

- `project/inc/txw430038b0_nv3041a_01_spi.h`
  - EdgeTX-facing adapter interface.
  - Exposes display operations in the style expected by the UI layer:
    - `txw430038b0_nv3041a_01_getWidth`
    - `txw430038b0_nv3041a_01_getHeight`
    - `txw430038b0_nv3041a_01_spi_setWindow`
    - `txw430038b0_nv3041a_01_spi_drawBitmap`
    - `txw430038b0_nv3041a_01_spi_drawString`
    - `txw430038b0_nv3041a_01_spi_sleep`
    - `txw430038b0_nv3041a_01_spi_wakeup`
  - This is the preferred include for EdgeTX integration.

- `project/inc/txw430038b0_nv3041a_01_panel.h`
  - Panel-level primitives and configuration constants.
  - Exposes:
    - panel name
    - panel init
    - raw `set_window`
    - raw `write_pixels_rgb565`
    - panel width/height macros
    - bus-mode selection macros
  - Intended for board bring-up, tests, and low-level panel work.

- `project/inc/qspi2_lcd_bus.h`
  - Minimal public transport API for the QSPI2 LCD bus.
  - Exposes only:
    - `qspi2_lcd_bus_write`
    - `qspi2_lcd_bus_read`
    - `qspi2_lcd_bus_write_pixels_rgb565`
  - This header should remain narrow.

### Private / debug-oriented headers

- `project/inc/qspi2_lcd_bus_priv.h`
  - Internal transport helpers that should not be treated as stable public API.
  - Exposes:
    - reset pulse helper
    - TE wait helpers
    - raw debug read helper
    - diagnostic register capture structures/functions
  - Used by bring-up/demo code such as `main.c`.
  - Do not make higher UI layers depend on this header.

### Source files

- `project/src/qspi2_lcd_bus.c`
  - Implements QSPI2 command-port transfers for the LCD.
  - Responsibilities:
    - register/command write transactions
    - register/command read transactions
    - GRAM pixel streaming over QSPI2
    - EDMA chunking for pixel writes
    - transport-local timeout and error handling
  - Does not know about panel geometry, clipping, or bitmap layout policy.

- `project/src/txw430038b0_nv3041a_01_spi.c`
  - Implements both:
    - panel setup and raw panel operations
    - EdgeTX-style display adapter functions
  - Responsibilities:
    - panel init sequence
    - window programming
    - bitmap clipping
    - choosing fast path vs row-by-row fallback for bitmap drawing
  - This is where UI-level drawing policy belongs.

## Current Call Flow

### Initialization

1. Board init configures clocks/GPIO/QSPI/EDMA.
2. `txw430038b0_nv3041a_01_init()`
3. Panel init sequence sends many register writes through `qspi2_lcd_bus_write()`
4. Default full-screen window is programmed once at the end of init.

### Bitmap drawing

High-level call:

- `txw430038b0_nv3041a_01_spi_drawBitmap(dc, x, y)`

Fast path:

1. Clip the requested rectangle.
2. If the visible bitmap region is still contiguous in source memory:
   - one `set_window`
   - one `write_pixels_rgb565`
3. `qspi2_lcd_bus_write_pixels_rgb565()` sends one RAM write command, then
   pushes pixel bytes using EDMA in chunks.

Fallback path:

1. Clip the requested rectangle.
2. For each visible row:
   - program a 1-line window
   - send that row with `write_pixels_rgb565`

## Why The Layers Are Split This Way

### `qspi2_lcd_bus.c`

This file should answer only:

- How do bytes/pixels move over QSPI2?
- How do we handle QSPI2 + EDMA correctly?

It should not answer:

- What is the current clipping policy?
- How should a bitmap with stride be walked?
- How does EdgeTX want to render a display object?

### `txw430038b0_nv3041a_01_spi.c`

This file should answer:

- How does this panel initialize?
- How are windows and pixels mapped to panel commands?
- What is the best way to render an image region?

It is the right place for:

- fast-path selection
- clipping
- contiguous-vs-strided source handling

## Current EDMA Strategy

`qspi2_lcd_bus_write_pixels_rgb565()` currently uses:

- `BYTE -> BYTE` EDMA transfers
- an internal staging buffer
- explicit RGB565 byte packing as `high-byte, low-byte`

This was chosen because direct `HALFWORD` transfer caused visible corruption,
while the old CPU-driven byte-write path was known-good.

The function sends:

1. one QSPI2 RAM write command
2. many EDMA chunks from the staging buffer until all pixels are emitted
3. one final wait for `QSPI_CMDSTS`

## Important Interface Rules

### Preferred includes

- For EdgeTX/UI integration:
  - include `txw430038b0_nv3041a_01_spi.h`

- For low-level panel bring-up/tests:
  - include `txw430038b0_nv3041a_01_panel.h`

- For low-level QSPI2 transport work:
  - include `qspi2_lcd_bus.h`

- For debug-only bus inspection and TE helpers:
  - include `qspi2_lcd_bus_priv.h`

### Avoid

- Do not include `qspi2_lcd_bus_priv.h` from UI-layer code.
- Do not build new rendering logic directly on top of raw QSPI2 bus functions.
- Do not duplicate EdgeTX display types in multiple headers.

## Known Limitations

- `txw430038b0_nv3041a_01_spi_drawString()` is currently a stub.
- The panel adapter still lives in the same `.c` file as the panel driver.
  - This is acceptable for now.
  - If the integration grows, consider splitting:
    - `txw430038b0_nv3041a_01_panel.c`
    - `txw430038b0_nv3041a_01_edgetx_adapter.c`
- Demo/test code in `main.c` still directly calls some panel/private helpers.
  - That is okay for bring-up, but not the ideal final application shape.

## Recommended Future Cleanup

1. Replace `edgetx_lcd_compat.h` with real EdgeTX headers.
2. Implement or remove the `drawString()` adapter stub.
3. If EdgeTX integration grows, split panel logic from the EdgeTX adapter logic
   into separate source files.
4. Add a stride-aware rectangle writer if non-contiguous bitmap paths become a
   performance bottleneck.

