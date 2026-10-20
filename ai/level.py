"""Đọc màn chơi ``.data`` đúng định dạng của ``Floor::initialize`` (code/tile.cpp).

Định dạng (các số cách nhau bởi khoảng trắng bất kỳ, đọc theo token):

    n m                      # số dòng, số cột
    <n dòng, mỗi dòng m số>  # 0 trống/lỗ, 1 sàn, 2 dễ vỡ (cam),
                             # 3 công tắc thường, 4 công tắc nặng
    sx sy along              # vị trí bắt đầu + tư thế X/Y/Z
    tx ty                    # ô đích (trên lưới phải là 0 = lỗ)
    k                        # số công tắc thường
    x y a b   (k dòng)       # công tắc tại (x,y) bật/tắt ô cầu (a,b)
    h                        # số công tắc nặng
    x y a b   (h dòng)

Mỗi công tắc chỉ điều khiển MỘT ô cầu (``std::map<int,int>`` trong C++). Nếu
một vị trí công tắc khai báo hai lần, dòng sau ghi đè dòng trước (giống
``map[key] = value``).
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .state import Orientation

EMPTY, NORMAL, FRAGILE, NORMAL_SWITCH, HEAVY_SWITCH = 0, 1, 2, 3, 4
TILE_NAMES = {
    EMPTY: "empty",
    NORMAL: "normal",
    FRAGILE: "fragile",
    NORMAL_SWITCH: "normal_switch",
    HEAVY_SWITCH: "heavy_switch",
}


@dataclass(frozen=True, slots=True)
class Switch:
    """Một công tắc: ``kind`` là ``"normal"`` hoặc ``"heavy"``."""

    kind: str
    pos: tuple[int, int]
    target: tuple[int, int]
    bit: int  # chỉ số bit trong State.bridge_mask


class Level:
    """Dữ liệu tĩnh của một màn chơi (không đổi trong lúc tìm kiếm)."""

    def __init__(
        self,
        grid: list[list[int]],
        start: tuple[int, int],
        start_orientation: Orientation,
        goal: tuple[int, int],
        normal_switches: list[tuple[int, int, int, int]],
        heavy_switches: list[tuple[int, int, int, int]],
        name: str = "<level>",
    ) -> None:
        self.name = name
        self.n = len(grid)
        self.m = len(grid[0]) if grid else 0
        self.grid: tuple[tuple[int, ...], ...] = tuple(tuple(row) for row in grid)
        self.start = start
        self.start_orientation = start_orientation
        self.goal = goal

        # Gộp như std::map: khóa trùng -> giữ đích của dòng sau, thứ tự lần đầu xuất hiện.
        switches: list[Switch] = []
        for kind, entries in (("normal", normal_switches), ("heavy", heavy_switches)):
            targets: dict[tuple[int, int], tuple[int, int]] = {}
            for x, y, a, b in entries:
                targets[(x, y)] = (a, b)
            for pos, target in targets.items():
                switches.append(Switch(kind, pos, target, len(switches)))
        self.switches: tuple[Switch, ...] = tuple(switches)
        self._switch_at = {(s.kind, s.pos): s for s in self.switches}

        # Ô cầu -> mask các công tắc điều khiển nó (một ô có thể bị nhiều công tắc bật/tắt).
        bits: dict[tuple[int, int], int] = {}
        for s in self.switches:
            bits[s.target] = bits.get(s.target, 0) | (1 << s.bit)
        self.bridge_bits: dict[tuple[int, int], int] = bits

        self.validate()

    # ------------------------------------------------------------------ đọc file
    @classmethod
    def from_file(cls, path: str | Path) -> "Level":
        path = Path(path)
        return cls.from_text(path.read_text(encoding="utf-8"), name=path.name)

    @classmethod
    def from_text(cls, text: str, name: str = "<text>") -> "Level":
        tokens = iter(text.split())

        def nxt() -> str:
            try:
                return next(tokens)
            except StopIteration:
                raise ValueError(f"{name}: file kết thúc sớm") from None

        def nint() -> int:
            return int(nxt())

        n, m = nint(), nint()
        grid = [[nint() for _ in range(m)] for _ in range(n)]
        sx, sy = nint(), nint()
        along = nxt()
        if along not in ("X", "Y", "Z"):
            raise ValueError(f"{name}: tư thế bắt đầu phải là X/Y/Z, nhận '{along}'")
        tx, ty = nint(), nint()
        normal = [(nint(), nint(), nint(), nint()) for _ in range(nint())]
        heavy = [(nint(), nint(), nint(), nint()) for _ in range(nint())]
        return cls(grid, (sx, sy), Orientation(along), (tx, ty), normal, heavy, name=name)

    # ---------------------------------------------------------------- kiểm tra
    def in_bounds(self, x: int, y: int) -> bool:
        return 0 <= x < self.n and 0 <= y < self.m

    def validate(self) -> None:
        """Bắt các màn mà game C++ sẽ xử lý sai/không xác định."""
        err = f"{self.name}:"
        if not self.in_bounds(*self.goal):
            raise ValueError(f"{err} ô đích {self.goal} nằm ngoài lưới")
        if self.grid[self.goal[0]][self.goal[1]] != EMPTY:
            raise ValueError(f"{err} ô đích {self.goal} phải là 0 (lỗ) trên lưới")
        for row in self.grid:
            for v in row:
                if v not in TILE_NAMES:
                    raise ValueError(f"{err} loại ô không hợp lệ: {v}")
        expected = {"normal": NORMAL_SWITCH, "heavy": HEAVY_SWITCH}
        for s in self.switches:
            for cell in (s.pos, s.target):
                if not self.in_bounds(*cell):
                    raise ValueError(f"{err} công tắc {s} trỏ ra ngoài lưới")
            if self.grid[s.pos[0]][s.pos[1]] != expected[s.kind]:
                raise ValueError(f"{err} ô {s.pos} không phải công tắc {s.kind}")
            if self.grid[s.target[0]][s.target[1]] != EMPTY:
                # C++ bật lại cầu luôn tạo ô type 1 -> ô cầu ban đầu khác 0 sẽ đổi loại.
                raise ValueError(f"{err} ô cầu {s.target} phải là 0 lúc bắt đầu")
            if s.target == self.goal:
                raise ValueError(f"{err} ô cầu không được trùng ô đích")
        for x in range(self.n):
            for y in range(self.m):
                v = self.grid[x][y]
                if v == NORMAL_SWITCH and ("normal", (x, y)) not in self._switch_at:
                    raise ValueError(f"{err} công tắc thường {(x, y)} thiếu dòng ánh xạ cầu")
                if v == HEAVY_SWITCH and ("heavy", (x, y)) not in self._switch_at:
                    raise ValueError(f"{err} công tắc nặng {(x, y)} thiếu dòng ánh xạ cầu")

    # ------------------------------------------------------------ truy vấn ô
    def has_tile(self, x: int, y: int, mask: int) -> bool:
        """Ô (x, y) có sàn không, với cấu hình cầu ``mask``. Ngoài lưới = không."""
        if not self.in_bounds(x, y):
            return False
        present = self.grid[x][y] != EMPTY
        bits = self.bridge_bits.get((x, y), 0)
        if bits and (bin(mask & bits).count("1") & 1):
            present = not present
        return present

    def tile_type(self, x: int, y: int, mask: int) -> int:
        """Loại ô hiện tại (``EMPTY`` nếu không có sàn). Cầu được bật là ô thường."""
        if not self.has_tile(x, y, mask):
            return EMPTY
        v = self.grid[x][y]
        return v if v != EMPTY else NORMAL

    def switch_at(self, kind: str, x: int, y: int) -> Switch:
        return self._switch_at[(kind, (x, y))]

    def __repr__(self) -> str:
        return (
            f"Level({self.name}, {self.n}x{self.m}, start={self.start}"
            f"{self.start_orientation.value}, goal={self.goal}, switches={len(self.switches)})"
        )
