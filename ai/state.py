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
        bridge_mask: bit thứ ``i`` = số lần công tắc thứ ``i`` đã được kích hoạt
            (mod 2). Hai trạng thái cùng vị trí nhưng khác ``bridge_mask`` là
            HAI trạng thái khác nhau (cầu đóng/mở khác nhau).
    """

    x: int
    y: int
    orientation: Orientation
    bridge_mask: int = 0

    def cells(self) -> tuple[tuple[int, int], ...]:
        """Các ô mà khối đang chiếm."""
        if self.orientation is Orientation.Z:
            return ((self.x, self.y),)
        if self.orientation is Orientation.X:
            return ((self.x, self.y), (self.x + 1, self.y))
        return ((self.x, self.y), (self.x, self.y + 1))

    def __str__(self) -> str:
        return f"({self.x},{self.y},{self.orientation.value},mask={self.bridge_mask:b})"
