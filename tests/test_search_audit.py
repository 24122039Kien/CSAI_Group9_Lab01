"""Kiểm chứng chéo cả 4 solver với đáp án tính độc lập trên TOÀN BỘ đồ thị trạng thái.

Không dùng solver nào của nhóm làm chuẩn: đồ thị được liệt kê bằng ``successors()``,
khoảng cách thật tới đích tính bằng BFS ngược, chi phí tối ưu tính bằng Dijkstra riêng.
Xem Docs/report6.md. Màn 13 (47 680 trạng thái) được bỏ qua để bộ test chạy nhanh;
script kiểm chứng trong report6 đã chạy đủ cả 16 màn.
"""

from __future__ import annotations

import heapq
import zlib
from collections import Counter, deque
from functools import lru_cache

import pytest

from ai import BloxorzProblem, Level
from ai.heuristics import manhattan_heuristic, zero_heuristic
from ai.solvers import AStarSolver, BFSSolver, DFSSolver, UCSSolver
from tests.helpers import level_path

LEVELS = [lv for lv in range(1, 17) if lv != 13]
# Chi phí ngẫu nhiên: màn đại diện (công tắc, ô cam, nhiều cầu, split) cho bộ test nhanh.
RANDOM_COST_LEVELS = [1, 2, 4, 5, 9, 12, 15, 16]

SOLVERS = {
    "bfs": BFSSolver,
    "dfs": DFSSolver,
    "ucs": UCSSolver,
    "astar": lambda p: AStarSolver(p, manhattan_heuristic),
    "astar_zero": lambda p: AStarSolver(p, zero_heuristic),
}


class _Counting:
    """Bọc bài toán, ghi lại mỗi lần gọi ``successors()`` (= một lần mở rộng)."""

    def __init__(self, problem):
        self._problem = problem
        self.calls: Counter = Counter()
        self.order: list = []
        self.children = 0

    def __getattr__(self, name):
        return getattr(self._problem, name)

    def successors(self, state):
        out = self._problem.successors(state)
        self.calls[state] += 1
        self.order.append(state)
        self.children += len(out)
        return out


def _problem(level: int, cost_fn=None) -> BloxorzProblem:
    lvl = Level.from_file(level_path(level))
    return BloxorzProblem(lvl) if cost_fn is None else BloxorzProblem(lvl, cost_fn=cost_fn)


def _enumerate(problem):
    start = problem.initial_state()
    edges, queue = {start: None}, deque([start])
    while queue:
        s = queue.popleft()
        edges[s] = problem.successors(s)
        for _, t, _ in edges[s]:
            if t not in edges:
                edges[t] = None
                queue.append(t)
    return start, edges


@lru_cache(maxsize=None)
def _graph(level: int):
    """(bài toán, trạng thái đầu, cạnh, khoảng cách thật tới đích theo số bước)."""
    problem = _problem(level)
    start, edges = _enumerate(problem)
    reverse = {s: [] for s in edges}
    for s, out in edges.items():
        for _, t, _ in out:
            reverse[t].append(s)
    dist = {s: 0 for s in edges if problem.is_goal(s)}
    queue = deque(dist)
    while queue:
        t = queue.popleft()
        for s in reverse[t]:
            if s not in dist:
                dist[s] = dist[t] + 1
                queue.append(s)
    return problem, start, edges, dist


def _dijkstra(problem, start, edges):
    best, heap, tick = {start: 0.0}, [(0.0, 0, start)], 1
    while heap:
        g, _, s = heapq.heappop(heap)
        if g > best[s]:
            continue
        if problem.is_goal(s):
            return g
        for _, t, c in edges[s]:
            if g + c < best.get(t, float("inf")):
                best[t] = g + c
                heapq.heappush(heap, (g + c, tick, t))
                tick += 1
    return None


@pytest.mark.parametrize("name", sorted(SOLVERS))
@pytest.mark.parametrize("level", LEVELS)
def test_counters_match_real_work_and_no_state_is_expanded_twice(level: int, name: str):
    problem, start, edges, dist = _graph(level)
    counting = _Counting(problem)

    result = SOLVERS[name](counting).run()

    assert result.nodes_expanded == len(counting.order)
    assert result.nodes_generated == counting.children
    assert max(counting.calls.values(), default=1) == 1
    if start not in dist:
        assert result.status == "no_solution"
    elif name == "dfs":
        assert problem.is_valid_solution(result.path)
        assert result.solution_length >= dist[start]
    else:
        assert problem.is_valid_solution(result.path)
        assert result.solution_length == dist[start]


@pytest.mark.parametrize("level", LEVELS)
def test_ucs_is_exactly_astar_with_zero_heuristic(level: int):
    """Cùng hàng đợi theo g, cùng cách phá hòa -> cùng số liệu, cùng lời giải."""
    problem = _graph(level)[0]

    ucs = UCSSolver(problem).run()
    astar0 = AStarSolver(problem, zero_heuristic).run()

    assert ucs.path == astar0.path
    assert ucs.nodes_expanded == astar0.nodes_expanded
    assert ucs.nodes_generated == astar0.nodes_generated


@pytest.mark.parametrize("level", LEVELS)
def test_dfs_expands_in_reference_graph_search_order(level: int):
    """DFS đồ thị chuẩn (đánh dấu khi đẩy, đích khi lấy ra, U D L R) viết độc lập."""
    problem = _graph(level)[0]
    expected, stack = [], [problem.initial_state()]
    seen = set(stack)
    while stack:
        s = stack.pop()
        if problem.is_goal(s):
            break
        expected.append(s)
        for _, t, _ in reversed(problem.successors(s)):
            if t not in seen:
                seen.add(t)
                stack.append(t)
    counting = _Counting(problem)

    DFSSolver(counting).run()

    assert counting.order == expected


@pytest.mark.parametrize("level", LEVELS)
def test_manhattan_is_admissible_and_consistent_on_whole_graph(level: int):
    problem, _, edges, dist = _graph(level)
    for s, out in edges.items():
        h = manhattan_heuristic(problem, s)
        if problem.is_goal(s):
            assert h == 0
        if s in dist:
            assert h <= dist[s], (s, h, dist[s])
        for _, t, cost in out:
            assert h <= cost + manhattan_heuristic(problem, t), (s, t)


@pytest.mark.parametrize("seed", range(2))
@pytest.mark.parametrize("level", RANDOM_COST_LEVELS)
def test_cost_optimal_solvers_agree_with_dijkstra_on_random_costs(level: int, seed: int):
    """Chi phí ngẫu nhiên 1..7 mỗi nước (không đồng nhất, cố định theo cạnh)."""

    def cost_fn(state, action, next_state):
        return 1.0 + zlib.crc32(f"{seed}|{state}|{action}|{next_state}".encode()) % 7

    problem = _problem(level, cost_fn)
    start, edges = _enumerate(problem)
    expected = _dijkstra(problem, start, edges)

    for solver in (UCSSolver(problem), AStarSolver(problem, zero_heuristic),
                   AStarSolver(problem, manhattan_heuristic)):  # bước rẻ nhất = 1 -> vẫn admissible
        result = solver.run()
        assert result.cost == pytest.approx(expected)
        assert result.cost == pytest.approx(problem.path_cost(result.path))
