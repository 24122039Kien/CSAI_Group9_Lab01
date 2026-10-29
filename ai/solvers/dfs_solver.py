"""Depth-First Search (đề mục 3.3)."""

from __future__ import annotations

from ..state import State
from .base_solver import BaseSolver, ParentMap, SearchResult


class DFSSolver(BaseSolver):
    """DFS: đi sâu trước, KHÔNG đảm bảo tối ưu.

    Sử dụng ngăn xếp tường minh (list + ``pop()``), không đệ quy -> không bị
    RecursionError trên màn lớn.
    Tập parent vừa làm bảng visited (chống chu trình, đề yêu cầu), vừa phục vụ
    truy vết đường đi nghiệm từ đích về gốc.

    Quy ước (ghi trong báo cáo):
      * Đánh dấu đã thăm khi ĐẨY vào stack: mỗi trạng thái vào stack tối đa một
        lần, cha là node đầu tiên sinh ra nó (khớp cây của Search Visualizer).
      * Kiểm tra đích khi LẤY RA khỏi stack (mã giả ``Docs/system.md`` mục 8.1).
      * Đẩy con theo thứ tự NGƯỢC để lấy ra theo thứ tự U, D, L, R
        (0U … 1R khi khối đã tách).
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
