"""Tiện ích dùng chung cho test."""

from __future__ import annotations

from pathlib import Path

from ai import BloxorzProblem, Level

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "code" / "data"

# Lời giải đã được xác nhận thắng trong game thật (và là tối ưu theo số bước).
SOLUTIONS = {
    1: "R U U R U R U R D L",
    2: (
        "D R D L D L U R U D L D R U R D R R U R R D R R R R U D L L L L U L L L D D L "
        "U R R R R R U U R U R R R U R L U U L L L L L D L U R D L"
    ),
}
OPTIMAL_LENGTH = {1: 10, 2: 68}


def level_path(n: int) -> Path:
    return DATA / f"{n}.data"


def load_problem(n: int) -> BloxorzProblem:
    return BloxorzProblem(Level.from_file(level_path(n)))


def make_level(rows, start, along, goal, normal=(), heavy=()) -> Level:
    """Tạo màn từ danh sách chuỗi chữ số. ``rows[x][y]`` = ô (x, y).

    Nhắc lại: R/L đổi x (dòng), U/D đổi y (cột).
    """
    lines = [f"{len(rows)} {len(rows[0])}"]
    lines += [" ".join(r) for r in rows]
    lines.append(f"{start[0]} {start[1]} {along}")
    lines.append(f"{goal[0]} {goal[1]}")
    lines.append(str(len(normal)))
    lines += [" ".join(map(str, s)) for s in normal]
    lines.append(str(len(heavy)))
    lines += [" ".join(map(str, s)) for s in heavy]
    return Level.from_text("\n".join(lines), name="test-level")


def make_problem(*args, **kwargs) -> BloxorzProblem:
    return BloxorzProblem(make_level(*args, **kwargs))
