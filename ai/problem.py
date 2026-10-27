"""Mô hình bài toán tìm kiếm Bloxorz - chép CHÍNH XÁC luật của game C++.

Nguồn luật (code/):

* ``SuperBlock::update`` (superblock.cpp) - bảng chuyển tư thế/vị trí.
* ``Game::check`` (game.cpp) - thắng/thua, chạy MỖI FRAME.
* ``Game::check_after_step`` (game.cpp) - sau mỗi nước: nếu thắng/thua thì
  dừng, KHÔNG kích hoạt công tắc; ngược lại kích hoạt công tắc tuần tự.

Ghi chú: sau khi công tắc đổi cầu, frame kế tiếp ``Game::check`` chạy lại -> nếu
cầu dưới khối vừa biến mất thì khối rơi (thua). ``simulate`` mô phỏng cả bước
kiểm tra lại này.

Mọi thay đổi luật ở đây PHẢI được làm tương ứng trong C++ (và ngược lại),
kèm test trong ``tests/test_problem.py``.
"""

from __future__ import annotations

from enum import Enum
from typing import Callable

from .level import FRAGILE, HEAVY_SWITCH, NORMAL_SWITCH, SPLIT_SWITCH, Level
from .state import ACTIONS, SPLIT_ACTIONS, Orientation, State

X, Y, Z = Orientation.X, Orientation.Y, Orientation.Z

# (tư thế hiện tại, hướng) -> (tư thế mới, dx, dy)   -- SuperBlock::update
TRANSITIONS: dict[Orientation, dict[str, tuple[Orientation, int, int]]] = {
    X: {"L": (Z, -1, 0), "R": (Z, 2, 0), "U": (X, 0, 1), "D": (X, 0, -1)},
    Y: {"U": (Z, 0, 2), "D": (Z, 0, -1), "R": (Y, 1, 0), "L": (Y, -1, 0)},
    Z: {"L": (X, -2, 0), "R": (X, 1, 0), "U": (Y, 0, 1), "D": (Y, 0, -2)},
}

CUBE_DELTAS: dict[str, tuple[int, int]] = {
    "U": (0, 1),
    "D": (0, -1),
    "L": (-1, 0),
    "R": (1, 0),
}


class Outcome(Enum):
    """Kết quả sau một nước đi (``Game::check``: -1 / 1 / 0)."""

    CONTINUE = "continue"
    WON = "won"
    LOST = "lost"


CostFn = Callable[[State, str, State], float]


def unit_cost(state: State, action: str, next_state: State) -> float:
    """Mỗi lần lăn tốn 1 -> tối ưu theo số nước đi."""
    return 1.0


