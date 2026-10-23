"""Các trường hợp biên của A*, độc lập với heuristic Manhattan chưa cài."""

from ai.heuristics import zero_heuristic
from ai.solvers import AStarSolver
from ai.state import Orientation, State
from tests.helpers import make_problem


class _GraphProblem:
    """Đồ thị nhỏ để kiểm tra tìm đường mà không phụ thuộc luật lăn khối."""

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


def test_astar_reports_no_solution():
    problem = make_problem(["100"], (0, 0), "Z", (0, 2))

    result = AStarSolver(problem, zero_heuristic).run()

    assert result.status == "no_solution"
    assert result.path is None and result.cost is None
    assert result.nodes_expanded == 1
    assert result.nodes_generated == 0


def test_astar_handles_start_already_at_goal():
    problem = make_problem(["0"], (0, 0), "Z", (0, 0))

    result = AStarSolver(problem, zero_heuristic).run()

    assert result.status == "ok"
    assert result.path == [] and result.cost == 0
    assert result.nodes_expanded == 0
    assert result.max_frontier == 1


def test_astar_waits_for_cheapest_goal_to_be_popped():
    start, a, goal = (State(x, 0, Orientation.Z) for x in range(3))
    problem = _GraphProblem(
        {
            start: [("direct", goal, 10.0), ("to_a", a, 1.0)],
            a: [("finish", goal, 1.0)],
        },
        start,
        goal,
    )

    result = AStarSolver(problem, zero_heuristic).run()

    assert result.path == ["to_a", "finish"]
    assert result.cost == 2.0


def test_astar_reopens_state_for_admissible_inconsistent_heuristic():
    start, a, b, goal = (State(x, 0, Orientation.Z) for x in range(4))
    problem = _GraphProblem(
        {
            start: [("to_a", a, 4.0), ("to_b", b, 1.0)],
            a: [("finish", goal, 4.0)],
            b: [("shortcut", a, 1.0)],
        },
        start,
        goal,
    )
    # h(b) = 5 là chi phí thật, nhưng lớn hơn cost(b, a) + h(a) = 1.
    estimates = {start: 0.0, a: 0.0, b: 5.0, goal: 0.0}

    result = AStarSolver(problem, lambda p, s: estimates[s]).run()

    assert result.path == ["to_b", "shortcut", "finish"]
    assert result.cost == 6.0
    assert problem.expanded == [start, a, b, a]


def test_astar_skips_stale_heap_entries():
    start, a, b, unreachable = (State(x, 0, Orientation.Z) for x in range(4))
    problem = _GraphProblem(
        {
            start: [("to_a", a, 5.0), ("to_b", b, 1.0)],
            b: [("shortcut", a, 1.0)],
        },
        start,
        unreachable,
    )

    result = AStarSolver(problem, zero_heuristic).run()

    assert result.status == "no_solution"
    assert problem.expanded == [start, b, a]
    assert result.nodes_expanded == 3
    assert result.nodes_generated == 3
