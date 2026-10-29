# Ghi chép cài đặt UCSSolver — Hai Dang

> **Ngày thực hiện:** 29/10/2026  
> **File đã sửa:** `ai/solvers/ucs_solver.py`  
> **Trạng thái:** ✅ Hoàn thành — 164 test passed, 3 skipped (chỉ DFS chưa cài)

> **Ghi chú khi merge vào `main` (29/10/2026):** file được chuyển từ `haidang.md` ở gốc repo vào
> `Docs/ucs_walkthrough.md` cho cùng chỗ với `bfs_walkthrough.md`, `astar_walkthrough.md`. DFS được
> merge cùng lúc, nên số liệu test bên dưới là tại thời điểm viết; hiện tại `python -m pytest -q`
> cho 220 passed, 1 skipped (thêm `tests/test_ucs_solver.py`). Bảng ở mục 3 nay có thể chạy lại
> đủ 4 thuật toán bằng `python main.py code/data/1.data --compare --memory`.

---

## 1. Phân tích cấu trúc repository

### Các file quan trọng đã đọc

| File | Vai trò |
|---|---|
| `README.md` | Tổng quan dự án Bloxorz Solver, cấu trúc thư mục, cách chạy |
| `ai/state.py` | Định nghĩa `State(x, y, orientation, bridge_mask, cubes)` — frozen dataclass, hashable |
| `ai/problem.py` | `BloxorzProblem`: luật game, `successors()` trả `list[(action, next_state, cost)]`, `is_goal()`, `step_cost()` dùng `cost_fn` |
| `ai/solvers/base_solver.py` | Lớp cơ sở `BaseSolver` + `SearchResult`: khung đo thời gian (`time.perf_counter`), bộ nhớ (`tracemalloc`), các bộ đếm |
| `ai/solvers/bfs_solver.py` | BFS hoàn chỉnh — hàng đợi FIFO, early goal test, `ParentMap` |
| `ai/solvers/astar_solver.py` | A\* hoàn chỉnh — `heapq` với `(f, h, counter, g, state)`, lazy deletion qua `best_g`, goal test on pop |
| `ai/solvers/ucs_solver.py` | **Trước khi sửa:** chỉ có stub `raise NotImplementedError` |
| `ai/tracing.py` | `TracingProblem` bọc problem để ghi vết — hỗ trợ `relax=True` cho UCS/A\* |
| `tests/test_solvers.py` | Test yêu cầu: UCS phải tìm lời giải **tối ưu** (màn 1 = 10 bước, màn 2 = 68 bước) |
| `main.py` | CLI: `--algo ucs`, `--compare`, `--memory`, `--trace` |

### Các quy ước nhóm đã xác lập

1. **Kiểu trả về:** `SearchResult` (tạo bằng `self.make_result(path)`)
2. **Bộ đếm:** `self.nodes_expanded` (node lấy ra VÀ sinh con), `self.nodes_generated` (tổng con sinh ra), `self.max_frontier`
3. **Đo thời gian:** `BaseSolver.run()` dùng `time.perf_counter()` → ms
4. **Đo bộ nhớ:** `tracemalloc` chạy ở lượt riêng (tham số `measure_memory`)
5. **Chi phí bước:** `self.problem.successors(state)` trả `(action, next_state, cost)`, mặc định `unit_cost = 1.0`
6. **Truy vết:** `BaseSolver.reconstruct_path(parent, state)` qua `ParentMap`

---

## 2. Các bước đã thực hiện

### Bước 1 — Đọc và hiểu toàn bộ codebase

- Đọc `README.md` để nắm kiến trúc tổng thể
- Đọc `state.py` → hiểu `State` là frozen dataclass, hashable, dùng làm key trong dict/set
- Đọc `problem.py` → nắm giao diện `successors()`, `is_goal()`, `step_cost()`
- Đọc `base_solver.py` → hiểu khung `BaseSolver`, `SearchResult`, cách đo thời gian/bộ nhớ
- Đọc `bfs_solver.py` → tham khảo cách dùng `ParentMap`, cập nhật bộ đếm
- Đọc `astar_solver.py` → tham khảo kỹ thuật `heapq`, lazy deletion, `best_g`
- Đọc `tracing.py` → xác nhận UCS cần `relax=True` (đã có sẵn)
- Đọc `test_solvers.py` → biết yêu cầu: UCS thuộc `OPTIMAL_SOLVERS`, phải trả lời giải tối ưu

### Bước 2 — Thiết kế thuật toán UCS

```
UCS(problem):
  1. start ← problem.initial_state()
  2. best_g ← {start: 0.0}        # chi phí tốt nhất đã biết
  3. parent ← {start: None}        # truy vết đường đi
  4. frontier ← min-heap [(0.0, 0, start)]  # (g, counter, state)
  5. WHILE frontier ≠ ∅:
       (g, _, state) ← heappop(frontier)
       IF g > best_g[state]: CONTINUE    # lazy deletion
       IF is_goal(state): RETURN path     # goal test on pop → tối ưu
       nodes_expanded += 1
       FOR (action, next_state, cost) IN successors(state):
           nodes_generated += 1
           new_g ← g + cost
           IF new_g ≥ best_g.get(next_state, ∞): CONTINUE
           best_g[next_state] ← new_g
           parent[next_state] ← (state, action)
           heappush(frontier, (new_g, next(counter), next_state))
           max_frontier ← max(max_frontier, len(frontier))
  6. RETURN None  # vô nghiệm
```

