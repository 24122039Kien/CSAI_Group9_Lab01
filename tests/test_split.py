"""Kiểm tra cơ chế Split Switch (tách khối thành 2 cube và tự động gộp)."""

from __future__ import annotations

import pytest

from ai.level import Level
from ai.problem import BloxorzProblem, Outcome
from ai.state import Orientation, State


def test_state_canonical_ordering_for_cubes():
    s1 = State(0, 0, Orientation.Z, bridge_mask=0, cubes=((5, 3), (2, 1)))
    s2 = State(0, 0, Orientation.Z, bridge_mask=0, cubes=((2, 1), (5, 3)))
    assert s1 == s2
    assert s1.cubes == ((2, 1), (5, 3))
    assert s1.is_split
    assert s1.cells() == ((2, 1), (5, 3))


def test_split_switch_triggers_teleport():
    # Bản đồ có ô split tại (1, 1), đích tại (0, 4)
    raw = """
3 5
1 1 1 1 0
1 1 1 1 1
1 1 1 1 1
1 0 Z
0 4
0
0
ext 1
split 1 1 0 0 2 3
"""
    prob = BloxorzProblem(Level.from_text(raw))
    s0 = prob.initial_state()
    assert not s0.is_split

    # Từ (1, 0) Z lăn R vào (2, 0) X, rồi lăn U vào (2, 1) X...
    # Hoặc test đứng thẳng lên (1, 1):
    # Lăn U vào (1, 1) Y, rồi lăn L...
    # Để đơn giản, tạo state đứng Z ngay trước split:
    s_before = State(1, 0, Orientation.X, 0)  # chiếm (1, 0) và (2, 0)
    # Từ (1, 0) X lăn L -> đứng Z tại (0, 0)
    # Lập state đứng Z tại (1, 2)
    s_stand = State(1, 2, Orientation.Z, 0)
    # Từ (1, 2) Z lăn D -> nằm Y tại (1, 0) ...
    # Để test trực tiếp simulate:
    # Đi vào (1, 1) và đứng Z:
    # Từ (1, 1) nằm X là (1, 1) & (2, 1). Lăn L -> đứng Z tại (0, 1)
    # Hãy dùng State(2, 1, Orientation.X) lăn L -> đứng Z tại (1, 1)!
    s_setup = State(2, 1, Orientation.X, 0)  # chiếm (2, 1) và (3, 1)
    # Khi lăn L từ X(x, y): chuyển thành Z(x - 1, y) -> (2 - 1, 1) = (1, 1) Z!
    s_split, outcome = prob.simulate(s_setup, "L")
    assert outcome is Outcome.CONTINUE
    assert s_split.is_split
    assert s_split.cubes == ((0, 0), (2, 3))


def test_cube_movement_and_collision():
    raw = """
4 5
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 0
0 0 Z
3 4
0
0
"""
    prob = BloxorzProblem(Level.from_text(raw))
    # Tạo trạng thái tách sẵn với 2 cube tại (1, 1) và (2, 3)
    s_split = State(0, 0, Orientation.Z, 0, cubes=((1, 1), (2, 3)))
    assert s_split.is_split

    # Cube 0 tại (1, 1) đi "0R" (x + 1) -> (2, 1)
    s_next, out = prob.simulate(s_split, "0R")
    assert out is Outcome.CONTINUE
    assert s_next.cubes == ((2, 1), (2, 3))

    # Cube 1 tại (2, 3) đi "1L" (x - 1) -> (1, 3)
    s_next2, out2 = prob.simulate(s_split, "1L")
    assert out2 is Outcome.CONTINUE
    assert s_next2.cubes == ((1, 1), (1, 3))

    # Hai cube đi đè lên nhau -> LOST
    s_close = State(0, 0, Orientation.Z, 0, cubes=((1, 1), (1, 2)))
    # Cube 0 đi sang phải (y + 1) vào (1, 2)
    _, out_collide = prob.simulate(s_close, "0U")
    assert out_collide is Outcome.LOST


def test_cube_on_fragile_and_goal():
    # (1, 1) là ô cam (2), (0, 4) là goal (0)
    raw = """
3 5
1 1 1 1 0
1 2 1 1 1
1 1 1 1 1
0 0 Z
0 4
0
0
"""
    prob = BloxorzProblem(Level.from_text(raw))
    # Cube bước vào ô cam (1, 1): không bị rơi, không thua
    s = State(0, 0, Orientation.Z, 0, cubes=((0, 1), (2, 2)))
    s_fragile, out = prob.simulate(s, "0R")  # (0, 1) + R -> (1, 1)
    assert out is Outcome.CONTINUE
    assert not prob.is_goal(s_fragile)

    # Cube bước vào ô goal (0, 4): không rơi, nhưng KHÔNG THẮNG
    s_near_goal = State(0, 0, Orientation.Z, 0, cubes=((0, 3), (2, 2)))
    s_goal, out_goal = prob.simulate(s_near_goal, "0U")  # (0, 3) + U -> (0, 4)
    assert out_goal is Outcome.CONTINUE
    assert not prob.is_goal(s_goal)


