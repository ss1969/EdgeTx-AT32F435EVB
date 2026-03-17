import argparse
import os
import struct
import re


def _read_u16_le(b, off):
    return struct.unpack_from("<H", b, off)[0]


def _read_u32_le(b, off):
    return struct.unpack_from("<I", b, off)[0]


def _read_i32_le(b, off):
    return struct.unpack_from("<i", b, off)[0]


def _mask_shift_width(mask):
    if mask == 0:
        return 0, 0
    shift = 0
    while (mask & 1) == 0:
        mask >>= 1
        shift += 1
    width = 0
    while (mask & 1) == 1:
        mask >>= 1
        width += 1
    return shift, width


def _extract_component(value, mask):
    shift, width = _mask_shift_width(mask)
    if width == 0:
        return 0
    v = (value & mask) >> shift
    if width >= 8:
        return v & 0xFF
    max_in = (1 << width) - 1
    return (v * 255 + (max_in // 2)) // max_in


def _rgb888_to_565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def load_bmp_as_rgb565(path):
    with open(path, "rb") as f:
        data = f.read()

    if len(data) < 54:
        raise ValueError("file too small")

    if data[0:2] != b"BM":
        raise ValueError("not a BMP (missing BM signature)")

    pixel_offset = _read_u32_le(data, 10)
    dib_size = _read_u32_le(data, 14)
    if dib_size < 40:
        raise ValueError(f"unsupported DIB header size: {dib_size}")

    width = _read_i32_le(data, 18)
    height = _read_i32_le(data, 22)
    top_down = height < 0
    height_abs = -height if top_down else height
    planes = _read_u16_le(data, 26)
    bpp = _read_u16_le(data, 28)
    compression = _read_u32_le(data, 30)

    if planes != 1:
        raise ValueError(f"unsupported planes: {planes}")
    if width <= 0 or height_abs <= 0:
        raise ValueError(f"invalid dimensions: {width}x{height}")

    if pixel_offset >= len(data):
        raise ValueError("pixel data offset out of range")

    if compression == 0:
        if bpp not in (24, 32, 16):
            raise ValueError(f"unsupported bpp for BI_RGB: {bpp}")
    elif compression in (3, 6):
        if bpp != 16:
            raise ValueError(f"unsupported bpp for BI_BITFIELDS/ALPHABITFIELDS: {bpp}")
    else:
        raise ValueError(f"unsupported compression: {compression}")

    masks = None
    if compression in (3, 6):
        masks_off = 14 + dib_size
        if masks_off + 12 > len(data):
            raise ValueError("bitfield masks out of range")
        rmask = _read_u32_le(data, masks_off + 0)
        gmask = _read_u32_le(data, masks_off + 4)
        bmask = _read_u32_le(data, masks_off + 8)
        masks = (rmask, gmask, bmask)

    row_pixels = width
    bytes_per_pixel = (bpp + 7) // 8
    row_stride_unpadded = row_pixels * bytes_per_pixel
    row_stride = (row_stride_unpadded + 3) & ~3

    need_size = pixel_offset + row_stride * height_abs
    if need_size > len(data):
        raise ValueError("truncated BMP (not enough pixel data)")

    out = [0] * (width * height_abs)

    def src_row_index(dst_y):
        return dst_y if top_down else (height_abs - 1 - dst_y)

    for y in range(height_abs):
        src_y = src_row_index(y)
        row_off = pixel_offset + src_y * row_stride
        for x in range(width):
            px_off = row_off + x * bytes_per_pixel
            if bpp == 24:
                b = data[px_off + 0]
                g = data[px_off + 1]
                r = data[px_off + 2]
            elif bpp == 32:
                b = data[px_off + 0]
                g = data[px_off + 1]
                r = data[px_off + 2]
            elif bpp == 16:
                raw = _read_u16_le(data, px_off)
                if masks:
                    r = _extract_component(raw, masks[0])
                    g = _extract_component(raw, masks[1])
                    b = _extract_component(raw, masks[2])
                else:
                    r = ((raw >> 10) & 0x1F) * 255 // 31
                    g = ((raw >> 5) & 0x1F) * 255 // 31
                    b = (raw & 0x1F) * 255 // 31
            else:
                raise ValueError("unreachable")

            out[y * width + x] = _rgb888_to_565(r, g, b)

    return width, height_abs, out


def write_h_file(out_h, var_name, width, height, pixels):
    base_h = os.path.basename(out_h)
    guard = base_h.upper().replace(".", "_").replace("-", "_")
    if not guard.endswith("_H"):
        guard = guard + "_H"

    out_dir = os.path.dirname(out_h)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    with open(out_h, "w", newline="\n") as f:
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")
        f.write(f"#define BMP_WIDTH {width}U\n")
        f.write(f"#define BMP_HEIGHT {height}U\n")
        f.write(f"#define BMP_FORMAT 0U\n\n")
        f.write(f"static const unsigned short bmp_pixels[BMP_WIDTH * BMP_HEIGHT] = {{\n")
        for i, px in enumerate(pixels):
            if i % 12 == 0:
                f.write("  ")
            f.write(f"0x{px:04X}u")
            if i != len(pixels) - 1:
                f.write(", ")
            if (i % 12) == 11:
                f.write("\n")
        if (len(pixels) % 12) != 0:
            f.write("\n")
        f.write("};\n\n")
        f.write(f"#endif\n")


def _sanitize_identifier(name):
    name = re.sub(r"[^0-9A-Za-z_]", "_", name)
    if not name:
        name = "bmp"
    if name[0].isdigit():
        name = "_" + name
    return name


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bmp", help="input BMP file path")
    args = ap.parse_args()

    stem = os.path.splitext(os.path.basename(args.bmp))[0]
    var_name = _sanitize_identifier(stem)
    out_h = os.path.join("project", "inc", f"{var_name}_rgb565.h")

    width, height, pixels = load_bmp_as_rgb565(args.bmp)
    write_h_file(out_h, var_name, width, height, pixels)
    print(f"OK: {args.bmp} -> {out_h} ({width}x{height}, RGB565)")


if __name__ == "__main__":
    main()
