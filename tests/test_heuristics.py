"""Heuristic Manhattan (TV5): giá trị cụ thể, phản ví dụ và trường hợp split switch."""

from __future__ import annotations

import math

from ai import BloxorzProblem, Level, Orientation, State
from ai.heuristics import manhattan_heuristic
from ai.solvers import AStarSolver, BFSSolver
from tests.helpers import make_problem

X, Y, Z = Orientation.X, Orientation.Y, Orientation.Z

# Màn đồ chơi 3x4 dùng xuyên suốt Docs/system.md (đích (2,2), ô cam ở dòng giữa)
TOY = ["1111", "1221", "1101"]


def test_toy_level_values_match_system_md():
    p = make_problem(TOY, start=(2, 0), along="Z", goal=(2, 2))
    h = lambda s: manhattan_heuristic(p, s)  # noqa: E731
    assert h(State(2, 0, Z)) == 1  # A: d = 2
    assert h(State(2, 1, Y)) == 1  # B: vắt qua lỗ đích, d = 0 nhưng chưa thắng
    assert h(State(0, 0, X)) == 2  # C: d = 3
    assert h(State(0, 2, X)) == 1  # K: d = 1
    assert h(State(2, 2, Z)) == 0  # O: đích


def test_no_plus_one_for_lying_block():
    # Khối nằm X(1,0) chiếm (1,0),(2,0), đích (3,0): một nước R là thắng -> h phải <= 1
    p = make_problem(["1", "1", "1", "0"], start=(1, 0), along="X", goal=(3, 0))
    assert p.replay("R")[2] == 1 and p.is_valid_solution("R")
    assert manhattan_heuristic(p, p.initial_state()) == 1


PORTAL = """2 14
1 1 1 5 0 0 0 0 0 0 1 0 0 0
0 0 0 0 0 0 0 0 0 0 1 1 0 0
0 0 Z
1 12
0
0
ext 1
split 0 3 0 10 1 11
"""


def test_split_teleport_does_not_break_admissibility():
    # Split switch (0,3) xa đích nhưng đưa 2 cube tới sát đích (1,12):
    # U U (đứng lên switch) -> 0R (gộp) -> U (đứng vào đích) = 4 nước.
    p = BloxorzProblem(Level.from_text(PORTAL))
    assert p.is_valid_solution("U U 0R U")
    start = p.initial_state()
    naive = max(math.ceil((abs(0 - 1) + abs(0 - 12)) / 2), 1)
    assert naive == 7 > 4  # Manhattan thường đánh giá quá
    assert manhattan_heuristic(p, start) <= 4
    result = AStarSolver(p, heuristic=manhattan_heuristic).run()
    assert result.solution_length == BFSSolver(p).run().solution_length == 4
