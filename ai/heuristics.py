"""Các hàm heuristic cho A*: ``h(problem, state) -> float``.

Yêu cầu của đề (mục 3.5): giải thích cách thiết kế, chứng minh admissible
(không bao giờ đánh giá quá chi phí thật) và/hoặc consistent - THEO ĐÚNG hàm
chi phí đang dùng trong ``BloxorzProblem``.
"""

from __future__ import annotations

from .problem import BloxorzProblem
from .state import State


def zero_heuristic(problem: BloxorzProblem, state: State) -> float:
    """h = 0: luôn admissible; A* với h = 0 chính là UCS (dùng để đối chứng)."""
    return 0.0


def manhattan_heuristic(problem: BloxorzProblem, state: State) -> float:
    """Heuristic khoảng cách Manhattan tới lỗ đích.

    TODO(TV5 - heuristic): cài đặt và chứng minh admissible/consistent.

    Gợi ý:
      * ``state.cells()`` cho các ô khối đang chiếm, ``problem.goal`` là ô đích.
      * Lấy d = khoảng cách Manhattan NHỎ NHẤT từ các ô bị chiếm tới đích.
      * Mỗi nước lăn, ô gần đích nhất dịch chuyển tối đa 2 ô (xem bảng
        ``TRANSITIONS`` trong problem.py) -> cần ít nhất ceil(d / 2) nước.
      * Nếu chưa ở đích thì cần ít nhất 1 nước: dùng max(ceil(d / 2), 1).
        CẨN THẬN: cộng thêm "+1 nếu đang nằm" có thể KHÔNG admissible
        (phản ví dụ: khối nằm cách đích 1 ô chỉ cần 1 nước để dựng đứng).
      * Nếu dùng chi phí không đồng nhất: nhân với chi phí nhỏ nhất của một
        nước để vẫn admissible.
      * Trạng thái đích: trả về 0 (``problem.is_goal(state)``).
    """
    raise NotImplementedError("manhattan_heuristic chưa được cài (TODO)")


HEURISTICS = {
    "zero": zero_heuristic,
    "manhattan": manhattan_heuristic,
}
