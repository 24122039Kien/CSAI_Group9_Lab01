"""Kiểm thử cho UCSSolver: trường hợp biên, tối ưu theo tổng chi phí, bản ghi cũ, bộ màn, Visualizer."""

from __future__ import annotations

import pytest

from ai import BloxorzProblem, Level
from ai.heuristics import zero_heuristic
from ai.solvers import AStarSolver, BFSSolver, UCSSolver
from ai.state import Orientation, State
from ai.tracing import TracingProblem
from tests.helpers import OPTIMAL_LENGTH, level_path, load_problem, make_problem


class _GraphProblem:
    """Đồ thị nhỏ để kiểm tra UCS mà không phụ thuộc luật lăn khối."""

    def __init__(self, graph, start, goal):
        self.graph = graph
        self.start = start
        self.goal = goal
        self.expanded = []

    def initial_state(self):
        return self.start

    def is_goal(self, state):
        return state == self.goal

    def successors(self, state):
        self.expanded.append(state)
        return self.graph.get(state, [])

    def path_cost(self, path):
        state, total = self.start, 0.0
        for action in path:
            _, state, cost = next(
                edge for edge in self.graph.get(state, []) if edge[0] == action
            )
            total += cost
        return total


def _standing_costs_more(state: State, action: str, next_state: State) -> float:
    """Chi phí không đồng nhất chỉ dùng cho test: dựng đứng khối tốn 3, nước khác tốn 1."""
    return 3.0 if next_state.orientation is Orientation.Z else 1.0


def test_ucs_handles_start_already_at_goal():
    problem = make_problem(["0"], (0, 0), "Z", (0, 0))

    result = UCSSolver(problem).run()

    assert result.status == "ok"
    assert result.path == [] and result.cost == 0
    assert result.nodes_expanded == 0
    assert result.max_frontier == 1


def test_ucs_reports_no_solution():
    problem = make_problem(["100"], (0, 0), "Z", (0, 2))

    result = UCSSolver(problem).run()

    assert result.status == "no_solution"
    assert result.path is None and result.cost is None
    assert result.nodes_expanded == 1
    assert result.nodes_generated == 0


def test_ucs_prefers_cheaper_path_over_shorter_one():
    """Đường 1 bước tốn 10, đường 3 bước tốn 3: UCS chọn đường rẻ, BFS chọn đường ngắn."""
    start, a, b, goal = (State(x, 0, Orientation.Z) for x in range(4))
    graph = {
        start: [("direct", goal, 10.0), ("to_a", a, 1.0)],
        a: [("to_b", b, 1.0)],
        b: [("finish", goal, 1.0)],
    }

    result = UCSSolver(_GraphProblem(graph, start, goal)).run()

    assert result.path == ["to_a", "to_b", "finish"]
    assert result.cost == 3.0
    assert BFSSolver(_GraphProblem(graph, start, goal)).run().path == ["direct"]


def test_ucs_tests_goal_on_pop_not_on_generate():
    """Đích đã được sinh ra (g = 10) nhưng chỉ được nhận khi lấy ra với g nhỏ nhất (g = 2)."""
    start, a, goal = (State(x, 0, Orientation.Z) for x in range(3))
    problem = _GraphProblem(
        {
            start: [("direct", goal, 10.0), ("to_a", a, 1.0)],
            a: [("finish", goal, 1.0)],
        },
        start,
        goal,
    )

    result = UCSSolver(problem).run()

    assert result.path == ["to_a", "finish"]
    assert result.cost == 2.0
    assert problem.expanded == [start, a]


def test_ucs_skips_stale_heap_entries_without_counting_them():
    """``a`` vào heap hai lần (g = 5 rồi g = 2); bản ghi g = 5 bị bỏ qua, không tính là mở rộng."""
    start, a, b, unreachable = (State(x, 0, Orientation.Z) for x in range(4))
    problem = _GraphProblem(
        {
            start: [("to_a", a, 5.0), ("to_b", b, 1.0)],
            b: [("shortcut", a, 1.0)],
        },
        start,
        unreachable,
    )

    result = UCSSolver(problem).run()

    assert result.status == "no_solution"
    assert problem.expanded == [start, b, a]
    assert result.nodes_expanded == 3
    assert result.nodes_generated == 3


