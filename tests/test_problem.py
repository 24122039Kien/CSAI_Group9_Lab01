"""Kiểm tra mô phỏng luật (ai/problem.py) khớp với game C++."""

from __future__ import annotations

import pytest

from ai import Orientation, Outcome, State
from tests.helpers import SOLUTIONS, load_problem, make_level, make_problem

X, Y, Z = Orientation.X, Orientation.Y, Orientation.Z


# ------------------------------------------------------------------ đọc màn
def test_parse_level_1():
    p = load_problem(1)
    lv = p.level
    assert (lv.n, lv.m) == (13, 13)
    assert lv.start == (3, 3) and lv.start_orientation is Z
    assert lv.goal == (7, 7)
    assert [(s.kind, s.pos, s.target, s.bit) for s in lv.switches] == [
        ("normal", (2, 5), (6, 4), 0),
        ("normal", (10, 10), (9, 11), 1),
        ("heavy", (10, 6), (5, 6), 2),
    ]
    assert p.initial_state() == State(3, 3, Z, 0)


def test_parse_level_2():
    lv = load_problem(2).level
    assert (lv.n, lv.m) == (19, 14)
    assert lv.start == (4, 7) and lv.goal == (6, 10)
    assert [s.kind for s in lv.switches] == ["heavy"] * 3


def test_validate_goal_must_be_hole():
    with pytest.raises(ValueError):
        make_level(["111"], start=(0, 0), along="Z", goal=(0, 2))


def test_validate_switch_needs_mapping():
    with pytest.raises(ValueError):
        make_level(["1300"], start=(0, 0), along="Z", goal=(0, 3))


# ------------------------------------------------------- bảng chuyển trạng thái
OPEN = ["0" + "1" * 9] + ["1" * 10] * 9  # lưới 10x10 toàn sàn, đích (0, 0)

TRANSITION_CASES = [
    # (tư thế, hướng) -> (x, y, tư thế) từ ô (4, 4)  -- SuperBlock::update
    ("Z", "L", (2, 4, X)), ("Z", "R", (5, 4, X)), ("Z", "U", (4, 5, Y)), ("Z", "D", (4, 2, Y)),
    ("X", "L", (3, 4, Z)), ("X", "R", (6, 4, Z)), ("X", "U", (4, 5, X)), ("X", "D", (4, 3, X)),
    ("Y", "U", (4, 6, Z)), ("Y", "D", (4, 3, Z)), ("Y", "R", (5, 4, Y)), ("Y", "L", (3, 4, Y)),
]


@pytest.mark.parametrize("along,action,expected", TRANSITION_CASES)
def test_transition_table(along, action, expected):
    p = make_problem(OPEN, start=(4, 4), along=along, goal=(0, 0))
    s0 = p.initial_state()
    s1, outcome = p.simulate(s0, action)
    assert outcome is Outcome.CONTINUE
    assert (s1.x, s1.y, s1.orientation) == expected
    # Sau mỗi lần lăn, khối rời hoàn toàn các ô cũ và chạm cạnh khối cũ.
    old, new = set(s0.cells()), set(s1.cells())
    assert not old & new
    assert any(abs(a - c) + abs(b - d) == 1 for a, b in old for c, d in new)


# ------------------------------------------------------ lời giải đã biết
@pytest.mark.parametrize("n", [1, 2])
def test_known_solution_wins_on_last_move(n):
    p = load_problem(n)
    moves = SOLUTIONS[n].split()
    final, outcome, steps = p.replay(moves)
    assert outcome is Outcome.WON
    assert steps == len(moves)
    assert p.is_goal(final)
    assert p.is_valid_solution(moves)
    # Chưa thắng trước nước cuối.
    _, prefix_outcome, _ = p.replay(moves[:-1])
    assert prefix_outcome is Outcome.CONTINUE


@pytest.mark.parametrize("n", [1, 2])
def test_known_solution_cost_with_unit_cost(n):
    p = load_problem(n)
    assert p.path_cost(SOLUTIONS[n]) == len(SOLUTIONS[n].split())


def test_winning_move_does_not_trigger_switches():
    p = load_problem(1)
    moves = SOLUTIONS[1].split()
    before, _, _ = p.replay(moves[:-1])
    after, outcome = p.simulate(before, moves[-1])
    assert outcome is Outcome.WON
    assert after.bridge_mask == before.bridge_mask


# ------------------------------------------------------------- nước đi thua
def test_fall_off_map_level_1():
    p = load_problem(1)
    s0 = p.initial_state()
    assert p.simulate(s0, "L")[1] is Outcome.LOST
    assert p.simulate(s0, "D")[1] is Outcome.LOST
    assert p.actions(s0) == ["U", "R"]
    assert [a for a, _, _ in p.successors(s0)] == ["U", "R"]
    with pytest.raises(ValueError):
        p.result(s0, "L")


