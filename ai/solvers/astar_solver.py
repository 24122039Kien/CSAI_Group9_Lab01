"""A* Search (đề mục 3.5)."""

from __future__ import annotations

from heapq import heappop, heappush
from itertools import count
from typing import Callable

from ..heuristics import manhattan_heuristic
from ..problem import BloxorzProblem
from ..state import State
from .base_solver import BaseSolver, ParentMap, SearchResult

Heuristic = Callable[[BloxorzProblem, State], float]


class AStarSolver(BaseSolver):
    """A*: mở rộng node có f = g + h nhỏ nhất.

    Args:
        problem: bài toán.
        heuristic: hàm ``h(problem, state)``; mặc định ``manhattan_heuristic``.

    Heap ưu tiên f nhỏ, rồi h nhỏ; counter phân biệt các bản ghi bằng điểm.
    Lưu cả g trong heap để nhận biết bản ghi cũ khi tìm được đường tốt hơn.
    Cho phép mở rộng lại trạng thái nếu g giảm, kể cả khi heuristic chưa
    consistent. Heuristic admissible và h(goal) = 0 bảo đảm lời giải tối ưu.
    """

    name = "astar"

    def __init__(self, problem: BloxorzProblem, heuristic: Heuristic = manhattan_heuristic) -> None:
        super().__init__(problem)
        self.heuristic = heuristic

    def solve(self) -> SearchResult:
        # 1. Khởi tạo: chưa đi nước nào nên chi phí tới trạng thái đầu là 0.
        start = self.problem.initial_state()
        best_g: dict[State, float] = {start: 0.0}
        parent: ParentMap = {start: None}
        counter = count()

        # Mỗi phần tử: (f = g + h, h, số thứ tự, g, trạng thái).
        frontier: list[tuple[float, float, int, float, State]] = []
        start_h = self.heuristic(self.problem, start)
        heappush(frontier, (start_h, start_h, next(counter), 0.0, start))
        self.max_frontier = 1

        # 2. Thử các trạng thái đang chờ, ưu tiên f nhỏ nhất.
        while frontier:
            _, _, _, g, state = heappop(frontier)

            # Bản ghi này thuộc đường cũ; đã có đường tới state rẻ hơn.
            if g != best_g[state]:
                continue

            # 3. Chỉ kiểm tra đích sau khi lấy bản ghi còn hiệu lực khỏi heap.
            if self.problem.is_goal(state):
                path = self.reconstruct_path(parent, state)
                return self.make_result(path)

            # 4. Sinh các nước hợp lệ; problem đã xử lý luật game.
            self.nodes_expanded += 1
            for action, next_state, step_cost in self.problem.successors(state):
                self.nodes_generated += 1
                new_g = g + step_cost

                # Chỉ lưu nếu đây là đường tốt hơn tới next_state.
                if new_g >= best_g.get(next_state, float("inf")):
                    continue

                best_g[next_state] = new_g
                parent[next_state] = (state, action)
                h = self.heuristic(self.problem, next_state)
                heappush(
                    frontier,
                    (new_g + h, h, next(counter), new_g, next_state),
                )
                self.max_frontier = max(self.max_frontier, len(frontier))

        # 5. Đã xét hết các trạng thái có thể tới mà không gặp đích.
        return self.make_result(None)
