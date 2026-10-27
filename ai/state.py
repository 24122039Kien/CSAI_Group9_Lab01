"""Biểu diễn trạng thái của bài toán Bloxorz.

Quy ước tọa độ giống hệt game C++ (``code/``):

* ``x`` là chỉ số DÒNG trong file ``.data``, ``y`` là chỉ số CỘT.
* Hướng đi: ``R`` -> x+1, ``L`` -> x-1, ``U`` -> y+1, ``D`` -> y-1.
  (Khi nhìn file text, "Right" là đi XUỐNG một dòng, "Up" là sang PHẢI một cột.)

Ba tư thế của khối (biến ``along`` trong C++):

* ``Z`` - đứng, chiếm 1 ô ``(x, y)``.
* ``X`` - nằm dọc theo trục x, chiếm ``(x, y)`` và ``(x + 1, y)``.
* ``Y`` - nằm dọc theo trục y, chiếm ``(x, y)`` và ``(x, y + 1)``.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum

# Thứ tự hành động cố định -> kết quả BFS/DFS tái lập được.
ACTIONS: tuple[str, ...] = ("U", "D", "L", "R")
SPLIT_ACTIONS: tuple[str, ...] = ("0U", "0D", "0L", "0R", "1U", "1D", "1L", "1R")


class Orientation(str, Enum):
    """Tư thế của khối, trùng tên với hằng X/Y/Z trong ``superblock.cpp``."""

    X = "X"
    Y = "Y"
    Z = "Z"


@dataclass(frozen=True, slots=True)
class State:
    """Một trạng thái tìm kiếm (bất biến, hashable -> dùng làm khóa set/dict).

    Attributes:
        x, y: ô "neo" của khối (ô có chỉ số nhỏ nhất).
        orientation: tư thế X/Y/Z.
        bridge_mask: mỗi Ô CẦU có một bit (``Level.bridge_bits``), bit bật = ô cầu
            đang mở. Hai trạng thái cùng vị trí nhưng khác ``bridge_mask`` là
            HAI trạng thái khác nhau (cầu đóng/mở khác nhau).
        cubes: khi khối bị tách thành 2 cube 1x1x1, lưu cặp tọa độ ((x1, y1), (x2, y2))
            đã chuẩn hóa sắp xếp; None khi khối chưa tách.
    """

    x: int
    y: int
    orientation: Orientation
    bridge_mask: int = 0
    cubes: tuple[tuple[int, int], tuple[int, int]] | None = None

    def __post_init__(self) -> None:
        if self.cubes is not None:
            # Chuẩn hóa thứ tự 2 cube để tránh lặp trạng thái
            c1, c2 = self.cubes
            if c1 > c2:
                object.__setattr__(self, "cubes", (c2, c1))
            # Cập nhật ô neo theo cube có tọa độ nhỏ hơn
            c_first = min(c1, c2)
            object.__setattr__(self, "x", c_first[0])
            object.__setattr__(self, "y", c_first[1])

    @property
    def is_split(self) -> bool:
        """True nếu khối đang bị tách thành 2 cube 1x1x1."""
        return self.cubes is not None

    def cells(self) -> tuple[tuple[int, int], ...]:
        """Các ô mà khối/cubes đang chiếm."""
        if self.cubes is not None:
            return self.cubes
        if self.orientation is Orientation.Z:
            return ((self.x, self.y),)
        if self.orientation is Orientation.X:
            return ((self.x, self.y), (self.x + 1, self.y))
        return ((self.x, self.y), (self.x, self.y + 1))

    def __str__(self) -> str:
        if self.cubes is not None:
            c1, c2 = self.cubes
            return f"(cubes=[{c1},{c2}],mask={self.bridge_mask:b})"
        return f"({self.x},{self.y},{self.orientation.value},mask={self.bridge_mask:b})"
