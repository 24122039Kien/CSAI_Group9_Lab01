"""A* Search (đề mục 3.5)."""

from __future__ import annotations

from typing import Callable

from ..heuristics import manhattan_heuristic
from ..problem import BloxorzProblem
from ..state import State
from .base_solver import BaseSolver, SearchResult

Heuristic = Callable[[BloxorzProblem, State], float]


class AStarSolver(BaseSolver):
    """A*: mở rộng node có f = g + h nhỏ nhất.

    Args:
        problem: bài toán.
        heuristic: hàm ``h(problem, state)``; mặc định ``manhattan_heuristic``.

    TODO(TV4): cài đặt ``solve``.

    Gợi ý:
      * Giống UCS nhưng khóa heap là ``(f, h, counter, state)`` với
        ``f = g + self.heuristic(self.problem, state)``; phá hòa theo h nhỏ.
      * Giữ ``best_g`` và lazy deletion như UCS; kiểm tra đích khi pop.
      * Heuristic admissible -> A* tối ưu; thêm consistent -> mỗi trạng thái
        chỉ cần mở rộng một lần.
      * Kiểm chứng: với cùng hàm chi phí, cost(A*) phải bằng cost(UCS) trên
        mọi màn; nếu khác -> heuristic không admissible hoặc có bug.
    """

    name = "astar"

    def __init__(self, problem: BloxorzProblem, heuristic: Heuristic = manhattan_heuristic) -> None:
        super().__init__(problem)
        self.heuristic = heuristic

    def solve(self) -> SearchResult:
        raise NotImplementedError("AStarSolver.solve chưa được cài (TODO)")
