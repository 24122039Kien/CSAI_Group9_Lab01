"""Depth-First Search (đề mục 3.3)."""

from __future__ import annotations

from ..state import State
from .base_solver import BaseSolver, ParentMap, SearchResult


class DFSSolver(BaseSolver):
    """DFS: đi sâu trước, KHÔNG đảm bảo tối ưu.

    TODO(TV2): cài đặt ``solve``.

    Gợi ý:
      * Dùng stack tường minh (list + ``pop()``), KHÔNG đệ quy (tránh
        RecursionError trên màn lớn).
      * Bắt buộc chống lặp (đề yêu cầu): tập/dict đã thăm theo ``State``.
      * Thứ tự đẩy vào stack quyết định hướng duyệt: nếu muốn duyệt theo thứ
        tự U, D, L, R thì đẩy theo thứ tự NGƯỢC lại. Ghi rõ trong báo cáo.
      * Nếu nhóm chọn IDS thay cho DFS: tạo ``ids_solver.py`` riêng và ghi rõ
        trong báo cáo (đề cho phép).
      * Cập nhật bộ đếm và trả về ``self.make_result(path)``.
    """

    name = "dfs"

    def solve(self) -> SearchResult:
        # 1. Khởi tạo: ngăn xếp chứa trạng thái đầu.
        start = self.problem.initial_state()
        stack: list[State] = [start]
        parent: ParentMap = {start: None}
        self.max_frontier = 1

        while stack:
            # 2. Lấy trạng thái vào SAU CÙNG (đỉnh ngăn xếp) -> đi sâu trước.
            state = stack.pop()

            # 3. Kiểm tra đích khi lấy ra.
            if self.problem.is_goal(state):
                path = self.reconstruct_path(parent, state)
                return self.make_result(path)

            # 4. Mở rộng: đẩy con chưa thăm, theo thứ tự ngược của successors().
            self.nodes_expanded += 1
            for action, nxt, _cost in reversed(self.problem.successors(state)):
                self.nodes_generated += 1

                if nxt not in parent:
                    parent[nxt] = (state, action)
                    stack.append(nxt)

            if len(stack) > self.max_frontier:
                self.max_frontier = len(stack)

        # 5. Đã xét hết các trạng thái tới được mà không gặp đích.
        return self.make_result(None)
