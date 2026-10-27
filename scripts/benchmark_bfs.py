"""Script đo đạc hiệu năng và sinh bảng đối sánh cho thuật toán BFS."""

from __future__ import annotations

import sys
from pathlib import Path

# Thêm thư mục gốc vào PYTHONPATH
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

if sys.stdout.encoding.lower() != "utf-8":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

from ai import BloxorzProblem, Level
from ai.solvers import BFSSolver


def run_benchmark():
    data_dir = ROOT / "code" / "data"
    level_files = sorted(
        [f for f in data_dir.glob("*.data") if f.stem.isdigit()],
        key=lambda f: int(f.stem),
    )

    print("=" * 90)
    print("                     BẢNG ĐÁNH GIÁ HIỆU NĂNG THUẬT TOÁN BFS")
    print("=" * 90)
    print(f"| {'Màn':^5} | {'Trạng thái':^12} | {'Số bước':^8} | {'Expanded':^10} | {'Generated':^10} | {'Max Queue':^10} | {'Thời gian (ms)':^14} | {'RAM (KB)':^10} |")
    print(f"|{'-'*7}|{'-'*14}|{'-'*10}|{'-'*12}|{'-'*12}|{'-'*12}|{'-'*16}|{'-'*12}|")

    for f in level_files:
        lvl_num = int(f.stem)
        try:
            level = Level.from_file(f)
            problem = BloxorzProblem(level)
            result = BFSSolver(problem).run(measure_memory=True)

            status_str = "THẮNG (OK)" if result.found else "VÔ NGHIỆM"
            steps_str = str(result.solution_length) if result.found else "—"
            time_str = f"{result.time_ms:.2f}"
            ram_str = f"{result.peak_memory_kb:.1f}" if result.peak_memory_kb is not None else "—"

            print(
                f"| {lvl_num:^5} | {status_str:^12} | {steps_str:^8} | "
                f"{result.nodes_expanded:^10} | {result.nodes_generated:^10} | {result.max_frontier:^10} | "
                f"{time_str:^14} | {ram_str:^10} |"
            )
        except Exception as e:
            print(f"| {lvl_num:^5} | {'LỖI':^12} | {str(e)[:40]} |")

    print("=" * 90)


if __name__ == "__main__":
    run_benchmark()
