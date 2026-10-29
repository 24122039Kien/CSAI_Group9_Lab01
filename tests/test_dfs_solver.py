"""Kiểm thử cho DFSSolver: thứ tự duyệt, chống lặp, trường hợp biên, toàn bộ bộ màn, Visualizer."""

from __future__ import annotations

import pytest

from ai.solvers import BFSSolver, DFSSolver
from ai.state import Orientation, State
from ai.tracing import TracingProblem
from tests.helpers import OPTIMAL_LENGTH, load_problem, make_problem


class _GraphProblem:
    """Đồ thị trừu tượng để kiểm tra logic DFS độc lập với bàn cờ Bloxorz."""

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


def test_dfs_start_already_at_goal():
    """Khi ô xuất phát đã trùng với ô đích, DFS trả về đường đi rỗng mà không mở rộng node nào."""
    problem = make_problem(["0"], (0, 0), "Z", (0, 0))
    result = DFSSolver(problem).run()

    assert result.status == "ok"
    assert result.path == []
    assert result.cost == 0
    assert result.nodes_expanded == 0
    assert result.max_frontier == 1


def test_dfs_reports_no_solution():
    """Bàn chơi bị chia cắt không có đường tới đích: DFS trả về no_solution."""
    problem = make_problem(["100"], (0, 0), "Z", (0, 2))
    result = DFSSolver(problem).run()

    assert result.status == "no_solution"
    assert result.path is None
    assert result.cost is None
    assert result.nodes_expanded >= 1


def test_dfs_avoids_cycles():
    """Đồ thị có chu trình kín: DFS không bị lặp vô tận nhờ bảng visited (parent)."""
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
    result = DFSSolver(problem).run()

    assert result.found
    assert result.path == ["r", "finish"]
    # s0 chỉ được expand 1 lần duy nhất
    assert problem.expanded.count(s0) == 1


def test_dfs_goes_deep_along_first_action():
    """DFS đi sâu theo hành động ĐẦU TIÊN của successors() trước, nên không tối ưu."""
    start = State(0, 0, Orientation.Z)
    a1 = State(1, 0, Orientation.Z)
    a2 = State(2, 0, Orientation.Z)
    b1 = State(0, 1, Orientation.Z)
    goal = State(3, 3, Orientation.Z)

    # Đường dài đi qua hành động đầu tiên: start -> a1 -> a2 -> goal (3 bước)
    # Đường ngắn đi qua hành động thứ hai: start -> b1 -> goal (2 bước, BFS sẽ chọn)
    graph = {
        start: [("to_a1", a1, 1.0), ("to_b1", b1, 1.0)],
        a1: [("to_a2", a2, 1.0)],
        a2: [("to_goal_long", goal, 1.0)],
        b1: [("to_goal_short", goal, 1.0)],
    }
    problem = _GraphProblem(graph, start, goal)
    result = DFSSolver(problem).run()

    assert result.path == ["to_a1", "to_a2", "to_goal_long"]
    assert problem.expanded == [start, a1, a2]


def test_dfs_matches_system_md_toy_trace():
    """Màn đồ chơi Docs/system.md mục 8.2: lời giải U U L D R, mở rộng 22 trạng thái."""
    problem = make_problem(["1111", "1221", "1101"], start=(2, 0), along="Z", goal=(2, 2))
    result = DFSSolver(problem).run()

    assert result.path == ["U", "U", "L", "D", "R"]
    assert result.nodes_expanded == 22


@pytest.mark.parametrize("level", sorted(OPTIMAL_LENGTH))
def test_dfs_solves_every_solvable_level(level: int):
    """DFS giải được mọi màn có lời giải; lời giải hợp lệ nhưng không ngắn hơn tối ưu."""
    problem = load_problem(level)
    result = DFSSolver(problem).run()

    assert result.status == "ok"
    assert problem.is_valid_solution(result.path)
    assert result.solution_length >= OPTIMAL_LENGTH[level]
    assert result.cost == pytest.approx(result.solution_length)
    assert result.nodes_expanded > 0
    assert result.nodes_generated >= result.nodes_expanded
    assert result.max_frontier >= 1


def test_dfs_level_11_explores_whole_reachable_region():
    """Màn 11 vô nghiệm: DFS duyệt hết vùng tới được, đúng bằng số trạng thái BFS đã duyệt."""
    problem = load_problem(11)
    result = DFSSolver(problem).run()

    assert result.status == "no_solution"
    assert result.nodes_expanded == BFSSolver(problem).run().nodes_expanded


def test_dfs_solves_level_16_split_switch():
    """Màn 16 (split switch): lời giải phải chứa các action điều khiển cube riêng lẻ."""
    problem = load_problem(16)
    result = DFSSolver(problem).run()

    assert problem.is_valid_solution(result.path)
    assert any(len(a) == 2 and a[0] in "01" for a in result.path)


@pytest.mark.parametrize("level", [1, 16])
def test_dfs_solution_follows_visualizer_tree(level: int):
    """Đánh dấu khi ĐẨY: cha của mỗi bước trong lời giải trùng cạnh cha-con mà Visualizer vẽ."""
    problem = load_problem(level)
    traced = TracingProblem(problem)
    result = DFSSolver(traced).run()

    assert result.path == DFSSolver(problem).run().path
    assert len(traced.steps) == result.nodes_expanded

    state = problem.initial_state()
    for action in result.path:
        child = problem.result(state, action)
        node = traced.nodes[traced.ids[child]]
        assert node["parent"] == traced.ids[state]
        assert node["action"] == action
        state = child
