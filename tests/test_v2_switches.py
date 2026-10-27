"""Kiểm tra các cơ chế công tắc v2: điều khiển nhiều ô cầu và chế độ permanent."""

from __future__ import annotations

import pytest

from ai.level import Level
from ai.problem import BloxorzProblem, Outcome
from ai.state import Orientation, State


def test_v2_parse_multi_bridge_and_modes():
    # Ô đích (0, 4) phải là 0 trên lưới
    raw = """
3 5
1 1 1 1 0
1 3 0 0 1
1 1 1 1 1
0 0 Z
0 4
1
1 1 1 2
0
ext 1
switch normal 1 1 toggle 2 1 2 1 3
"""
    level = Level.from_text(raw, name="v2-test")
    # Công tắc tại (1, 1) trong ext 1 ghi đè công tắc cũ, điều khiển 2 ô (1, 2) và (1, 3)
    switch = level.switch_at("normal", 1, 1)
    assert switch.targets == ((1, 2), (1, 3))
    assert switch.mode == "toggle"
    assert (1, 2) in level.bridge_bits
    assert (1, 3) in level.bridge_bits
    assert level.bridge_bits[(1, 2)] == level.bridge_bits[(1, 3)]


def test_v2_multi_bridge_toggle():
    # Bản đồ 3x5, đích tại (2, 4) = 0
    raw = """
3 5
1 1 1 1 1
1 3 0 0 1
1 1 1 1 0
0 0 Z
2 4
0
0
ext 1
switch normal 1 1 toggle 2 1 2 1 3
"""
    prob = BloxorzProblem(Level.from_text(raw))
    s0 = prob.initial_state()
    # Ban đầu 2 ô cầu đang đóng (chưa có sàn)
    assert not prob.level.has_tile(1, 2, s0.bridge_mask)
    assert not prob.level.has_tile(1, 3, s0.bridge_mask)

    # Kích hoạt switch lần 1: cả 2 ô cầu đều bật lên
    mask1 = prob._activate_switch(0, "normal", 1, 1)
    assert prob.level.has_tile(1, 2, mask1)
    assert prob.level.has_tile(1, 3, mask1)

    # Dẫm lần 2: toggle lại -> cả 2 ô cầu tắt đi
    mask2 = prob._activate_switch(mask1, "normal", 1, 1)
    assert not prob.level.has_tile(1, 2, mask2)
    assert not prob.level.has_tile(1, 3, mask2)


def test_v2_permanent_on_switch():
    raw = """
3 5
1 1 1 1 1
1 3 0 1 1
1 1 1 1 0
0 0 Z
2 4
0
0
ext 1
switch normal 1 1 permanent_on 1 1 2
"""
    prob = BloxorzProblem(Level.from_text(raw))
    s0 = prob.initial_state()
    assert not prob.level.has_tile(1, 2, s0.bridge_mask)

    # Kích hoạt lần 1 -> cầu mở
    mask1 = prob._activate_switch(0, "normal", 1, 1)
    assert prob.level.has_tile(1, 2, mask1)

    # Kích hoạt lần 2 -> cầu VẪN MỞ (không bị đóng lại như toggle)
    mask2 = prob._activate_switch(mask1, "normal", 1, 1)
    assert prob.level.has_tile(1, 2, mask2)


def test_v2_permanent_off_switch():
    raw = """
3 5
1 1 1 1 1
1 4 0 1 1
1 1 1 1 0
0 0 Z
2 4
0
0
ext 1
switch heavy 1 1 permanent_off 1 1 2
"""
    prob = BloxorzProblem(Level.from_text(raw))
    # Kích hoạt switch -> bit được set
    mask1 = prob._activate_switch(0, "heavy", 1, 1)
    # Dẫm tiếp lần nữa -> vẫn giữ bit đã set
    mask2 = prob._activate_switch(mask1, "heavy", 1, 1)
    assert mask1 == mask2


def test_v2_multiple_switches_controlling_same_bridge_tile():
    raw = """
3 5
1 1 1 1 1
1 3 0 3 1
1 1 1 1 0
0 0 Z
2 4
0
0
ext 1
switch normal 1 1 toggle 1 1 2
switch normal 1 3 toggle 1 1 2
"""
    prob = BloxorzProblem(Level.from_text(raw))
    mask = 0
    assert not prob.level.has_tile(1, 2, mask)

    # Switch 1 bật -> mở cầu
    mask = prob._activate_switch(mask, "normal", 1, 1)
    assert prob.level.has_tile(1, 2, mask)

    # Switch 2 bật -> đóng cầu (XOR parity)
    mask = prob._activate_switch(mask, "normal", 1, 3)
    assert not prob.level.has_tile(1, 2, mask)

    # Switch 1 bật lại -> mở cầu trở lại
    mask = prob._activate_switch(mask, "normal", 1, 1)
    assert prob.level.has_tile(1, 2, mask)
