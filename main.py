"""CLI chạy các solver Bloxorz.

Ví dụ:
    python main.py code/data/1.data --algo astar
    python main.py code/data/2.data --algo bfs --out solution.txt
    python main.py code/data/1.data --compare --memory

Mã thoát: 0 tìm được lời giải, 1 không có lời giải, 2 solver chưa cài,
3 lời giải sai luật (bug), 4 lỗi đọc màn chơi.
Game C++ (``code/autoplay.cpp``) gọi CLI này với ``--out`` rồi tự phát lại lời giải.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ai import BloxorzProblem, Level
from ai.heuristics import HEURISTICS
from ai.solvers import SOLVERS, SearchResult

EXIT_OK, EXIT_NO_SOLUTION, EXIT_NOT_IMPLEMENTED, EXIT_INVALID, EXIT_BAD_LEVEL = 0, 1, 2, 3, 4


def make_solver(name: str, problem: BloxorzProblem, heuristic: str):
    if name == "astar":
        return SOLVERS[name](problem, heuristic=HEURISTICS[heuristic])
    return SOLVERS[name](problem)


def run_one(name: str, problem: BloxorzProblem, heuristic: str, memory: bool) -> SearchResult:
    """Chạy một solver. Bộ nhớ được đo ở lượt chạy riêng để không ảnh hưởng thời gian."""
    try:
        result = make_solver(name, problem, heuristic).run()
        if memory:
            result.peak_memory_kb = make_solver(name, problem, heuristic).run(measure_memory=True).peak_memory_kb
    except NotImplementedError as exc:
        return SearchResult(algorithm=name, status="not_implemented", extra={"message": str(exc)})
    if result.found and not problem.is_valid_solution(result.path):
        result.status = "invalid"
    return result


def describe(result: SearchResult) -> str:
    if result.status == "not_implemented":
        return f"[{result.algorithm}] chưa cài ({result.extra.get('message', 'TODO')})"
    if result.status == "no_solution":
        return f"[{result.algorithm}] không tìm thấy lời giải (mở rộng {result.nodes_expanded} node)"
    lines = [
        f"[{result.algorithm}] số bước        : {result.solution_length}",
        f"[{result.algorithm}] tổng chi phí   : {result.cost:g}",
        f"[{result.algorithm}] node mở rộng   : {result.nodes_expanded}",
        f"[{result.algorithm}] node sinh ra   : {result.nodes_generated}",
        f"[{result.algorithm}] frontier max   : {result.max_frontier}",
        f"[{result.algorithm}] thời gian      : {result.time_ms:.3f} ms",
    ]
    if result.peak_memory_kb is not None:
        lines.append(f"[{result.algorithm}] bộ nhớ đỉnh    : {result.peak_memory_kb:.1f} KB")
    lines.append(f"[{result.algorithm}] lời giải       : {' '.join(result.path)}")
    if result.status == "invalid":
        lines.append(f"[{result.algorithm}] CẢNH BÁO: lời giải KHÔNG hợp lệ khi phát lại theo luật game!")
    return "\n".join(lines)


def print_table(results: list[SearchResult]) -> None:
    header = f"{'thuật toán':<10} {'trạng thái':<16} {'số bước':>8} {'chi phí':>8} {'mở rộng':>9} {'frontier':>9} {'ms':>10} {'KB':>9}"
    print(header)
    print("-" * len(header))
    for r in results:
        fmt = lambda v, spec: "-" if v is None else format(v, spec)  # noqa: E731
        print(
            f"{r.algorithm:<10} {r.status:<16} {fmt(r.solution_length, 'd'):>8} {fmt(r.cost, 'g'):>8} "
            f"{r.nodes_expanded:>9} {r.max_frontier:>9} {r.time_ms:>10.3f} {fmt(r.peak_memory_kb, '.1f'):>9}"
        )


def main(argv: list[str] | None = None) -> int:
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")  # console Windows
        except (AttributeError, ValueError):
            pass

    parser = argparse.ArgumentParser(description="Bloxorz solver (BFS / DFS / UCS / A*)")
    parser.add_argument("level", help="đường dẫn file màn chơi .data, ví dụ code/data/1.data")
    parser.add_argument("--algo", choices=sorted(SOLVERS), default="astar", help="thuật toán (mặc định: astar)")
    parser.add_argument("--heuristic", choices=sorted(HEURISTICS), default="manhattan", help="heuristic cho A*")
    parser.add_argument("--out", help="ghi chuỗi nước đi (vd 'R U U R') ra file này")
    parser.add_argument("--compare", action="store_true", help="chạy tất cả thuật toán và in bảng so sánh")
    parser.add_argument("--memory", action="store_true", help="đo thêm bộ nhớ đỉnh (lượt chạy riêng)")
    args = parser.parse_args(argv)

    try:
        level = Level.from_file(args.level)
    except (OSError, ValueError) as exc:
        print(f"Lỗi đọc màn chơi: {exc}", file=sys.stderr)
        return EXIT_BAD_LEVEL
    problem = BloxorzProblem(level)
    print(f"Màn: {level}")

    if args.compare:
        results = [run_one(name, problem, args.heuristic, args.memory) for name in SOLVERS]
        print_table(results)
        return EXIT_OK

    result = run_one(args.algo, problem, args.heuristic, args.memory)
    print(describe(result))
    if result.status == "not_implemented":
        return EXIT_NOT_IMPLEMENTED
    if result.status == "invalid":
        return EXIT_INVALID
    if not result.found:
        return EXIT_NO_SOLUTION
    if args.out:
        Path(args.out).write_text(" ".join(result.path) + "\n", encoding="utf-8")
        print(f"Đã ghi lời giải ra {args.out}")
    return EXIT_OK


if __name__ == "__main__":
    sys.exit(main())
