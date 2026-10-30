"""Breadth-First Search (đề mục 3.2)."""

from __future__ import annotations

from collections import deque

from ..state import State
from .base_solver import BaseSolver, ParentMap, SearchResult


class BFSSolver(BaseSolver):
    """BFS: duyệt theo từng tầng, tối ưu theo SỐ NƯỚC ĐI.

    Sử dụng hàng đợi FIFO (collections.deque).
    Tập parent vừa làm bảng visited (tránh chu trình và trạng thái trùng lặp),
    vừa phục vụ truy vết đường đi nghiệm từ đích về gốc.
    """

    name = "bfs"

    def solve(self) -> SearchResult:
        start = self.problem.initial_state()

        # Trường hợp biên: Trạng thái bắt đầu đã là đích
        if self.problem.is_goal(start):
            self.max_frontier = 1
            return self.make_result([])

        # Hàng đợi FIFO và tập cha/đã thăm
        frontier: deque[State] = deque([start])
        parent: ParentMap = {start: None}
        self.max_frontier = 1

        while frontier:
            state = frontier.popleft()
            self.nodes_expanded += 1

            # Đếm đủ mọi con đã sinh, kể cả các con đứng sau đích khi
            # Early Goal Test trả về giữa vòng lặp (cùng quy ước với DFS/UCS/A*).
            children = self.problem.successors(state)
            self.nodes_generated += len(children)

            for action, nxt, _cost in children:
                if nxt not in parent:
                    parent[nxt] = (state, action)

                    # Kiểm tra đích sớm (Early Goal Test)
                    if self.problem.is_goal(nxt):
                        path = self.reconstruct_path(parent, nxt)
                        return self.make_result(path)

                    frontier.append(nxt)
                    if len(frontier) > self.max_frontier:
                        self.max_frontier = len(frontier)

        return self.make_result(None)