def test_out_of_bounds_is_a_fall():
    p = make_problem(["1110"], start=(0, 0), along="Z", goal=(0, 3))
    s0 = p.initial_state()
    assert p.simulate(s0, "L")[1] is Outcome.LOST  # x = -2
    assert p.simulate(s0, "D")[1] is Outcome.LOST  # y = -2
    assert p.simulate(s0, "R")[1] is Outcome.LOST  # x = 1, 2 ngoài lưới 1 dòng


FRAGILE = ["000", "010", "010", "020", "020", "000"]


def test_standing_on_fragile_loses_but_lying_is_safe():
    p = make_problem(FRAGILE, start=(1, 1), along="Z", goal=(5, 1))
    s1, o1 = p.simulate(p.initial_state(), "R")  # nằm trên (2,1) thường + (3,1) cam
    assert o1 is Outcome.CONTINUE and s1 == State(2, 1, X, 0)
    _, o2 = p.simulate(s1, "R")  # dựng đứng trên (4,1) cam
    assert o2 is Outcome.LOST


def test_lying_fully_on_fragile_is_safe():
    p = make_problem(FRAGILE, start=(3, 1), along="X", goal=(5, 1))
    assert p.check(p.initial_state()) is Outcome.CONTINUE


# ---------------------------------------------------------------- đích
def test_win_by_standing_on_goal():
    p = make_problem(["1110"], start=(0, 0), along="Z", goal=(0, 3))
    final, outcome, steps = p.replay("U U")
    assert outcome is Outcome.WON and steps == 2
    assert final == State(0, 3, Z, 0)


def test_lying_across_goal_hole_neither_wins_nor_loses():
    p = make_problem(["101"], start=(0, 0), along="Z", goal=(0, 1))
    s1, outcome = p.simulate(p.initial_state(), "U")  # Y chiếm (0,1)=lỗ đích và (0,2)
    assert outcome is Outcome.CONTINUE
    assert not p.is_goal(s1)


# ------------------------------------------------------ công tắc và cầu
SWITCH = ["00000", "01130", "00000", "00000", "00000"]  # công tắc thường (1,3) -> cầu (3,1)


def test_normal_switch_toggles_when_lying_on_it():
    p = make_problem(SWITCH, start=(1, 1), along="Z", goal=(4, 4), normal=[(1, 3, 3, 1)])
    lv = p.level
    s1, o1 = p.simulate(p.initial_state(), "U")  # Y trên (1,2),(1,3)
    assert o1 is Outcome.CONTINUE and s1.bridge_mask == 1
    assert lv.has_tile(3, 1, s1.bridge_mask) and not lv.has_tile(3, 1, 0)
    s2, _ = p.simulate(s1, "D")  # về đứng (1,1)
    s3, _ = p.simulate(s2, "U")  # đè lên công tắc lần nữa -> tắt cầu
    assert s2.bridge_mask == 1
    assert s3 == State(1, 2, Y, 0) and s3 != s1  # cùng vị trí, khác cầu -> khác trạng thái


HEAVY = ["00000", "04110", "00000", "00000", "00000"]  # công tắc nặng (1,1) -> cầu (3,1)


def test_heavy_switch_triggers_only_when_standing():
    standing = make_problem(HEAVY, start=(1, 2), along="Y", goal=(4, 4), heavy=[(1, 1, 3, 1)])
    s, outcome = standing.simulate(standing.initial_state(), "D")  # Y(1,2) -> đứng (1,1)
    assert outcome is Outcome.CONTINUE and s == State(1, 1, Z, 1)

    lying = make_problem(HEAVY, start=(1, 3), along="Z", goal=(4, 4), heavy=[(1, 1, 3, 1)])
    s, outcome = lying.simulate(lying.initial_state(), "D")  # đứng (1,3) -> nằm (1,1),(1,2)
    assert outcome is Outcome.CONTINUE and s == State(1, 1, Y, 0)


def test_bridge_vanishing_under_block_loses():
    # (1,1) công tắc A và (2,3) công tắc B cùng điều khiển cầu (3,3).
    rows = ["00000", "03110", "00030", "00000", "00000", "00000"]
    p = make_problem(rows, start=(1, 3), along="Z", goal=(5, 4), normal=[(1, 1, 3, 3), (2, 3, 3, 3)])
    s1, o1 = p.simulate(p.initial_state(), "D")  # nằm trên A -> cầu mở
    assert o1 is Outcome.CONTINUE and p.level.has_tile(3, 3, s1.bridge_mask)
    s2, o2 = p.simulate(s1, "U")  # đứng lại (1,3)
    assert o2 is Outcome.CONTINUE and s2 == State(1, 3, Z, s1.bridge_mask)
    _, o3 = p.simulate(s2, "R")  # nằm trên B + cầu -> B đóng cầu -> rơi
    assert o3 is Outcome.LOST


def test_bridge_state_is_part_of_state():
    a, b = State(1, 2, Z, 0), State(1, 2, Z, 1)
    assert a != b and len({a, b}) == 2
    assert hash(State(1, 2, Z, 1)) == hash(b)