class BloxorzProblem:
    """Bài toán tìm kiếm trên một ``Level``.

    Args:
        level: màn chơi.
        cost_fn: hàm chi phí ``(state, action, next_state) -> float`` (> 0).
            Mặc định ``unit_cost``. TODO(nhóm, UCS): thiết kế hàm chi phí
            KHÔNG đồng nhất + lý giải trong báo cáo (xem Docs/report0.md 5.3).
    """

    def __init__(self, level: Level, cost_fn: CostFn = unit_cost) -> None:
        self.level = level
        self.cost_fn = cost_fn

    @classmethod
    def from_file(cls, path, cost_fn: CostFn = unit_cost) -> "BloxorzProblem":
        return cls(Level.from_file(path), cost_fn)

    @property
    def goal(self) -> tuple[int, int]:
        return self.level.goal

    def initial_state(self) -> State:
        x, y = self.level.start
        return State(x, y, self.level.start_orientation, 0)

    # ------------------------------------------------------------ luật game
    def check(self, state: State) -> Outcome:
        """Bản sao của ``Game::check`` (thua được ưu tiên hơn thắng)."""
        if state.is_split:
            assert state.cubes is not None
            c1, c2 = state.cubes
            lv, mask = self.level, state.bridge_mask
            for cx, cy in (c1, c2):
                if not lv.has_tile(cx, cy, mask) and (cx, cy) != lv.goal:
                    return Outcome.LOST
            # Khối tách: an toàn trên fragile, không bao giờ rơi qua goal nhưng không thắng
            return Outcome.CONTINUE

        lv, (x, y), o, mask = self.level, (state.x, state.y), state.orientation, state.bridge_mask
        gameover = won = False

        if not lv.has_tile(x, y, mask):
            if (x, y) != lv.goal:
                gameover = True
            elif o is Z:
                won = True
        elif o is Z and lv.tile_type(x, y, mask) == FRAGILE:
            gameover = True

        if o is X and not lv.has_tile(x + 1, y, mask) and (x + 1, y) != lv.goal:
            gameover = True
        if o is Y and not lv.has_tile(x, y + 1, mask) and (x, y + 1) != lv.goal:
            gameover = True

        if gameover:
            return Outcome.LOST
        return Outcome.WON if won else Outcome.CONTINUE

    def _activate_switch(self, mask: int, kind: str, x: int, y: int) -> int:
        """Áp tác dụng của công tắc lên các ô cầu (bit = ô cầu đang mở), như Floor::*_toggle."""
        switch = self.level.switch_at(kind, x, y)
        if switch.mode == "permanent_on":
            return mask | switch.mask  # mở mọi ô cầu, ô đang mở giữ nguyên
        if switch.mode == "permanent_off":
            return mask & ~switch.mask  # đóng mọi ô cầu, ô đang đóng giữ nguyên
        return mask ^ switch.mask  # toggle: đảo từng ô cầu

    def _toggle(self, mask: int, kind: str, x: int, y: int) -> int:
        return self._activate_switch(mask, kind, x, y)

    def simulate(self, state: State, action: str) -> tuple[State, Outcome]:
        """Thực hiện một nước đi, trả về (trạng thái mới, kết quả).

        Thứ tự giống hệt C++: lăn -> ``check`` -> (nếu chưa thắng/thua) kích hoạt
        công tắc TUẦN TỰ trên cấu hình cầu đang thay đổi -> ``check`` lại.
        """
        if state.is_split:
            assert state.cubes is not None
            c_idx = int(action[0])
            direction = action[1:]
            dx, dy = CUBE_DELTAS[direction]
            c_old = state.cubes[c_idx]
            c_other = state.cubes[1 - c_idx]
            c_new = (c_old[0] + dx, c_old[1] + dy)

            # Hai cube không được trùng ô
            if c_new == c_other:
                return state, Outcome.LOST

            s = State(0, 0, Z, state.bridge_mask, cubes=(c_new, c_other))
            outcome = self.check(s)
            if outcome is not Outcome.CONTINUE:
                return s, outcome

            # Kích hoạt công tắc thường bởi cube
            lv, mask = self.level, s.bridge_mask
            if lv.tile_type(c_new[0], c_new[1], mask) == NORMAL_SWITCH:
                mask = self._activate_switch(mask, "normal", c_new[0], c_new[1])

            if mask != s.bridge_mask:
                s = State(0, 0, Z, mask, cubes=s.cubes)
                if self.check(s) is Outcome.LOST:
                    return s, Outcome.LOST

            # Tự động gộp khối (Recombination) nếu 4-kề. Gộp chỉ đổi hình dạng, không phải
            # một lần "đè" mới: công tắc dưới cube vừa lăn đã được kích hoạt ở trên, cube kia
            # đã nằm sẵn, nên không kích hoạt lại (giống Game::check_after_step).
            assert s.cubes is not None
            c1, c2 = s.cubes
            if abs(c1[0] - c2[0]) == 1 and c1[1] == c2[1]:
                # Kề dọc -> gộp thành khối nằm theo trục X
                s = State(min(c1[0], c2[0]), c1[1], X, s.bridge_mask)
            elif c1[0] == c2[0] and abs(c1[1] - c2[1]) == 1:
                # Kề ngang -> gộp thành khối nằm theo trục Y
                s = State(c1[0], min(c1[1], c2[1]), Y, s.bridge_mask)

            return s, Outcome.CONTINUE

        # Khối nguyên lăn 1 bước
        new_o, dx, dy = TRANSITIONS[state.orientation][action]
        s = State(state.x + dx, state.y + dy, new_o, state.bridge_mask)

        outcome = self.check(s)
        if outcome is not Outcome.CONTINUE:
            return s, outcome  # nước thắng/thua: công tắc KHÔNG được kích hoạt

        # Đứng Z trên ô split switch -> tách khối và dịch chuyển
        if new_o is Z and self.level.split_switch_at(s.x, s.y) is not None:
            sp = self.level.split_switch_at(s.x, s.y)
            assert sp is not None
            s = State(0, 0, Z, s.bridge_mask, cubes=sp.targets)
            return s, self.check(s)

        lv, x, y, mask = self.level, s.x, s.y, s.bridge_mask
        if lv.tile_type(x, y, mask) == NORMAL_SWITCH:
            mask = self._activate_switch(mask, "normal", x, y)
        if new_o is X and lv.tile_type(x + 1, y, mask) == NORMAL_SWITCH:
            mask = self._activate_switch(mask, "normal", x + 1, y)
        if new_o is Y and lv.tile_type(x, y + 1, mask) == NORMAL_SWITCH:
            mask = self._activate_switch(mask, "normal", x, y + 1)
        if new_o is Z and lv.tile_type(x, y, mask) == HEAVY_SWITCH:
            mask = self._activate_switch(mask, "heavy", x, y)

        if mask != s.bridge_mask:
            s = State(x, y, new_o, mask)
            return s, self.check(s)  # frame sau: cầu dưới khối biến mất -> rơi
        return s, Outcome.CONTINUE

    # ------------------------------------------------- giao diện tìm kiếm
    def actions(self, state: State) -> list[str]:
        """Các hướng đi KHÔNG làm thua, theo thứ tự cố định."""
        candidates = SPLIT_ACTIONS if state.is_split else ACTIONS
        return [a for a in candidates if self.simulate(state, a)[1] is not Outcome.LOST]

    def result(self, state: State, action: str) -> State:
        next_state, outcome = self.simulate(state, action)
        if outcome is Outcome.LOST:
            raise ValueError(f"Nước đi {action} từ {state} làm thua")
        return next_state

    def successors(self, state: State) -> list[tuple[str, State, float]]:
        """Danh sách ``(action, next_state, cost)`` của các nước hợp lệ."""
        out = []
        candidates = SPLIT_ACTIONS if state.is_split else ACTIONS
        for a in candidates:
            next_state, outcome = self.simulate(state, a)
            if outcome is not Outcome.LOST:
                out.append((a, next_state, self.step_cost(state, a, next_state)))
        return out

    def is_goal(self, state: State) -> bool:
        if state.is_split:
            return False
        return self.check(state) is Outcome.WON

    def step_cost(self, state: State, action: str, next_state: State) -> float:
        return self.cost_fn(state, action, next_state)

    # ------------------------------------------------------------ tiện ích
    def replay(self, moves) -> tuple[State, Outcome, int]:
        """Phát lại chuỗi nước đi từ trạng thái đầu.

        Dừng ngay khi thắng/thua (như game). Trả về (trạng thái cuối, kết quả,
        số nước đã đi). ``moves`` có thể là list hoặc chuỗi "R U U R".
        """
        if isinstance(moves, str):
            moves = moves.split()
        state, outcome = self.initial_state(), Outcome.CONTINUE
        steps = 0
        for steps, a in enumerate(moves, start=1):
            valid_actions = SPLIT_ACTIONS if state.is_split else ACTIONS
            if a not in valid_actions:
                raise ValueError(f"Nước đi không hợp lệ: {a!r}")
            state, outcome = self.simulate(state, a)
            if outcome is not Outcome.CONTINUE:
                break
        return state, outcome, steps

    def is_valid_solution(self, moves) -> bool:
        """True nếu chuỗi nước đi thắng đúng ở nước CUỐI CÙNG."""
        moves = moves.split() if isinstance(moves, str) else list(moves)
        _, outcome, steps = self.replay(moves)
        return outcome is Outcome.WON and steps == len(moves)

    def path_cost(self, moves) -> float:
        """Tổng chi phí của một chuỗi nước đi hợp lệ."""
        moves = moves.split() if isinstance(moves, str) else list(moves)
        state, total = self.initial_state(), 0.0
        for a in moves:
            nxt = self.result(state, a)
            total += self.step_cost(state, a, nxt)
            state = nxt
        return total
