# QSPI2 LCD Config Notes

## Confirmed Working Setup

Target panel:

- `TXW430038B0 / NV3041A-01`

Current verified-good configuration:

- Command path: `QSPI 1-lane`
- Pixel path: `QSPI 4-lane`
- Pixel write opcode: `0x32`
- Pixel write address field: `0x00 0x2C 0x00`
- Pixel operation mode: `QSPI_OPERATE_MODE_114`
- `0x43 (QSPI_DCTL) = 0x01`
- Color order: `RGB`
- Pixel format: `RGB565`

The startup log should match:

```text
LCD: cfg bus=QSPI2 cmd=111 pixel=114 gram=RAM32 dctl=01 rgb=565
```

## Important Register Meaning

For register `0x43`:

- `bit4` = `bgr`
- `bit0` = `sbyte`

Confirmed-good value:

- `0x01` = `bgr=0`, `sbyte=1`

Observed behavior:

- Writing `F800` should display `red`
- If `F800` displays `blue`, the panel is effectively running in `BGR`
- `0x11` caused red/blue swap in our test and was rejected

## Current Code Locations

Init table `0x43` value:

- [project/src/txw430038b0_nv3041a_01_spi.c](/d:/Projects/EdgeTX/AT32F435ZMT7EV/project/src/txw430038b0_nv3041a_01_spi.c)

Pixel write command setup:

- [project/src/qspi2_lcd_bus.c](/d:/Projects/EdgeTX/AT32F435ZMT7EV/project/src/qspi2_lcd_bus.c)

Startup log string:

- [project/src/main.c](/d:/Projects/EdgeTX/AT32F435ZMT7EV/project/src/main.c)

## Tested And Rejected

- `0x43 = 0x11`
  Result: red/blue swapped, effectively `BGR`

- Pixel path `02h + 111`
  Was useful for comparison testing, but current confirmed-good setup is `32h + 114`

## Recovery Hint

If colors look wrong again, test in this order:

1. Check `0x43`
2. Verify startup log prints `dctl=01 rgb=565`
3. Send `F800`, `07E0`, `001F`, `FFFF`
4. Confirm display shows `red`, `green`, `blue`, `white`
