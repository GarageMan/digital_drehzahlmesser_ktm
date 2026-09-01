#!/usr/bin/env python3
"""
Minimal, self-contained PNG -> LVGL v9 C image converter.

Targets color format LV_COLOR_FORMAT_ARGB8888 (header value 0x10),
independent of the display's own color depth (v9 decodes/blits image data
in its own format regardless of the display's native format, unlike v8
where LV_COLOR_DEPTH had to match). See src/draw/lv_image_dsc.h and
src/misc/lv_color.h (struct lv_color32_t) in the LVGL v9 sources for the
struct layout this script targets.

LVGL v9 stores ARGB8888 pixels as 4 bytes each, in the same byte order as
lv_color32_t on a little-endian host: Blue, Green, Red, Alpha (identical
byte order to v8's CF_TRUE_COLOR_ALPHA - only the header struct/magic
changed between v8 and v9, not the pixel layout). We reorder the decoded
RGBA bytes to BGRA accordingly.

No external dependency (no Pillow/PIL): PNG decoding (IHDR/IDAT/IEND,
8-bit depth, color type 2/RGB or 6/RGBA, non-interlaced only - the only
cases this project's assets use, verified against the previously
PIL-generated arrays byte-for-byte) is done with the standard library's
`zlib` module. Anything outside that (16-bit depth, palette, interlacing)
raises instead of silently mishandling it.

Usage:
    python3 png_to_lvgl.py input.png output_basename symbol_name
Produces:
    <output_basename>.c
    <output_basename>.h
"""
import struct
import sys
import zlib
from pathlib import Path

LV_IMAGE_HEADER_MAGIC = 0x19
LV_COLOR_FORMAT_ARGB8888 = 0x10
BYTES_PER_PIXEL = 4

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def _paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    return b if pb <= pc else c


def read_png_rgba(png_path: str):
    """Decode an 8-bit, non-interlaced RGB/RGBA PNG into flat RGBA bytes."""
    data = Path(png_path).read_bytes()
    if data[:8] != PNG_SIGNATURE:
        raise ValueError(f"{png_path}: not a PNG file")

    pos = 8
    w = h = bit_depth = color_type = None
    idat = bytearray()
    while pos < len(data):
        length = struct.unpack(">I", data[pos : pos + 4])[0]
        ctype = data[pos + 4 : pos + 8]
        chunk = data[pos + 8 : pos + 8 + length]
        if ctype == b"IHDR":
            w, h, bit_depth, color_type, _cm, _fm, interlace = struct.unpack(
                ">IIBBBBB", chunk
            )
            if bit_depth != 8:
                raise ValueError(f"{png_path}: unsupported bit depth {bit_depth} (need 8)")
            if color_type not in (2, 6):
                raise ValueError(
                    f"{png_path}: unsupported color type {color_type} (need 2=RGB or 6=RGBA)"
                )
            if interlace != 0:
                raise ValueError(f"{png_path}: interlaced PNGs are not supported")
        elif ctype == b"IDAT":
            idat += chunk
        elif ctype == b"IEND":
            break
        pos += 8 + length + 4  # length + type + data + crc

    channels = 4 if color_type == 6 else 3
    stride = w * channels
    raw = zlib.decompress(bytes(idat))

    out = bytearray(w * h * 4)
    prev = bytearray(stride)
    off = 0
    for y in range(h):
        ftype = raw[off]
        off += 1
        line = bytearray(raw[off : off + stride])
        off += stride
        # Undo the per-scanline PNG filter (see RFC 2083 section 6).
        for x in range(stride):
            a = line[x - channels] if x >= channels else 0
            b = prev[x]
            c = prev[x - channels] if x >= channels else 0
            if ftype == 0:
                pass
            elif ftype == 1:
                line[x] = (line[x] + a) & 0xFF
            elif ftype == 2:
                line[x] = (line[x] + b) & 0xFF
            elif ftype == 3:
                line[x] = (line[x] + ((a + b) // 2)) & 0xFF
            elif ftype == 4:
                line[x] = (line[x] + _paeth(a, b, c)) & 0xFF
            else:
                raise ValueError(f"{png_path}: unsupported filter type {ftype}")
        prev = line
        for x in range(w):
            si = x * channels
            di = (y * w + x) * 4
            r, g, bl = line[si], line[si + 1], line[si + 2]
            al = line[si + 3] if channels == 4 else 255
            out[di], out[di + 1], out[di + 2], out[di + 3] = r, g, bl, al

    return w, h, bytes(out)


def convert(png_path: str, out_base: str, symbol: str):
    w, h, rgba = read_png_rgba(png_path)

    out = bytearray()
    for i in range(0, len(rgba), 4):
        r, g, b, a = rgba[i], rgba[i + 1], rgba[i + 2], rgba[i + 3]
        out += bytes((b, g, r, a))  # BGRA order, per lv_color32_t

    data_size = len(out)
    stride = w * BYTES_PER_PIXEL

    c_lines = []
    c_lines.append('#include "lvgl/lvgl.h"')
    c_lines.append("")
    c_lines.append("#ifndef LV_ATTRIBUTE_MEM_ALIGN")
    c_lines.append("#define LV_ATTRIBUTE_MEM_ALIGN")
    c_lines.append("#endif")
    c_lines.append("")
    c_lines.append(f"static LV_ATTRIBUTE_MEM_ALIGN const uint8_t {symbol}_map[] = {{")
    # 16 bytes per line for readability
    for i in range(0, len(out), 16):
        chunk = out[i : i + 16]
        c_lines.append("    " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
    c_lines.append("};")
    c_lines.append("")
    c_lines.append(f"const lv_image_dsc_t {symbol} = {{")
    c_lines.append("    .header.magic = LV_IMAGE_HEADER_MAGIC,")
    c_lines.append("    .header.cf = LV_COLOR_FORMAT_ARGB8888,")
    c_lines.append("    .header.flags = 0,")
    c_lines.append(f"    .header.w = {w},")
    c_lines.append(f"    .header.h = {h},")
    c_lines.append(f"    .header.stride = {stride},")
    c_lines.append("    .header.reserved_2 = 0,")
    c_lines.append(f"    .data_size = {data_size},")
    c_lines.append(f"    .data = {symbol}_map,")
    c_lines.append("};")
    c_lines.append("")

    h_lines = []
    guard = f"{symbol.upper()}_H"
    h_lines.append(f"#ifndef {guard}")
    h_lines.append(f"#define {guard}")
    h_lines.append("")
    h_lines.append('#include "lvgl/lvgl.h"')
    h_lines.append("")
    h_lines.append(f"extern const lv_image_dsc_t {symbol};")
    h_lines.append("")
    h_lines.append(f"#endif /* {guard} */")
    h_lines.append("")

    Path(out_base + ".c").write_text("\n".join(c_lines))
    Path(out_base + ".h").write_text("\n".join(h_lines))
    print(f"{png_path}: {w}x{h}, {data_size} bytes -> {out_base}.c/.h (symbol: {symbol})")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(__doc__)
        sys.exit(1)
    convert(sys.argv[1], sys.argv[2], sys.argv[3])
