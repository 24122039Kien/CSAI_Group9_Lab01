"""Depth-First Search (đề mục 3.3)."""

from __future__ import annotations

from .base_solver import BaseSolver, SearchResult


class DFSSolver(BaseSolver):
    """DFS: đi sâu trước, KHÔNG đảm bảo tối ưu.

    TODO(TV2): cài đặt ``solve``.

    Gợi ý:
      * Dùng stack tường minh (list + ``pop()``), KHÔNG đệ quy (tránh
        RecursionError trên màn lớn).
      * Bắt buộc chống lặp (đề yêu cầu): tập/dict đã thăm theo ``State``.
      * Thứ tự đẩy vào stack quyết định hướng duyệt: nếu muốn duyệt theo thứ
        tự U, D, L, R thì đẩy theo thứ tự NGƯỢC lại. Ghi rõ trong báo cáo.
      * Nếu nhóm chọn IDS thay cho DFS: tạo ``ids_solver.py`` riêng và ghi rõ
        trong báo cáo (đề cho phép).
      * Cập nhật bộ đếm và trả về ``self.make_result(path)``.
    """

    name = "dfs"

    def solve(self) -> SearchResult:
        raise NotImplementedError("DFSSolver.solve chưa được cài (TODO)")
