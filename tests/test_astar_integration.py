"""Đối chiếu A*/heuristic với BFS độc lập trên toàn bộ đồ thị hai màn mẫu.

BFS chỉ dùng làm chuẩn kiểm chứng trong test, không thay solver của TV1.
Khi TV5 cài Manhattan, các trường hợp tương ứng sẽ tự chạy thay vì SKIP.
"""

from collections import defaultdict, deque
from dataclasses import dataclass
from math import isfinite

import pytest

from ai.heuristics import manhattan_heuristic, zero_heuristic
from ai.problem import BloxorzProblem
from ai.solvers import AStarSolver
from ai.state import State
from tests.helpers import OPTIMAL_LENGTH, load_problem


@dataclass
class _ReferenceGraph:
    problem: BloxorzProblem
    states: set[State]
    edges: list[tuple[State, State, float]]
    distances: dict[State, int]
    optimal_length: int


@pytest.fixture(scope="module", params=[1, 2, 15, 16], ids=["level1", "level2", "level15", "level16"])
def reference_graph(request):
    problem = load_problem(request.param)
    start = problem.initial_state()
    states = {start}
    pending = deque([start])
    edges = []
    reverse = defaultdict(list)
    goals = []

    # Duyệt mọi trạng thái đạt được; thắng là trạng thái kết thúc của game.
    while pending:
        state = pending.popleft()
        if problem.is_goal(state):
            goals.append(state)
            continue
        for _, child, cost in problem.successors(state):
            assert cost == 1.0, "Chuẩn BFS này dùng chi phí mỗi nước bằng 1."
            edges.append((state, child, cost))
            reverse[child].append(state)
            if child not in states:
                states.add(child)
                pending.append(child)

    # Công tắc có thể làm cạnh có hướng: BFS ngược từ tất cả trạng thái thắng.
    distances = {goal: 0 for goal in goals}
    pending = deque(goals)
    while pending:
        state = pending.popleft()
        for previous in reverse[state]:
            if previous not in distances:
                distances[previous] = distances[state] + 1
                pending.append(previous)

    return _ReferenceGraph(problem, states, edges, distances, OPTIMAL_LENGTH[request.param])


@pytest.fixture(
    scope="module",
    params=[zero_heuristic, manhattan_heuristic],
    ids=["zero", "manhattan"],
)
def heuristic(request, reference_graph):
    function = request.param
    problem = reference_graph.problem
    try:
        function(problem, problem.initial_state())
    except NotImplementedError:
        pytest.skip(f"{function.__name__} chưa được cài; chờ phần heuristic của TV5.")
    return function


def test_reference_bfs_matches_known_optimum(reference_graph):
    graph = reference_graph
    assert graph.distances[graph.problem.initial_state()] == graph.optimal_length


def test_heuristic_is_a_lower_bound_on_every_reachable_state(reference_graph, heuristic):
    graph = reference_graph
    for state in graph.states:
        estimate = heuristic(graph.problem, state)
        assert isfinite(estimate) and estimate >= 0, f"h không hợp lệ tại {state}"
        if graph.problem.is_goal(state):
            assert estimate == 0, f"h(goal) phải bằng 0 tại {state}"
        if state in graph.distances:
            assert estimate <= graph.distances[state] + 1e-9, (
                f"h({state}) = {estimate} vượt chi phí thật {graph.distances[state]}"
            )


def test_heuristic_is_consistent_on_every_reachable_edge(reference_graph, heuristic):
    graph = reference_graph
    for state, child, cost in graph.edges:
        assert heuristic(graph.problem, state) <= cost + heuristic(graph.problem, child) + 1e-9, (
            f"Heuristic không consistent trên cạnh {state} -> {child}"
        )


def test_astar_matches_independent_bfs(reference_graph, heuristic):
    graph = reference_graph
    result = AStarSolver(graph.problem, heuristic=heuristic).run()

    assert result.found
    assert graph.problem.is_valid_solution(result.path)
    assert result.cost == pytest.approx(graph.distances[graph.problem.initial_state()])
    assert result.solution_length == graph.optimal_length
