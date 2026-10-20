"""Uniform-Cost Search (đề mục 3.4)."""

from __future__ import annotations

from .base_solver import BaseSolver, SearchResult


class UCSSolver(BaseSolver):
    """UCS: mở rộng node có chi phí đường đi g nhỏ nhất, tối ưu theo TỔNG CHI PHÍ.

    TODO(TV3): cài đặt ``solve`` + thiết kế hàm chi phí không đồng nhất
    (truyền vào ``BloxorzProblem(level, cost_fn=...)``) và lý giải trong báo cáo.

    Gợi ý:
      * Frontier: ``heapq`` với phần tử ``(g, counter, state)`` - ``counter``
        tăng dần để phá hòa, tránh so sánh hai State với nhau.
      * ``best_g: dict[State, float]``; khi pop ra mà ``g > best_g[state]`` thì
        bỏ qua (lazy deletion), KHÔNG tính là node mở rộng.
      * Kiểm tra đích khi LẤY RA khỏi heap (không phải khi sinh) -> đảm bảo tối ưu.
      * Chi phí bước: ``cost`` trong ``self.problem.successors(state)``.
      * Với chi phí mặc định = 1, UCS cho cùng độ dài với BFS.
    """

    name = "ucs"

    def solve(self) -> SearchResult:
        raise NotImplementedError("UCSSolver.solve chưa được cài (TODO)")
