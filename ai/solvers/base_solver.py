"""Lớp cơ sở cho mọi solver + cấu trúc kết quả dùng chung cho thực nghiệm."""

from __future__ import annotations

import time
import tracemalloc
from abc import ABC, abstractmethod
from dataclasses import dataclass, field

from ..problem import BloxorzProblem
from ..state import State

# parent[state] = (trạng thái cha, hành động đi từ cha tới state); gốc -> None
ParentMap = dict[State, "tuple[State, str] | None"]


@dataclass
class SearchResult:
    """Kết quả một lần chạy solver - đủ 4 chỉ số đề yêu cầu.

    Attributes:
        algorithm: tên thuật toán.
        path: danh sách hành động "U/D/L/R", hoặc None nếu không tìm thấy.
        cost: tổng chi phí của path (theo ``problem.step_cost``).
        nodes_expanded: số node đã lấy ra khỏi frontier VÀ sinh con
            (không tính node "cũ" bị bỏ qua trong hàng đợi ưu tiên).
        nodes_generated: số node con đã sinh ra.
        max_frontier: kích thước frontier lớn nhất trong quá trình tìm.
        time_ms: thời gian tìm kiếm (ms).
        peak_memory_kb: bộ nhớ đỉnh (KB, đo bằng tracemalloc) hoặc None nếu
            không đo. Nên đo ở lượt chạy riêng vì tracemalloc làm chậm.
        status: "ok" | "no_solution" | "not_implemented".
    """

    algorithm: str
    path: list[str] | None = None
    cost: float | None = None
    nodes_expanded: int = 0
    nodes_generated: int = 0
    max_frontier: int = 0
    time_ms: float = 0.0
    peak_memory_kb: float | None = None
    status: str = "ok"
    extra: dict = field(default_factory=dict)

    @property
    def found(self) -> bool:
        return self.path is not None

    @property
    def solution_length(self) -> int | None:
        return None if self.path is None else len(self.path)


class BaseSolver(ABC):
    """Khung chung: lớp con chỉ cần cài ``solve()``.

    Trong ``solve()`` hãy cập nhật các bộ đếm ``self.nodes_expanded``,
    ``self.nodes_generated``, ``self.max_frontier`` rồi trả về
    ``self.make_result(path)``. ``run()`` sẽ đo thời gian/bộ nhớ.
    """

    name = "base"

    def __init__(self, problem: BloxorzProblem) -> None:
        self.problem = problem
        self.reset_counters()

    def reset_counters(self) -> None:
        self.nodes_expanded = 0
        self.nodes_generated = 0
        self.max_frontier = 0

    @abstractmethod
    def solve(self) -> SearchResult:
        """Chạy thuật toán tìm kiếm, trả về ``SearchResult``."""

    def run(self, measure_memory: bool = False) -> SearchResult:
        """Gọi ``solve()`` kèm đo thời gian (và bộ nhớ đỉnh nếu được yêu cầu).

        Raises:
            NotImplementedError: nếu solver chưa được cài.
        """
        self.reset_counters()
        if measure_memory:
            tracemalloc.start()
        start = time.perf_counter()
        try:
            result = self.solve()
        finally:
            elapsed = (time.perf_counter() - start) * 1000.0
            peak = None
            if measure_memory:
                peak = tracemalloc.get_traced_memory()[1] / 1024.0
                tracemalloc.stop()
        result.time_ms = elapsed
        result.peak_memory_kb = peak
        return result

    # ------------------------------------------------------------ tiện ích
    def make_result(self, path: list[str] | None) -> SearchResult:
        """Đóng gói kết quả từ path + các bộ đếm hiện tại."""
        return SearchResult(
            algorithm=self.name,
            path=path,
            cost=None if path is None else self.problem.path_cost(path),
            nodes_expanded=self.nodes_expanded,
            nodes_generated=self.nodes_generated,
            max_frontier=self.max_frontier,
            status="ok" if path is not None else "no_solution",
        )

    @staticmethod
    def reconstruct_path(parent: ParentMap, state: State) -> list[str]:
        """Truy vết từ ``state`` về gốc qua ``parent`` -> danh sách hành động."""
        actions: list[str] = []
        entry = parent[state]
        while entry is not None:
            prev, action = entry
            actions.append(action)
            entry = parent[prev]
        actions.reverse()
        return actions