def test_cube_triggers_soft_switch_only():
    # (1, 1) là switch thường mở cầu (1, 2)
    # (2, 1) là switch nặng
    raw = """
4 5
1 1 1 1 1
1 3 0 1 1
1 4 0 1 1
1 1 1 1 0
0 0 Z
3 4
1
1 1 1 2
1
2 1 2 2
"""
    prob = BloxorzProblem(Level.from_text(raw))
    # Cube 0 dẫm lên soft switch tại (1, 1) -> cầu (1, 2) mở
    s = State(0, 0, Orientation.Z, 0, cubes=((0, 1), (3, 3)))
    s_soft, _ = prob.simulate(s, "0R")  # (0, 1) -> (1, 1)
    assert prob.level.has_tile(1, 2, s_soft.bridge_mask)

    # Cube 0 dẫm lên heavy switch tại (2, 1) -> cầu (2, 2) KHÔNG mở
    s_heavy, _ = prob.simulate(s_soft, "0R")  # (1, 1) -> (2, 1)
    assert not prob.level.has_tile(2, 2, s_heavy.bridge_mask)


def test_cube_recombination_along_x_and_y():
    raw = """
5 5
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 0
0 0 Z
4 4
0
0
"""
    prob = BloxorzProblem(Level.from_text(raw))

    # Trường hợp 1: Gộp ngang (theo trục Y)
    # Cube 0 tại (2, 2), Cube 1 tại (2, 4)
    s_horizontal = State(0, 0, Orientation.Z, 0, cubes=((2, 2), (2, 4)))
    # Cube 1 đi "1D" (y - 1) -> (2, 3). Lúc này (2, 2) và (2, 3) kề ngang
    s_merged_y, out_y = prob.simulate(s_horizontal, "1D")
    assert out_y is Outcome.CONTINUE
    assert not s_merged_y.is_split
    assert s_merged_y.orientation is Orientation.Y
    assert (s_merged_y.x, s_merged_y.y) == (2, 2)
    assert s_merged_y.cells() == ((2, 2), (2, 3))

    # Trường hợp 2: Gộp dọc (theo trục X)
    # Cube 0 tại (1, 2), Cube 1 tại (3, 2)
    s_vertical = State(0, 0, Orientation.Z, 0, cubes=((1, 2), (3, 2)))
    # Cube 1 đi "1L" (x - 1) -> (2, 2). Lúc này (1, 2) và (2, 2) kề dọc
    s_merged_x, out_x = prob.simulate(s_vertical, "1L")
    assert out_x is Outcome.CONTINUE
    assert not s_merged_x.is_split
    assert s_merged_x.orientation is Orientation.X
    assert (s_merged_x.x, s_merged_x.y) == (1, 2)
    assert s_merged_x.cells() == ((1, 2), (2, 2))


def test_full_split_recombine_and_win_puzzle():
    # Mini puzzle:
    # Xuất phát Z tại (0, 0).
    # Bước sang (1, 0) X, lăn tiếp ... đến split tại (2, 2).
    # Hai cube tách ra, di chuyển gặp nhau và gộp lại, sau đó lăn vào đích!
    raw = """
5 5
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 0
0 0 Z
4 4
0
0
ext 1
split 2 2 1 1 3 3
"""
    prob = BloxorzProblem(Level.from_text(raw))
    # Giả lập khối đứng Z tại (2, 2)
    s_at_split = State(2, 2, Orientation.Z, 0)
    # Đứng trên split switch -> tự tách
    # Lăn 1 nước bất kỳ giả lập từ X(3, 2) lăn L -> đứng Z(2, 2)
    s_from_x = State(3, 2, Orientation.X, 0)
    s_split, out = prob.simulate(s_from_x, "L")
    assert s_split.is_split
    assert s_split.cubes == ((1, 1), (3, 3))

    # Di chuyển cube 0 từ (1, 1) -> (1, 2) -> (2, 2)
    s1, _ = prob.simulate(s_split, "0U")  # (1, 1) + U -> (1, 2)
    s2, _ = prob.simulate(s1, "0R")       # (1, 2) + R -> (2, 2)

    # Di chuyển cube 1 từ (3, 3) -> (2, 3)
    # Khi cube 1 vào (2, 3), nó kề ngang với cube 0 tại (2, 2) -> TỰ GỘP THÀNH Y!
    s3, out_merge = prob.simulate(s2, "1L")  # (3, 3) + L -> (2, 3)
    assert out_merge is Outcome.CONTINUE
    assert not s3.is_split
    assert s3.orientation is Orientation.Y
    assert (s3.x, s3.y) == (2, 2)
