"""Ghi lại quá trình tìm kiếm để game vẽ cây tìm kiếm từng bước (nút "Visualize").

Không sửa code solver: ``TracingProblem`` bọc ``BloxorzProblem`` và ghi lại mỗi lần
solver gọi ``successors(state)``. Mỗi lần gọi đúng là một lần MỞ RỘNG node, nên thứ
tự các lần gọi chính là thứ tự thuật toán mở rộng (BFS theo lớp, DFS đi sâu, A* theo f).

Định dạng file (dòng chữ, game C++ đọc bằng ``>>``)::

    BLOXORZ-TRACE 1
    algo <tên> heuristic <tên|-> found <0|1> truncated <0|1>
    N <id> <cha|-1> <nước|-> <độ sâu> <g> <h> <x> <y> <X|Y|Z|S> <mask> <c1x> <c1y> <c2x> <c2y>
    S <id mở rộng> <số node frontier> <k> <id con> <n|u|o> ...   (n mới, u đổi cha vì g tốt hơn, o đã có)
    P <id> <id> ...                                                (lời giải, nếu tìm thấy)
"""

from __future__ import annotations

from pathlib import Path
from typing import Callable

from .problem import BloxorzProblem
from .state import State

Heuristic = Callable[[BloxorzProblem, State], float]


class TracingProblem:
    """Bọc một ``BloxorzProblem``; mọi thuộc tính khác chuyển thẳng cho bài toán gốc."""

    def __init__(self, problem: BloxorzProblem, heuristic: Heuristic | None = None,
                 relax: bool = False, limit: int = 3000) -> None:
        self._problem = problem
        self._heuristic = heuristic
        self._relax = relax          # UCS/A*: đổi cha khi tìm được g nhỏ hơn
        self.limit = limit           # số lần mở rộng tối đa được ghi (cây quá lớn thì không vẽ nổi)
        self.truncated = False
        self.ids: dict[State, int] = {}
        self.nodes: list[dict] = []
        self.steps: list[tuple[int, int, list[tuple[int, str]]]] = []
        self._expanded: set[int] = set()

    def __getattr__(self, name):
        return getattr(self._problem, name)

    # ------------------------------------------------------------------ ghi
    def _node(self, state: State, parent: int, action: str, g: float) -> int:
        h = self._heuristic(self._problem, state) if self._heuristic else 0.0
        depth = 0 if parent < 0 else self.nodes[parent]["depth"] + 1
        self.ids[state] = len(self.nodes)
        self.nodes.append({"state": state, "parent": parent, "action": action,
                           "depth": depth, "g": g, "h": h})
        return self.ids[state]

    def initial_state(self) -> State:
        s = self._problem.initial_state()
        if s not in self.ids:
            self._node(s, -1, "-", 0.0)
        return s

    def successors(self, state: State):
        out = self._problem.successors(state)
        if self.truncated or state not in self.ids:
            return out
        if len(self.steps) >= self.limit:
            self.truncated = True
            return out
        pid = self.ids[state]
        self._expanded.add(pid)
        g0 = self.nodes[pid]["g"]
        children: list[tuple[int, str]] = []
        for action, child, cost in out:
            g = g0 + cost
            cid = self.ids.get(child)
            if cid is None:
                children.append((self._node(child, pid, action, g), "n"))
            elif self._relax and g < self.nodes[cid]["g"] - 1e-12:
                node = self.nodes[cid]
                node.update(parent=pid, action=action, g=g, depth=self.nodes[pid]["depth"] + 1)
                children.append((cid, "u"))
            else:
                children.append((cid, "o"))
        frontier = len(self.nodes) - len(self._expanded)
        self.steps.append((pid, frontier, children))
        return out

    # ------------------------------------------------------------------ xuất
    def write(self, path: str | Path, algo: str, heuristic: str, solution: list[str] | None) -> None:
        lines = ["BLOXORZ-TRACE 1",
                 f"algo {algo} heuristic {heuristic} found {int(solution is not None)} "
                 f"truncated {int(self.truncated)}"]
        for i, n in enumerate(self.nodes):
            s: State = n["state"]
            o = "S" if s.is_split else s.orientation.value
            (c1x, c1y), (c2x, c2y) = s.cubes if s.cubes else ((-1, -1), (-1, -1))
            lines.append(f"N {i} {n['parent']} {n['action']} {n['depth']} {n['g']:g} {n['h']:g} "
                         f"{s.x} {s.y} {o} {s.bridge_mask} {c1x} {c1y} {c2x} {c2y}")
        for pid, frontier, children in self.steps:
            items = " ".join(f"{cid} {kind}" for cid, kind in children)
            lines.append(f"S {pid} {frontier} {len(children)} {items}".rstrip())
        if solution is not None:
            state, ids = self._problem.initial_state(), [self.ids.get(self._problem.initial_state(), -1)]
            for action in solution:
                state = self._problem.result(state, action)
                ids.append(self.ids.get(state, -1))
            lines.append("P " + " ".join(str(i) for i in ids))
        Path(path).write_text("\n".join(lines) + "\n", encoding="utf-8")
