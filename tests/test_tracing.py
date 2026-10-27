"""TracingProblem (nút Visualize): ghi đúng thứ tự mở rộng mà không đổi kết quả solver."""

from __future__ import annotations

import main as cli
from ai import Orientation, State
from ai.heuristics import manhattan_heuristic
from ai.solvers import AStarSolver, BFSSolver
from ai.tracing import TracingProblem
from tests.helpers import level_path, load_problem, make_problem

X, Y, Z = Orientation.X, Orientation.Y, Orientation.Z


def test_bfs_expansion_order_matches_system_md_toy_trace():
    # Bảng 7.2 của Docs/system.md: A B C D E F G H I J K L M N rồi gặp đích O
    p = make_problem(["1111", "1221", "1101"], start=(2, 0), along="Z", goal=(2, 2))
    traced = TracingProblem(p)
    result = BFSSolver(traced).run()
    expanded = [traced.nodes[pid]["state"] for pid, _, _ in traced.steps]
    assert expanded[:4] == [State(2, 0, Z), State(2, 1, Y), State(0, 0, X), State(2, 3, Z)]
    assert result.path == BFSSolver(p).run().path


def test_trace_does_not_change_astar_result_and_counts_expansions():
    p = load_problem(1)
    traced = TracingProblem(p, heuristic=manhattan_heuristic, relax=True)
    result = AStarSolver(traced, heuristic=manhattan_heuristic).run()
    plain = AStarSolver(p, heuristic=manhattan_heuristic).run()
    assert result.path == plain.path and result.nodes_expanded == plain.nodes_expanded
    assert len(traced.steps) == result.nodes_expanded


def test_cli_trace_file_and_limit(tmp_path):
    trace, stats = tmp_path / "t.txt", tmp_path / "s.txt"
    code = cli.main([str(level_path(1)), "--algo", "bfs", "--trace", str(trace),
                     "--trace-limit", "20", "--stats", str(stats)])
    assert code == cli.EXIT_OK
    lines = trace.read_text(encoding="utf-8").splitlines()
    assert lines[0] == "BLOXORZ-TRACE 1" and "truncated 1" in lines[1]
    assert sum(line.startswith("S ") for line in lines) == 20
    assert lines[-1].startswith("P 0 ")
    assert "length 10" in stats.read_text(encoding="utf-8")
