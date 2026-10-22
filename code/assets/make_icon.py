"""Sinh code/assets/bloxorz.ico (khối đỏ đứng trên sàn 3x3, nhìn isometric) chỉ bằng thư viện chuẩn.

Chạy lại khi muốn đổi icon:  python code/assets/make_icon.py
"""
from __future__ import annotations

import math
import struct
import zlib
from pathlib import Path

SS = 3  # siêu lấy mẫu để khử răng cưa


def iso(x: float, y: float, z: float, size: int) -> tuple[float, float]:
    s = size * 0.16
    return (size / 2 + (x - y) * math.cos(math.radians(30)) * s,
            size * 0.47 + (x + y - 3) * math.sin(math.radians(30)) * s - z * s)


def inside(px: float, py: float, poly) -> bool:
    c = False
    for i in range(len(poly)):
        (x1, y1), (x2, y2) = poly[i], poly[i - 1]
        if (y1 > py) != (y2 > py) and px < (x2 - x1) * (py - y1) / (y2 - y1) + x1:
            c = not c
    return c


def shapes(size: int):
    out = []
    gap = 0.06
    for i in range(3):
        for j in range(3):
            if (i, j) == (1, 1):
                continue
            quad = [(i + gap, j + gap), (i + 1 - gap, j + gap), (i + 1 - gap, j + 1 - gap), (i + gap, j + 1 - gap)]
            col = (128, 140, 163) if (i + j) % 2 else (118, 129, 150)
            out.append(([iso(x, y, 0, size) for x, y in quad], col))
    # khối 1x1x2 đứng ở ô giữa
    a, b, h = 1.0, 2.0, 2.0
    out.append(([iso(b, a, 0, size), iso(b, b, 0, size), iso(b, b, h, size), iso(b, a, h, size)], (150, 24, 20)))
    out.append(([iso(a, b, 0, size), iso(b, b, 0, size), iso(b, b, h, size), iso(a, b, h, size)], (112, 16, 14)))
    out.append(([iso(a, a, h, size), iso(b, a, h, size), iso(b, b, h, size), iso(a, b, h, size)], (222, 48, 40)))
    return out


def render(size: int) -> bytes:
    shp = shapes(size)
    r = size * 0.18  # bo góc nền
    rows = []
    for y in range(size):
        row = bytearray([0])
        for x in range(size):
            acc = [0, 0, 0, 0]
            for sy in range(SS):
                for sx in range(SS):
                    px, py = x + (sx + 0.5) / SS, y + (sy + 0.5) / SS
                    dx = max(r - px, px - (size - r), 0)
                    dy = max(r - py, py - (size - r), 0)
                    if dx * dx + dy * dy > r * r:
                        continue
                    col = (12, 16, 32)
                    for poly, c in shp:
                        if inside(px, py, poly):
                            col = c
                    acc[0] += col[0]; acc[1] += col[1]; acc[2] += col[2]; acc[3] += 255
            n = SS * SS
            row += bytes([acc[0] // n, acc[1] // n, acc[2] // n, acc[3] // n])
        rows.append(bytes(row))
    raw = b"".join(rows)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def main() -> None:
    sizes = [256, 48, 32, 16]
    images = [render(s) for s in sizes]
    header = struct.pack("<HHH", 0, 1, len(sizes))
    offset = 6 + 16 * len(sizes)
    entries = b""
    for s, img in zip(sizes, images):
        entries += struct.pack("<BBBBHHII", s % 256, s % 256, 0, 0, 1, 32, len(img), offset)
        offset += len(img)
    out = Path(__file__).with_name("bloxorz.ico")
    out.write_bytes(header + entries + b"".join(images))
    Path(__file__).with_name("bloxorz_256.png").write_bytes(images[0])
    print("wrote", out.name)


if __name__ == "__main__":
    main()
