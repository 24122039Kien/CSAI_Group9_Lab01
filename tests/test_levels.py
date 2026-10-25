"""Chứng nhận bộ màn v1, lời giải tối ưu và vai trò thật của các cơ chế."""

from collections import deque
from functools import lru_cache
import json
import struct

import pytest

from ai.level import Level
from ai.problem import BloxorzProblem
from ai.state import Orientation
from tests.helpers import DATA, OPTIMAL_LENGTH, SOLUTIONS

CATALOG = json.loads((DATA / "catalog.json").read_text(encoding="utf-8"))["levels"]


@lru_cache(maxsize=None)
def shortest_length(level_number):
    return bfs_length(BloxorzProblem.from_file(DATA / f"{level_number}.data"))


def bfs_length(problem):
    """Chuẩn BFS độc lập, chỉ dùng trong test với chi phí mỗi nước bằng 1."""
    start = problem.initial_state()
    visited = {start}
    pending = deque([(start, 0)])
    while pending:
        state, depth = pending.popleft()
        if problem.is_goal(state):
            return depth
        for _, child, cost in problem.successors(state):
            assert cost == 1.0
            if child not in visited:
                visited.add(child)
                pending.append((child, depth + 1))
    return None


def test_catalog_has_ten_solvable_levels_and_a_nontrivial_unsolvable_case():
    assert sum(entry["status"] == "solvable" for entry in CATALOG) >= 10
    assert any(entry["status"] == "no_solution" for entry in CATALOG)
    identifiers = [entry["id"] for entry in CATALOG]
    assert identifiers == list(range(1, max(identifiers) + 1))
    assert {path.name for path in DATA.glob("*.data")} == {entry["file"] for entry in CATALOG}


@pytest.mark.parametrize("entry", CATALOG, ids=lambda entry: f"level{entry['id']}")
def test_level_metadata_and_solution_match_actual_board(entry):
    problem = BloxorzProblem.from_file(DATA / entry["file"])
    level = problem.level
    assert (level.n, level.m) == (entry["rows"], entry["columns"])
    assert sum(value != 0 for row in level.grid for value in row) == entry["tiles"]
    assert sum(value == 2 for row in level.grid for value in row) == entry["fragile_tiles"]
    assert sum(s.kind == "normal" for s in level.switches) == entry["normal_switches"]
    assert sum(s.kind == "heavy" for s in level.switches) == entry["heavy_switches"]
    assert len(level.bridge_bits) == entry["bridge_tiles"]
    assert shortest_length(entry["id"]) == entry["optimal_length"]

    if entry["status"] == "solvable":
        moves = (DATA / entry["solution_file"]).read_text(encoding="utf-8").split()
        assert problem.is_valid_solution(moves)
        assert len(moves) == entry["optimal_length"]
        assert SOLUTIONS[entry["id"]].split() == moves
        assert OPTIMAL_LENGTH[entry["id"]] == len(moves)
    else:
        assert entry["solution_file"] is None
        assert len(problem.successors(problem.initial_state())) > 0


@pytest.mark.parametrize("entry", [e for e in CATALOG if e["id"] > 2], ids=lambda entry: f"level{entry['id']}")
def test_new_levels_have_two_void_cells_around_the_board(entry):
    level = Level.from_file(DATA / entry["file"])
    for x in range(level.n):
        for y in range(level.m):
            if x < 2 or y < 2 or x >= level.n - 2 or y >= level.m - 2:
                assert level.grid[x][y] == 0
    assert 2 <= level.goal[0] < level.n - 2
    assert 2 <= level.goal[1] < level.m - 2
    assert level.grid[level.goal[0]][level.goal[1]] == 0
    assert level.grid[level.start[0]][level.start[1]] == 1


@pytest.mark.parametrize("entry", [e for e in CATALOG if e.get("fragile_must_change_optimum")], ids=lambda entry: f"level{entry['id']}")
def test_fragile_tiles_really_require_a_longer_route(entry):
    level = Level.from_file(DATA / entry["file"])
    floor = [[1 if value == 2 else value for value in row] for row in level.grid]
    baseline = BloxorzProblem(Level(floor, level.start, level.start_orientation, level.goal, [], []))
    assert bfs_length(baseline) == entry["without_fragile_optimal_length"]
    assert bfs_length(baseline) < entry["optimal_length"]


@pytest.mark.parametrize("entry", [e for e in CATALOG if e.get("every_switch_required")], ids=lambda entry: f"level{entry['id']}")
def test_every_switch_is_required_and_activated_correctly(entry):
    problem = BloxorzProblem.from_file(DATA / entry["file"])
    level = problem.level
    moves = (DATA / entry["solution_file"]).read_text(encoding="utf-8").split()
    activations = set()
    state = problem.initial_state()
    for action in moves:
        child, _ = problem.simulate(state, action)
        toggled = state.bridge_mask ^ child.bridge_mask
        for switch in level.switches:
            if toggled & (1 << switch.bit):
                activations.add(switch.bit)
                if switch.kind == "heavy":
                    assert child.orientation is Orientation.Z
        state = child
    assert activations == {switch.bit for switch in level.switches}

    for disabled in level.switches:
        grid = [list(row) for row in level.grid]
        grid[disabled.pos[0]][disabled.pos[1]] = 1
        normal = [(*s.pos, *s.target) for s in level.switches if s.kind == "normal" and s != disabled]
        heavy = [(*s.pos, *s.target) for s in level.switches if s.kind == "heavy" and s != disabled]
        without_switch = BloxorzProblem(Level(grid, level.start, level.start_orientation, level.goal, normal, heavy))
        assert bfs_length(without_switch) is None


@pytest.mark.parametrize("entry", CATALOG, ids=lambda entry: f"level{entry['id']}")
def test_previews_are_valid_pngs_with_readable_board_dimensions(entry):
    path = DATA / entry["preview"]
    with path.open("rb") as stream:
        header = stream.read(24)
    assert header[:8] == b"\x89PNG\r\n\x1a\n"
    width, height = struct.unpack(">II", header[16:24])
    assert width >= entry["columns"] * 32
    assert height >= entry["rows"] * 32