def test_ucs_breaks_ties_in_insertion_order_without_comparing_states():
    """Nhiều node cùng g: bộ đếm phá hòa (State không so sánh < được), lấy ra theo thứ tự sinh."""
    start, a, b, c, goal = (State(x, 0, Orientation.Z) for x in range(5))
    problem = _GraphProblem(
        {
            start: [("to_a", a, 1.0), ("to_b", b, 1.0), ("to_c", c, 1.0)],
            c: [("finish", goal, 1.0)],
        },
        start,
        goal,
    )

    result = UCSSolver(problem).run()

    assert result.path == ["to_c", "finish"]
    assert problem.expanded == [start, a, b, c]


def test_ucs_matches_bfs_layer_count_on_system_md_toy_level():
    """Màn đồ chơi Docs/system.md: với chi phí 1, UCS cho cùng độ dài lời giải với BFS."""
    problem = make_problem(["1111", "1221", "1101"], start=(2, 0), along="Z", goal=(2, 2))

    result = UCSSolver(problem).run()

    assert problem.is_valid_solution(result.path)
    assert result.solution_length == BFSSolver(problem).run().solution_length


@pytest.mark.parametrize("level", sorted(OPTIMAL_LENGTH))
def test_ucs_finds_optimal_solution_on_every_solvable_level(level: int):
    problem = load_problem(level)

    result = UCSSolver(problem).run()

    assert result.status == "ok"
    assert problem.is_valid_solution(result.path)
    assert result.solution_length == OPTIMAL_LENGTH[level]
    assert result.cost == pytest.approx(OPTIMAL_LENGTH[level])
    assert result.nodes_generated >= result.nodes_expanded > 0
    assert result.max_frontier >= 1


def test_ucs_level_11_explores_whole_reachable_region():
    """Màn 11 vô nghiệm: UCS mở rộng mỗi trạng thái tới được đúng một lần, như BFS."""
    problem = load_problem(11)

    result = UCSSolver(problem).run()

    assert result.status == "no_solution"
    assert result.nodes_expanded == BFSSolver(problem).run().nodes_expanded


@pytest.mark.parametrize("level", [1, 2, 4, 15, 16])
def test_ucs_is_cost_optimal_with_non_uniform_costs(level: int):
    """Chi phí không đồng nhất: tổng chi phí UCS = A* với h = 0 (cài độc lập), và không lớn hơn
    chi phí của lời giải ngắn nhất theo số bước."""
    problem = BloxorzProblem(Level.from_file(level_path(level)), cost_fn=_standing_costs_more)

    result = UCSSolver(problem).run()
    reference = AStarSolver(problem, zero_heuristic).run()
    shortest = BFSSolver(problem).run()

    assert problem.is_valid_solution(result.path)
    assert result.cost == pytest.approx(problem.path_cost(result.path))
    assert result.cost == pytest.approx(reference.cost)
    assert result.cost <= problem.path_cost(shortest.path) + 1e-9


@pytest.mark.parametrize("level", [1, 16])
def test_ucs_solution_follows_visualizer_tree(level: int):
    """Ghi vết với ``relax=True`` (như main.py) không đổi kết quả; cha trong cây khớp lời giải."""
    problem = load_problem(level)
    traced = TracingProblem(problem, relax=True, limit=10_000)  # màn 16 mở rộng 4600 node

    result = UCSSolver(traced).run()

    assert result.path == UCSSolver(problem).run().path
    assert len(traced.steps) == result.nodes_expanded

    state = problem.initial_state()
    for action in result.path:
        child = problem.result(state, action)
        node = traced.nodes[traced.ids[child]]
        assert node["parent"] == traced.ids[state]
        assert node["action"] == action
        state = child
