"""Render board diagrams from the actual .data files and catalog.

python -m pip install -r scripts/requirements-previews.txt
python scripts/render_level_previews.py
"""

import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

try:
    from PIL import Image, ImageDraw, ImageFont, ImageOps
except ImportError:
    raise SystemExit("Install previews support: python -m pip install -r scripts/requirements-previews.txt")

from ai.level import Level
from ai.state import State

DATA = ROOT / "code" / "data"
CELL = 32
COLORS = {0: "#111827", 1: "#CBD5E1", 2: "#F59E0B", 3: "#34D399", 4: "#C4B5FD", 5: "#38BDF8"}


def font(size):
    for path in (
        Path("C:/Windows/Fonts/segoeui.ttf"),
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
        Path("/System/Library/Fonts/Supplemental/Arial.ttf"),
    ):
        if path.exists():
            return ImageFont.truetype(str(path), size)
    return ImageFont.load_default(size=size)


def render(entry):
    level = Level.from_file(DATA / entry["file"])
    width = max(640, level.m * CELL + 80)
    height = level.n * CELL + 200
    image = Image.new("RGB", (width, height), "#0B1120")
    draw = ImageDraw.Draw(image)
    draw.text((24, 12), f"{entry['id']:02d}  |  {entry['group']}  |  {entry['title']}", font=font(20), fill="#F8FAFC")
    optimum = entry["optimal_length"]
    status = "NO SOLUTION" if optimum is None else f"Optimal: {optimum} moves"
    draw.text((24, 41), f"{level.n} x {level.m}  |  {status}  |  Initial board", font=font(14), fill="#94A3B8")
    left = (width - level.m * CELL) // 2
    top = 86
    small, label = font(11), font(14)
    for y in range(level.m):
        draw.text((left + y * CELL + CELL / 2, top - 13), str(y), anchor="mm", font=small, fill="#94A3B8")
    for x in range(level.n):
        draw.text((left - 13, top + x * CELL + CELL / 2), str(x), anchor="mm", font=small, fill="#94A3B8")
        for y in range(level.m):
            x0, y0 = left + y * CELL, top + x * CELL
            draw.rounded_rectangle((x0 + 1, y0 + 1, x0 + CELL - 1, y0 + CELL - 1), radius=3, fill=COLORS[level.grid[x][y]], outline="#253247")
            if level.grid[x][y] == 2:
                draw.line((x0 + 8, y0 + 6, x0 + 17, y0 + 15, x0 + 12, y0 + 21, x0 + 22, y0 + 27), fill="#78350F", width=2)
    for switch in level.switches:
        x, y = switch.pos
        draw.text((left + y * CELL + CELL / 2, top + x * CELL + CELL / 2), str(switch.bit + 1), anchor="mm", font=label, fill="#0F172A")
        for tx, ty in switch.targets:
            x0, y0 = left + ty * CELL + 3, top + tx * CELL + 3
            draw.rectangle((x0, y0, x0 + CELL - 6, y0 + CELL - 6), outline="#38BDF8", width=2)
            draw.text((x0 + (CELL - 6) / 2, y0 + (CELL - 6) / 2), str(switch.bit + 1), anchor="mm", font=small, fill="#38BDF8")
    for (sx, sy) in level.split_switches:
        cx, cy = left + sy * CELL + CELL / 2, top + sx * CELL + CELL / 2
        draw.text((cx, cy), "T", anchor="mm", font=label, fill="#0F172A")
    gx, gy = level.goal
    cx, cy = left + gy * CELL + CELL / 2, top + gx * CELL + CELL / 2
    draw.ellipse((cx - 12, cy - 12, cx + 12, cy + 12), fill="#111827", outline="#FB7185", width=3)
    draw.text((cx, cy), "G", anchor="mm", font=label, fill="#FB7185")
    start = State(*level.start, level.start_orientation)
    for x, y in start.cells():
        x0, y0 = left + y * CELL + 3, top + x * CELL + 3
        draw.rounded_rectangle((x0, y0, x0 + CELL - 6, y0 + CELL - 6), radius=3, fill="#60A5FA")
    sx, sy = level.start
    draw.text((left + sy * CELL + CELL / 2, top + sx * CELL + CELL / 2), "S", anchor="mm", font=label, fill="#0B1120")

    legend_y = top + level.n * CELL + 18
    legend = [("Floor", COLORS[1]), ("Fragile", COLORS[2]), ("Soft switch", COLORS[3]), ("Heavy switch", COLORS[4]), ("Split switch T", "#38BDF8"), ("Bridge off", "#38BDF8"), ("Start S", "#60A5FA"), ("Goal G", "#FB7185")]
    step = (width - 48) // 4
    for index, (text, color) in enumerate(legend):
        x0 = 24 + (index % 4) * step
        y0 = legend_y + (index // 4) * 25
        draw.rectangle((x0, y0, x0 + 12, y0 + 12), fill=color)
        draw.text((x0 + 20, y0 - 3), text, font=font(12), fill="#CBD5E1")
    draw.text((24, height - 24), "x = row, y = column. U: y+1; D: y-1; R: x+1; L: x-1.", font=font(12), fill="#94A3B8")
    return image


def main():
    catalog = json.loads((DATA / "catalog.json").read_text(encoding="utf-8"))
    panels = []
    for entry in catalog["levels"]:
        output = DATA / entry["preview"]
        output.parent.mkdir(parents=True, exist_ok=True)
        image = render(entry)
        image.save(output)
        panels.append(ImageOps.contain(image, (560, 440)))
    overview = Image.new("RGB", (1160, ((len(panels) + 1) // 2) * 460 + 20), "#020617")
    for index, panel in enumerate(panels):
        overview.paste(panel, (20 + (index % 2) * 580, 20 + (index // 2) * 460))
    overview.save(DATA / "previews" / "overview.png")
    print(f"Rendered {len(panels)} levels and overview.png")


if __name__ == "__main__":
    main()
