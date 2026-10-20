"""Breadth-First Search (đề mục 3.2)."""

from __future__ import annotations

from .base_solver import BaseSolver, SearchResult


class BFSSolver(BaseSolver):
    """BFS: duyệt theo từng tầng, tối ưu theo SỐ NƯỚC ĐI.

    TODO(TV1): cài đặt ``solve``.

    Gợi ý:
      * Frontier: ``collections.deque`` (FIFO).
      * ``parent: dict[State, tuple[State, str] | None]`` vừa làm tập đã thăm,
        vừa để truy vết bằng ``self.reconstruct_path(parent, goal_state)``.
        Đánh dấu đã thăm KHI ĐẨY vào hàng đợi để tránh trùng trong frontier.
      * Trạng thái gồm cả ``bridge_mask`` -> cùng vị trí nhưng khác cầu là
        trạng thái khác (đã được State xử lý sẵn khi dùng làm khóa dict).
      * Sinh con: ``for action, nxt, cost in self.problem.successors(state)``.
      * Kiểm tra đích: ``self.problem.is_goal(state)`` (có thể kiểm tra ngay
        khi sinh con - vẫn tối ưu với BFS; ghi rõ cách đếm node trong báo cáo).
      * Cập nhật ``self.nodes_expanded``, ``self.nodes_generated``,
        ``self.max_frontier``; trả về ``self.make_result(path)`` (path=None nếu
        không có lời giải).
    """

    name = "bfs"

    def solve(self) -> SearchResult:
        raise NotImplementedError("BFSSolver.solve chưa được cài (TODO)")
