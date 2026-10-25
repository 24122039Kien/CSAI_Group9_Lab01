"""Keep the new stress boards difficult and cross-check A* with independent BFS."""

from collections import deque
import json

import pytest

from ai.heuristics import zero_heuristic
from ai.problem import BloxorzProblem
from ai.solvers import AStarSolver
from tests.helpers import DATA

HARD_LEVELS = [
    entry for entry in json.loads((DATA / "catalog.json").read_text(encoding="utf-8"))["levels"]
    if entry.get("hard_level")
]


def bfs_certificate(problem):
    start = problem.initial_state()
    pending = deque([(start, 0)])
    seen = {start}
    masks = {start.bridge_mask}
    expanded = 0
    while pending:
        state, depth = pending.popleft()
        if problem.is_goal(state):
            return depth, expanded, masks
        expanded += 1
        for _, child, cost in problem.successors(state):
            assert cost == 1.0
            if child not in seen:
                seen.add(child)
                masks.add(child.bridge_mask)
                pending.append((child, depth + 1))
    return None, expanded, masks


def test_hard_suite_contains_four_and_six_switches_and_dense_fragile_terrain():
    assert {entry["normal_switches"] + entry["heavy_switches"] for entry in HARD_LEVELS} >= {4, 6}
    assert any(entry["fragile_tiles"] >= 100 for entry in HARD_LEVELS)


@pytest.mark.parametrize("entry", HARD_LEVELS, ids=lambda entry: f"level{entry['id']}")
def test_hard_board_matches_bfs_and_has_real_state_or_terrain_complexity(entry):
    problem = BloxorzProblem.from_file(DATA / entry["file"])
    optimum, expanded, masks = bfs_certificate(problem)
    result = AStarSolver(problem, heuristic=zero_heuristic).run()

    assert optimum == entry["optimal_length"]
    assert result.found and problem.is_valid_solution(result.path)
    assert result.solution_length == optimum
    assert result.cost == optimum
    assert expanded == entry["reference_nodes_expanded"]
    assert result.nodes_expanded == expanded

    if entry["normal_switches"] + entry["heavy_switches"]:
        switches = len(problem.level.switches)
        assert len(masks) == entry["expected_bridge_configurations"] == 2 ** switches
        assert expanded >= entry["minimum_reference_nodes"]
    else:
        state = problem.initial_state()
        touched = set()
        for action in result.path:
            state = problem.result(state, action)
            touched.update(cell for cell in state.cells() if problem.level.grid[cell[0]][cell[1]] == 2)
        assert len(touched) >= entry["minimum_fragile_visited"]
        assert entry["without_fragile_optimal_length"] < optimum