**Điểm thiết kế quan trọng:**

| Quyết định | Lý do |
|---|---|
| Heap tuple `(g, counter, state)` | `counter` tăng dần phá hòa khi `g` bằng nhau → tránh so sánh `State` (không hỗ trợ `<`) |
| Lazy deletion (kiểm tra `g > best_g`) | Không cần `decrease-key` (heapq không hỗ trợ), đơn giản hóa code |
| Goal test **on pop** (không phải on generate) | Đảm bảo khi lấy ra, `g` là chi phí thấp nhất — tính tối ưu |
| Không dùng `closed set` riêng | `best_g` kiêm nhiệm: vừa lưu chi phí, vừa chống lặp |
| So sánh `new_g >= best_g` (dấu `>=`) | Chỉ push khi tìm được đường **strictly** tốt hơn → giảm entry thừa trong heap |

### Bước 3 — Cài đặt vào `ucs_solver.py`

Thay thế stub `raise NotImplementedError(...)` bằng implementation hoàn chỉnh, tuân thủ:
- Import `heapq`, `itertools.count`, `State`, `ParentMap`
- Cập nhật đúng 3 bộ đếm: `nodes_expanded`, `nodes_generated`, `max_frontier`
- Trả về `self.make_result(path)` hoặc `self.make_result(None)`
- Comment tiếng Việt theo phong cách nhóm

### Bước 4 — Chạy test kiểm chứng

```
$ py -3 -m pytest tests/test_solvers.py -v -k "ucs"
tests/test_solvers.py::test_solver_finds_valid_solution[ucs-1] PASSED
tests/test_solvers.py::test_solver_finds_valid_solution[ucs-2] PASSED

$ py -3 -m pytest -q
164 passed, 3 skipped    (3 skipped = DFS chưa cài)
```

### Bước 5 — Kiểm tra bằng CLI

```
$ py -3 main.py code/data/1.data --algo ucs
[ucs] số bước        : 10
[ucs] tổng chi phí   : 10
[ucs] node mở rộng   : 160
[ucs] node sinh ra   : 452
[ucs] frontier max   : 47
[ucs] thời gian      : 1.683 ms
[ucs] lời giải       : U L R D R U R U R U

$ py -3 main.py code/data/2.data --algo ucs
[ucs] số bước        : 68
[ucs] tổng chi phí   : 68
[ucs] node mở rộng   : 377
```

---

## 3. Kết quả so sánh (màn 1, `--compare --memory`)

| Thuật toán | Số bước | Chi phí | Mở rộng | Frontier max | Thời gian (ms) | Bộ nhớ (KB) |
|---|---|---|---|---|---|---|
| **BFS** | 10 | 10 | 125 | 36 | 1.367 | 18.2 |
| **DFS** | — | — | chưa cài | — | — | — |
| **UCS** | 10 | 10 | 160 | 47 | 1.702 | 36.9 |
| **A\*** | 10 | 10 | 81 | 60 | 1.128 | 24.2 |

### Nhận xét

- **UCS cho lời giải tối ưu** (10 bước = BFS = A\*) ✅
- UCS mở rộng **nhiều node hơn BFS** (160 vs 125) vì BFS kiểm tra đích sớm (early goal test) còn UCS kiểm tra khi lấy ra (late goal test)
- A\* mở rộng **ít nhất** (81 node) nhờ heuristic Manhattan dẫn đường
- Bộ nhớ UCS (36.9 KB) lớn hơn BFS (18.2 KB) do heap chứa entry thừa (lazy deletion)
- Với `unit_cost`, UCS tương đương BFS về chất lượng lời giải, nhưng linh hoạt hơn khi dùng chi phí không đồng nhất

---

## 4. So sánh UCS với A\* (code)

UCS thực chất là **A\* với h = 0**. Điểm khác biệt trong code:

```diff
 # A*: heap entry có 5 phần tử
-(f, h, counter, g, state)   # sắp xếp theo f = g + h
+# UCS: heap entry có 3 phần tử
+(g, counter, state)          # sắp xếp theo g (vì h = 0)
```

UCS không cần import heuristic, không tính `f = g + h`, code gọn hơn.

---

## 5. Tích hợp sẵn có

Không cần sửa thêm file nào khác vì nhóm đã chuẩn bị sẵn:

- `ai/solvers/__init__.py` → đã đăng ký `"ucs": UCSSolver` trong `SOLVERS`
- `main.py` → CLI đã hỗ trợ `--algo ucs`
- `ai/tracing.py` → `TracingProblem` đã có `relax=True` cho UCS (ghi vết cho Visualizer)
- `tests/test_solvers.py` → đã có test cho UCS (trước đó bị SKIP do `NotImplementedError`)
- Game C++ → nút **Solve UCS** trong bảng AI đã sẵn sàng gọi `main.py --algo ucs`
