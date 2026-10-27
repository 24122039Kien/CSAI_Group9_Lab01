"""Kiểm thử chuyên sâu cho BFSSolver: tính tối ưu, trường hợp biên, map v2 (cầu đôi, split)."""

from __future__ import annotations

import pytest

from ai.solvers import BFSSolver
from ai.state import Orientation, State
from tests.helpers import OPTIMAL_LENGTH, load_problem, make_problem


class _GraphProblem:
    """Đồ thị trừu tượng để kiểm tra logic BFS độc lập với bàn cờ Bloxorz."""

    def __init__(self, graph: dict[State, list[tuple[str, State, float]]], start: State, goal: State) -> None:
        self.graph = graph
        self.start = start
        self.goal = goal
        self.expanded: list[State] = []

    def initial_state(self) -> State:
        return self.start

    def is_goal(self, state: State) -> bool:
        return state == self.goal

    def successors(self, state: State):
        self.expanded.append(state)
        return self.graph.get(state, [])

    def path_cost(self, path: list[str]) -> float:
        state = self.start
        total = 0.0
        for action in path:
            _, state, cost = next(edge for edge in self.graph.get(state, []) if edge[0] == action)
            total += cost
        return total


def test_bfs_start_already_at_goal():
    """Khi ô xuất phát đã trùng với ô đích, BFS phải trả về đường đi rỗng ngay lập tức."""
    problem = make_problem(["0"], (0, 0), "Z", (0, 0))
    result = BFSSolver(problem).run()

    assert result.status == "ok"
    assert result.path == []
    assert result.cost == 0
    assert result.nodes_expanded == 0
    assert result.max_frontier == 1


def test_bfs_reports_no_solution():
    """Bàn chơi bị chia cắt không có đường tới đích: BFS trả về no_solution."""
    problem = make_problem(["100"], (0, 0), "Z", (0, 2))
    result = BFSSolver(problem).run()

    assert result.status == "no_solution"
    assert result.path is None
    assert result.cost is None
    assert result.nodes_expanded >= 1


def test_bfs_avoids_cycles():
    """Đồ thị có chu trình kín: BFS không bị lặp vô tận nhờ bảng visited (parent)."""
    s0 = State(0, 0, Orientation.Z)
    s1 = State(1, 0, Orientation.Z)
    s2 = State(2, 0, Orientation.Z)
    goal = State(3, 0, Orientation.Z)

    # Vòng lặp giữa s0, s1, s2; từ s1 có nhánh dẫn tới goal
    graph = {
        s0: [("r", s1, 1.0)],
        s1: [("r", s2, 1.0), ("finish", goal, 1.0)],
        s2: [("loop", s0, 1.0)],
    }
    problem = _GraphProblem(graph, s0, goal)
    result = BFSSolver(problem).run()

    assert result.found
    assert result.path == ["r", "finish"]
    assert result.cost == 2.0
    # s0 chỉ được expand 1 lần duy nhất
    assert problem.expanded.count(s0) == 1


def test_bfs_finds_optimal_path_by_step_count():
    """BFS luôn tìm ra đường đi ít bước nhất trên đồ thị không trọng số."""
    start = State(0, 0, Orientation.Z)
    a1 = State(1, 0, Orientation.Z)
    a2 = State(2, 0, Orientation.Z)
    b1 = State(0, 1, Orientation.Z)
    goal = State(3, 3, Orientation.Z)

    # Đường dài: start -> a1 -> a2 -> goal (3 bước)
    # Đường ngắn: start -> b1 -> goal (2 bước)
    graph = {
        start: [("to_a1", a1, 1.0), ("to_b1", b1, 1.0)],
        a1: [("to_a2", a2, 1.0)],
        a2: [("to_goal_long", goal, 1.0)],
        b1: [("to_goal_short", goal, 1.0)],
    }
    problem = _GraphProblem(graph, start, goal)
    result = BFSSolver(problem).run()

    assert result.path == ["to_b1", "to_goal_short"]
    assert result.solution_length == 2


@pytest.mark.parametrize("level", [1, 2])
def test_bfs_solves_standard_levels_optimally(level: int):
    """BFS giải thành công các màn cơ bản 1 và 2 với số bước tối ưu tuyệt đối."""
    problem = load_problem(level)
    result = BFSSolver(problem).run()

    assert result.status == "ok"
    assert result.found
    assert problem.is_valid_solution(result.path)
    assert result.solution_length == OPTIMAL_LENGTH[level]
    assert result.cost == pytest.approx(OPTIMAL_LENGTH[level])
    assert result.nodes_expanded > 0
    assert result.nodes_generated >= result.nodes_expanded
    assert result.max_frontier >= 1


def test_bfs_solves_level_15_multi_bridge():
    """BFS giải thành công Level 15 (Multi-Bridge và Permanent Switch)."""
    problem = load_problem(15)
    result = BFSSolver(problem).run()

    assert result.status == "ok"
    assert result.found
    assert problem.is_valid_solution(result.path)
    assert result.solution_length == 37
    assert result.cost == 37.0


def test_bfs_solves_level_16_split_switch():
    """BFS giải thành công Level 16 (Stage 10 authentic: Split Switch & Recombination)."""
    problem = load_problem(16)
    result = BFSSolver(problem).run()

    assert result.status == "ok"
    assert result.found
    assert problem.is_valid_solution(result.path)
    assert result.solution_length == 49
    assert result.cost == 49.0
    # Lời giải phải chứa các action điều khiển cube riêng lẻ (ví dụ: '0U', '1R')
    assert any(len(a) == 2 and a[0] in "01" for a in result.path)
