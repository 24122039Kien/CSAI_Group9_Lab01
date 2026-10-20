"""Các solver tìm kiếm. Dùng ``SOLVERS[name](problem)`` để tạo solver theo tên."""

from .astar_solver import AStarSolver
from .base_solver import BaseSolver, SearchResult
from .bfs_solver import BFSSolver
from .dfs_solver import DFSSolver
from .ucs_solver import UCSSolver

SOLVERS: dict[str, type[BaseSolver]] = {
    "bfs": BFSSolver,
    "dfs": DFSSolver,
    "ucs": UCSSolver,
    "astar": AStarSolver,
}

__all__ = [
    "SOLVERS",
    "BaseSolver",
    "SearchResult",
    "BFSSolver",
    "DFSSolver",
    "UCSSolver",
    "AStarSolver",
]
