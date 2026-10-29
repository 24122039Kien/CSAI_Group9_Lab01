"""Uniform-Cost Search (đề mục 3.4)."""

from __future__ import annotations

from heapq import heappop, heappush
from itertools import count

from ..state import State
from .base_solver import BaseSolver, ParentMap, SearchResult


class UCSSolver(BaseSolver):
    """UCS: mở rộng node có chi phí đường đi g nhỏ nhất, tối ưu theo TỔNG CHI PHÍ.

    Frontier dùng ``heapq`` với phần tử ``(g, counter, state)``:
      * ``g``: chi phí tích lũy từ trạng thái đầu đến ``state``.
      * ``counter``: số thứ tự tăng dần, phá hòa khi hai ``g`` bằng nhau
        (tránh so sánh trực tiếp hai đối tượng ``State``).

    ``best_g: dict[State, float]`` lưu chi phí thấp nhất đã biết đến mỗi
    trạng thái. Khi pop ra mà ``g > best_g[state]`` thì bỏ qua (lazy
    deletion) và KHÔNG tính vào ``nodes_expanded``.

    Kiểm tra đích khi **lấy ra** khỏi heap (không phải khi sinh con) để
    đảm bảo tính tối ưu.

    Với hàm chi phí mặc định ``unit_cost`` (mỗi bước = 1), UCS cho lời giải
    cùng độ dài tối ưu với BFS.
    """

    name = "ucs"

    def solve(self) -> SearchResult:
        # 1. Khởi tạo ─────────────────────────────────────────────────────
        start = self.problem.initial_state()
        best_g: dict[State, float] = {start: 0.0}
        parent: ParentMap = {start: None}
        counter = count()

        # Mỗi phần tử: (g, số thứ tự, trạng thái).
        frontier: list[tuple[float, int, State]] = []
        heappush(frontier, (0.0, next(counter), start))
        self.max_frontier = 1

        # 2. Vòng lặp chính ───────────────────────────────────────────────
        while frontier:
            g, _, state = heappop(frontier)

            # Bản ghi cũ: đã tìm được đường rẻ hơn đến state → bỏ qua.
            if g > best_g.get(state, float("inf")):
                continue

            # 3. Kiểm tra đích SAU KHI lấy ra (đảm bảo g tối ưu).
            if self.problem.is_goal(state):
                path = self.reconstruct_path(parent, state)
                return self.make_result(path)

            # 4. Mở rộng node: sinh các con hợp lệ.
            self.nodes_expanded += 1
            for action, next_state, step_cost in self.problem.successors(state):
                self.nodes_generated += 1
                new_g = g + step_cost

                # Chỉ xử lý nếu tìm được đường rẻ hơn đến next_state.
                if new_g >= best_g.get(next_state, float("inf")):
                    continue

                best_g[next_state] = new_g
                parent[next_state] = (state, action)
                heappush(frontier, (new_g, next(counter), next_state))
                self.max_frontier = max(self.max_frontier, len(frontier))

        # 5. Hết trạng thái mà chưa tìm thấy đích → vô nghiệm.
        return self.make_result(None)
