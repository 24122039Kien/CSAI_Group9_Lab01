"""Các hàm heuristic cho A*: ``h(problem, state) -> float``.

Yêu cầu của đề (mục 3.5): giải thích cách thiết kế, chứng minh admissible
(không bao giờ đánh giá quá chi phí thật) và/hoặc consistent - THEO ĐÚNG hàm
chi phí đang dùng trong ``BloxorzProblem``.
"""

from __future__ import annotations

from functools import lru_cache
from math import ceil

from .level import Level
from .problem import BloxorzProblem
from .state import State


def zero_heuristic(problem: BloxorzProblem, state: State) -> float:
    """h = 0: luôn admissible; A* với h = 0 chính là UCS (dùng để đối chứng)."""
    return 0.0


def _manhattan(a: tuple[int, int], b: tuple[int, int]) -> int:
    return abs(a[0] - b[0]) + abs(a[1] - b[1])


@lru_cache(maxsize=64)
def _portal_costs(level: Level) -> tuple[tuple[tuple[int, int], int], ...]:
    """Khoảng cách nới lỏng từ mỗi split switch tới đích, coi dịch chuyển tốn 0 bước.

    ``P(S) = min`` trên các điểm đến ``t`` của ``S`` của ``D(t)``, với ``D`` định nghĩa ở
    ``_relaxed_distance``. Vài split switch có thể nối tiếp nhau nên lặp kiểu
    Bellman-Ford tới khi ổn định (số split switch rất nhỏ).
    """
    portals = list(level.split_switches.values())
    cost = {sp.pos: min(_manhattan(t, level.goal) for t in sp.targets) for sp in portals}
    changed = True
    while changed:
        changed = False
        for sp in portals:
            best = min(
                min([_manhattan(t, level.goal)] + [_manhattan(t, q) + cost[q] for q in cost])
                for t in sp.targets
            )
            if best < cost[sp.pos]:
                cost[sp.pos] = best
                changed = True
    return tuple(cost.items())


@lru_cache(maxsize=1 << 16)
def _relaxed_distance(level: Level, cell: tuple[int, int]) -> int:
    """D(c): khoảng cách Manhattan từ ô ``c`` tới đích, cho phép đi tắt qua split switch.

    ``D(c) = min(|c - đích|, min_S |c - S| + P(S))``. Không có split switch thì ``D``
    chính là Manhattan. ``D`` là cực tiểu của các hàm 1-Lipschitz nên cũng 1-Lipschitz:
    hai ô cách nhau k (Manhattan) thì D chênh nhau không quá k.
    """
    d = _manhattan(cell, level.goal)
    for pos, portal in _portal_costs(level):
        d = min(d, _manhattan(cell, pos) + portal)
    return d


def manhattan_heuristic(problem: BloxorzProblem, state: State) -> float:
    """Heuristic Manhattan: số nước tối thiểu để khối có thể tới lỗ đích.

    ``h(s) = 0`` nếu ``s`` là đích, ngược lại ``h(s) = max(ceil(d / 2), 1)`` với
    ``d = min D(c)`` trên các ô ``c`` khối (hoặc hai cube) đang chiếm.

    Vì sao đúng (chi phí 1 mỗi nước, đúng cost model của ``main.py``):

    * Bảng ``TRANSITIONS``: sau một nước lăn, mỗi ô mới cách một ô cũ tối đa 2.
      Cube chỉ đi 1 ô; gộp cube không đổi ô. Vì ``D`` 1-Lipschitz, ``d`` giảm tối
      đa 2 mỗi nước. Nước đứng lên split switch: ``d`` mới ``>= D(S) >= d`` cũ ``- 2``
      vì ``D(S) <= P(S) <= D(t)`` với mọi điểm đến ``t``. Cầu, ô trống, ô cam bị bỏ
      qua (nới lỏng) nên chỉ làm ``D`` nhỏ đi.
    * Đích có ``d = 0``  =>  cần ít nhất ``ceil(d / 2)`` nước; chưa thắng thì cần
      ít nhất 1 nước => **admissible**.
    * ``ceil(d / 2)`` giảm tối đa 1 mỗi nước; nước vào đích xuất phát từ trạng
      thái có ``d <= 2`` nên ``h = 1`` = chi phí => **consistent**
      (``h(s) <= 1 + h(s')``).

    Không cộng "+1 khi đang nằm": khối nằm cách đích 1 ô dựng đứng vào đích chỉ
    bằng 1 nước, công thức đó cho 2 (đánh giá quá). Nếu dùng chi phí không đồng
    nhất, nhân kết quả với chi phí nhỏ nhất của một nước để giữ admissible.
    """
    if problem.is_goal(state):
        return 0.0
    level = problem.level
    d = min(_relaxed_distance(level, cell) for cell in state.cells())
    return float(max(ceil(d / 2), 1))


HEURISTICS = {
    "zero": zero_heuristic,
    "manhattan": manhattan_heuristic,
}
