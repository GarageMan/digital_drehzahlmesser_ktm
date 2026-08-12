#!/usr/bin/env python3
"""
Minimal, self-contained PNG -> LVGL v8 C image converter.

Targets LV_COLOR_DEPTH == 32 (the default we use for the PC/SDL simulator
build), color format LV_IMG_CF_TRUE_COLOR_ALPHA (=5).

LVGL v8 stores CF_TRUE_COLOR_ALPHA pixels as 4 bytes each, in the same byte
order as lv_color32_t on a little-endian host: Blue, Green, Red, Alpha.
This matches struct lv_color32_t.ch { blue; green; red; alpha; } (LVGL v8,
lv_color.h). We reorder PIL's RGBA to BGRA accordingly.

Usage:
    python3 png_to_lvgl.py input.png output_basename symbol_name
Produces:
    <output_basename>.c
    <output_basename>.h
"""
import sys
from pathlib import Path
from PIL import Image

LV_IMG_CF_TRUE_COLOR_ALPHA = 5


def convert(png_path: str, out_base: str, symbol: str):
    img = Image.open(png_path).convert("RGBA")
    w, h = img.size
    px = img.tobytes()  # R,G,B,A per pixel

    out = bytearray()
    for i in range(0, len(px), 4):
        r, g, b, a = px[i], px[i + 1], px[i + 2], px[i + 3]
        out += bytes((b, g, r, a))  # BGRA order, per lv_color32_t

    data_size = len(out)

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
    c_lines.append(f"const lv_img_dsc_t {symbol} = {{")
    c_lines.append("    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,")
    c_lines.append("    .header.always_zero = 0,")
    c_lines.append("    .header.reserved = 0,")
    c_lines.append(f"    .header.w = {w},")
    c_lines.append(f"    .header.h = {h},")
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
    h_lines.append(f"extern const lv_img_dsc_t {symbol};")
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
