"""Kiểm tra các solver. Solver nào chưa cài (NotImplementedError) sẽ được SKIP."""

from __future__ import annotations

import pytest

import main as cli
from ai.heuristics import zero_heuristic
from ai.solvers import SOLVERS, AStarSolver, BaseSolver, SearchResult
from tests.helpers import OPTIMAL_LENGTH, SOLUTIONS, level_path, load_problem

OPTIMAL_SOLVERS = {"bfs", "ucs", "astar", "astar_zero"}


def build(name: str, problem):
    if name == "astar_zero":
        return AStarSolver(problem, heuristic=zero_heuristic)
    return SOLVERS[name](problem)


@pytest.mark.parametrize("level", [1, 2])
@pytest.mark.parametrize("name", ["bfs", "dfs", "ucs", "astar", "astar_zero"])
def test_solver_finds_valid_solution(name, level):
    problem = load_problem(level)
    try:
        result = build(name, problem).run()
    except NotImplementedError as exc:
        pytest.skip(f"{name} chưa cài: {exc}")

    assert result.found, f"{name} không tìm thấy lời giải cho màn {level}"
    assert problem.is_valid_solution(result.path), "lời giải không thắng khi phát lại"
    assert result.nodes_expanded > 0
    assert result.time_ms >= 0
    if name in OPTIMAL_SOLVERS:
        assert len(result.path) == OPTIMAL_LENGTH[level]
        assert result.cost == pytest.approx(OPTIMAL_LENGTH[level])


# -------------------------------------------- khung BaseSolver (đã cài sẵn)
class _ReplaySolver(BaseSolver):
    """Solver giả: trả về lời giải biết trước, chỉ để test khung đo đạc."""

    name = "replay"

    def __init__(self, problem, moves):
        super().__init__(problem)
        self.moves = moves

    def solve(self) -> SearchResult:
        self.nodes_expanded = 1
        return self.make_result(self.moves)


def test_base_solver_run_measures_time_and_memory():
    problem = load_problem(1)
    result = _ReplaySolver(problem, SOLUTIONS[1].split()).run(measure_memory=True)
    assert result.status == "ok" and result.solution_length == 10
    assert result.cost == 10
    assert result.time_ms >= 0 and result.peak_memory_kb is not None


def test_reconstruct_path():
    problem = load_problem(1)
    s0 = problem.initial_state()
    s1 = problem.result(s0, "R")
    s2 = problem.result(s1, "U")
    parent = {s0: None, s1: (s0, "R"), s2: (s1, "U")}
    assert BaseSolver.reconstruct_path(parent, s2) == ["R", "U"]
    assert BaseSolver.reconstruct_path(parent, s0) == []


# ------------------------------------------------------------------ CLI
def test_cli_reports_not_implemented_without_crashing(capsys, tmp_path):
    out = tmp_path / "solution.txt"
    try:
        SOLVERS["bfs"](load_problem(1)).run()
        pytest.skip("BFS đã được cài - test này chỉ dành cho khung ban đầu")
    except NotImplementedError:
        pass
    code = cli.main([str(level_path(1)), "--algo", "bfs", "--out", str(out)])
    assert code == cli.EXIT_NOT_IMPLEMENTED
    assert "chưa cài" in capsys.readouterr().out
    assert not out.exists()


def test_cli_compare_never_crashes(capsys):
    assert cli.main([str(level_path(1)), "--compare"]) == cli.EXIT_OK
    out = capsys.readouterr().out
    for name in SOLVERS:
        assert name in out


def test_cli_bad_level(tmp_path, capsys):
    bad = tmp_path / "bad.data"
    bad.write_text("2 2\n1 1\n", encoding="utf-8")
    assert cli.main([str(bad)]) == cli.EXIT_BAD_LEVEL
