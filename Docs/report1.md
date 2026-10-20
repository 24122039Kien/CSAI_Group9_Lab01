# Report 1 — Việc còn lại và hướng dẫn cho từng thành viên

> **Dành cho:** cả 5 thành viên nhóm 9 · **Đọc trong:** ~15 phút · **Cập nhật:** 20/10/2026
>
> Khung repo đã chạy được: game 3D tự phát lại lời giải, luật game trong Python đã cài và test đầy đủ. Tài liệu này liệt kê **mọi thứ còn thiếu**, và với từng người: **làm file nào, dùng hàm nào, khi nào coi là xong**.
>
> Tài liệu liên quan: [README](../README.md) (cài đặt, chạy) · [report0.md](report0.md) (phân tích đề, luật game ở mục 2.6) · [task.md](task.md) (ID task, giờ, hạn, để đưa vào Notion) · [implementation_plan.md](implementation_plan.md) (thiết kế kỹ thuật).

---

## 1. Repo đang có gì (đọc 2 phút)

| Đã xong | Ở đâu |
|---|---|
| Game 3D C++/OpenGL (của Chaitanya Patel, thầy cho phép dùng) | `code/` |
| Game tự phát lại lời giải: `./a.out 1 --play file`, `./a.out 2 --solve bfs` | `code/autoplay.cpp`, `code/main.cpp` |
| Đọc file màn chơi `.data` | `ai/level.py` |
| Trạng thái `State(x, y, orientation, bridge_mask)` | `ai/state.py` |
| **Luật game** (lăn, rơi, ô cam, công tắc, cầu, thắng) chép đúng từ C++ | `ai/problem.py` |
| Khung solver: đo thời gian/bộ nhớ, truy vết đường đi | `ai/solvers/base_solver.py` |
| CLI chạy solver | `main.py` |
| 36 test luật (PASS) + 10 test solver (đang SKIP vì chưa cài) | `tests/` |

**Những gì CHƯA có** được liệt kê ở mục 3. Tóm tắt: 4 thuật toán, heuristic, hàm chi phí, 3 cơ chế đề yêu cầu mà game gốc không có (split, permanent, một công tắc nhiều ô cầu), nút bấm trên GUI, ≥ 10 màn chơi, thực nghiệm, báo cáo, video.

---

## 2. Bắt đầu trong 20 phút

```bash
# 1. Lấy code
git clone https://github.com/24122039Kien/CSAI_Group9_Lab01.git
cd CSAI_Group9_Lab01

# 2. Cài đặt (WSL Ubuntu trên Windows, hoặc Linux)
sudo apt update
sudo apt install -y g++ make libglfw3-dev libglm-dev libgl-dev python3 python3-pytest

# 3. Test phần Python
python3 -m pytest -q                      # mong đợi: 36 passed, 10 skipped

# 4. Build và chạy game (PHẢI đứng trong thư mục code/)
cd code
make
echo "R U U R U R U R D L" > /tmp/sol1.txt
./a.out 1 --play /tmp/sol1.txt            # khối tự lăn 10 bước rồi in: WON, Steps : 10
./a.out 1                                 # tự chơi bằng phím mũi tên
```

Gặp lỗi cài đặt → ghi lên Notion (task S0-02) để README được sửa cho cả nhóm.

---

## 3. Toàn bộ những thứ chưa có

| # | Hạng mục | Chi tiết | Người làm | Task |
|---:|---|---|---|---|
| 1 | **BFS** | `BFSSolver.solve()` | TV1 | S1-01 |
| 2 | **DFS** | `DFSSolver.solve()` (IDS là tùy chọn) | TV2 | S1-02, S2-05 |
| 3 | **UCS** | `UCSSolver.solve()` | TV3 | S1-03 |
| 4 | **A\*** | `AStarSolver.solve()` | TV4 | S1-04 |
| 5 | **Heuristic** | `manhattan_heuristic` + heuristic nâng cao + chứng minh | TV5 | S1-05, S2-08, S3-07 |
| 6 | **Hàm chi phí không đồng nhất** | File mới `ai/costs.py` + tùy chọn `--cost` cho `main.py` | TV3 | S1-06, S2-06 |
| 7 | **Định dạng màn chơi v2** | Mở rộng `.data`: một công tắc nhiều ô cầu, permanent, split | TV1 | S0-04, S1-10, S2-01 |
| 8 | **Permanent + nhiều ô cầu** | Cả C++ (`tile.cpp`, `game.cpp`) và Python (`problem.py`) | TV1 | S2-01, S2-02 |
| 9 | **Split switch** | Python (TV1) + C++ vẽ 2 cube, đổi cube (TV2) | TV1, TV2 | S2-03, S2-04 |
| 10 | **Nút GUI** | Restart, Solve BFS/DFS/UCS/A\*, panel số liệu (Dear ImGui) | TV2 | S1-07, S3-01 |
| 11 | **New Game / chọn màn** | Danh sách màn trong `code/data/` | TV4 | S3-09 |
| 12 | **Solver không làm treo cửa sổ** | Gọi solver trong luồng nền | TV2 | S3-03 |
| 13 | **≥ 10 màn chơi** | Hiện chỉ có 2 màn; cần chia nhóm G1–G6 | TV4 | S1-08, S2-09, S3-04 |
| 14 | **Tài liệu màn chơi** | File mới `code/data/README.md` | TV4 | S3-11 |
| 15 | **Số liệu cho GUI** | `main.py --stats-out stats.json` | TV3 | S3-02 |
| 16 | **Timeout / giới hạn node** | `BaseSolver.run(timeout_s=..., max_nodes=...)` | TV3 | S3-10 |
| 17 | **Test chéo thuật toán** | File mới `tests/test_cross.py` | TV3 | S2-07 |
| 18 | **So khớp luật C++ ↔ Python** | File mới `scripts/check_parity.py` | TV3 | S3-05 |
| 19 | **Thực nghiệm** | Thư mục mới `experiments/`: `run_all.py`, `plot.py` | TV5 | S1-09, S3-06, S4-04, S4-05 |
| 20 | **Báo cáo PDF** | Mỗi người viết phần của mình, TV5 ráp | Tất cả | S5-02 … S5-10 |
| 21 | **Video demo** | Kịch bản (TV4), quay + dựng (TV2) | TV2, TV4 | S4-08, S5-09 |
| 22 | (Tùy chọn) ML suy trọng số chi phí | Thầy cho phép (Q&A câu 10) | TV3 | S3-08 |

---

## 4. Những thứ mọi người cần biết trước khi code

### 4.1. Tọa độ — dễ nhầm nhất

- `x` = chỉ số **dòng** trong file `.data`, `y` = chỉ số **cột**.
- `R` → x+1, `L` → x−1, `U` → y+1, `D` → y−1. Trong cửa sổ 3D `R` là sang phải, nhưng **khi nhìn file text thì `R` là đi xuống một dòng**.
- Tư thế: `Z` đứng chiếm `(x,y)`; `X` nằm chiếm `(x,y),(x+1,y)`; `Y` nằm chiếm `(x,y),(x,y+1)`.
- Bảng chuyển đầy đủ: README mục "Mô hình trạng thái".

### 4.2. Các hàm có sẵn để dùng trong solver

```python
from ai import BloxorzProblem

problem = BloxorzProblem.from_file("code/data/1.data")

s0 = problem.initial_state()            # print(s0) -> (3,3,Z,mask=0)
problem.is_goal(s0)                     # False
problem.successors(s0)                  # [('U', (3,4,Y,mask=0), 1.0), ('R', (4,3,X,mask=0), 1.0)]
                                        #   -> chỉ gồm các nước KHÔNG làm thua, thứ tự U, D, L, R
problem.goal                            # (7, 7) — ô lỗ đích
s0.cells()                              # các ô khối đang chiếm, ví dụ ((3, 3),)

problem.is_valid_solution("R U U R U R U R D L")   # True — dùng để tự kiểm tra lời giải
problem.path_cost(["R", "U"])                      # tổng chi phí của một dãy nước đi
```

- `State` **bất biến và hashable** → dùng thẳng làm khóa `dict`/`set`. Hai `State` cùng vị trí nhưng khác `bridge_mask` là hai trạng thái khác nhau (đúng như đề yêu cầu).
- **Không** tự viết lại luật trong solver; chỉ gọi `successors()` và `is_goal()`.

### 4.3. Khung solver — mỗi người chỉ viết thân `solve()`

```python
class BFSSolver(BaseSolver):
    name = "bfs"

    def solve(self) -> SearchResult:
        # Có sẵn để dùng:
        #   self.problem                       bài toán
        #   self.nodes_expanded += 1           đếm node mở rộng (lấy ra khỏi frontier và sinh con)
        #   self.nodes_generated += 1          đếm node con sinh ra
        #   self.max_frontier = max(...)       kích thước frontier lớn nhất
        #   self.reconstruct_path(parent, s)   parent[s] = (state_cha, action), gốc -> None
        #   return self.make_result(path)      path = list ["U","R",...] hoặc None nếu vô nghiệm
        raise NotImplementedError
```

- **Không cần** tự đo thời gian/bộ nhớ: `run()` ở lớp cha làm sẵn.
- `AStarSolver` có thêm `self.heuristic(self.problem, state)`.
- Mỗi file solver đã có docstring gợi ý (cấu trúc dữ liệu, chỗ kiểm tra đích, cách đếm node). Đọc kỹ trước khi viết.

### 4.4. Cách tự kiểm tra

```bash
python3 -m pytest tests/test_solvers.py -k bfs -q        # chỉ test BFS (thay bằng dfs/ucs/astar)
python3 main.py code/data/2.data --algo bfs              # in số bước, node mở rộng, thời gian
python3 main.py code/data/1.data --compare --memory      # bảng so sánh cả 4 thuật toán
cd code && ./a.out 2 --solve bfs                         # xem khối tự lăn trong game
```

Đáp án đúng: **BFS, UCS, A\* cho đúng 10 bước (màn 1) và 68 bước (màn 2)**. DFS chỉ cần lời giải hợp lệ (thường dài hơn).

### 4.5. Ba quy tắc bắt buộc

1. **Thân thuật toán tự viết.** Thầy có thể vấn đáp ngẫu nhiên. Nếu có dùng AI để hỏi hoặc tham khảo, ghi lại công cụ và prompt vào trang Notion "Sử dụng AI" (phụ lục report).
2. **Luật game nằm ở hai nơi** (C++ để chơi, Python để giải). Ai sửa luật thì sửa **cả hai**, thêm test, rồi chạy `./a.out <n> --play <lời giải>` để chắc hai bên khớp nhau.
3. **Mỗi task một branch + một Pull Request**, có 1 người review; `pytest` phải pass trước khi merge. Không sửa file của người khác; góp ý qua review.

---

## 5. Việc của từng người

> Ký hiệu: 🆕 = file chưa tồn tại, cần tạo · ✏️ = file có sẵn, cần sửa. "Ngày N" tính từ ngày cả nhóm bắt đầu; hạn chi tiết ở `task.md`.

### TV1 — BFS & Luật game

**Vai trò:** viết BFS và là "chủ" của luật game. Mọi thay đổi luật đều qua TV1 (cả C++ lẫn Python).

| File | Việc | Hạn |
|---|---|---|
| ✏️ `ai/solvers/bfs_solver.py` | Thân `solve()` — frontier `deque`, `parent` vừa là tập đã thăm vừa để truy vết | Ngày 4 |
| ✏️ `Docs/implementation_plan.md` mục 4.2 | **Chốt cú pháp `.data` v2** (tương thích ngược), cả nhóm đồng ý | Ngày 2 |
| ✏️ `ai/level.py` | `Level.from_text` đọc phần v2; `validate()` cho các trường mới | Ngày 6 |
| ✏️ `code/tile.cpp`, `code/tile.h` | `Floor::initialize` đọc v2; một công tắc bật/tắt **nhiều** ô; chế độ permanent `on`/`off` | Ngày 8 |
| ✏️ `code/game.cpp` | `check_after_step` dùng cơ chế công tắc mới | Ngày 8 |
| ✏️ `ai/problem.py`, `ai/state.py` | Permanent + nhiều ô cầu; **split**: `State` thêm 2 cube, 8 hành động `(cube, hướng)`, tự gộp khi kề nhau | Ngày 8 / 10 |
| ✏️ `tests/test_problem.py` | Test cho từng cơ chế mới (giống phong cách test hiện có: tạo màn nhỏ bằng `make_problem`) | theo từng việc |
| ✏️ `README.md` | Cập nhật định dạng v2, nút GUI, cách chạy thực nghiệm | Ngày 17 |

**Xong khi:** test BFS pass (10 / 68 bước); một màn v2 có permanent và split chơi được trong game; solver giải màn đó và `./a.out <n> --play` cho `WON`.
**Lưu ý:** luật cube (cube trên ô cam, trên lỗ đích, trên công tắc nặng, khi nào gộp) đã có đề xuất ở `report0.md` mục 9.1 (#5–#11). Chốt với nhóm rồi viết vào `implementation_plan.md` trước khi code. Phần vẽ split trong C++ do TV2 làm, TV1 cung cấp logic.

### TV2 — DFS & GUI

**Vai trò:** viết DFS và phụ trách mọi thứ người dùng nhìn thấy trong cửa sổ game; quay video.

| File | Việc | Hạn |
|---|---|---|
| ✏️ `ai/solvers/dfs_solver.py` | Thân `solve()` — stack tường minh (không đệ quy), tập đã thăm, thứ tự hành động cố định | Ngày 4 |
| 🆕 `code/imgui/` | Chép mã nguồn Dear ImGui (license MIT) + backend `imgui_impl_glfw`, `imgui_impl_opengl3` | Ngày 6 |
| 🆕 `code/ui.h`, `code/ui.cpp` | Gom toàn bộ giao diện ImGui vào đây (để TV4 thêm phần chọn màn mà không đụng `main.cpp`) | Ngày 6 |
| ✏️ `code/makefile`, `THIRD_PARTY_NOTICES.md` | Biên dịch ImGui; ghi nguồn ImGui | Ngày 6 |
| ✏️ `code/main.cpp` | Gọi `ui` mỗi frame; nút **Restart** (gọi lại `game.initialize`, reset `game_result`, `autoplayer`) | Ngày 6 |
| ✏️ `code/ui.cpp`, `code/autoplay.cpp` | Nút **Solve BFS/DFS/UCS/A\***; panel số liệu (đọc file từ `--stats-out` của TV3); gọi `run_solver` trong **`std::thread`** để cửa sổ không treo | Ngày 13–14 |
| ✏️ `code/superblock.*`, `code/controls.cpp` | **Split phía C++:** vẽ 2 khối 1×1×1, phím đổi cube, tô sáng cube đang điều khiển, tự gộp — theo đúng logic TV1 chốt | Ngày 10 |
| 🆕 `ai/solvers/ids_solver.py` (tùy chọn) | IDS + đăng ký trong `ai/solvers/__init__.py` (`SOLVERS["ids"]`) | Ngày 10 |

**Xong khi:** test DFS pass; trong game bấm được 4 nút Solve, khối tự lăn tới đích, panel hiện số liệu; Restart hoạt động; màn có split chơi được.
**Lưu ý:**
- Phím **Space** hiện đang dùng để đổi phép chiếu (`controls.cpp`). Nếu dùng Space để đổi cube thì chuyển phép chiếu sang phím khác và cập nhật README.
- `main.cpp` hiện chạy solver **trước** khi mở cửa sổ (`--solve`). Khi có nút Solve thì cần chạy trong cửa sổ, ở luồng nền.
- Video (S5-09): phủ đủ 5đ GUI + 5đ chức năng theo kịch bản của TV4; YouTube để **Public**.

### TV3 — UCS & Hàm chi phí & Đo đạc

**Vai trò:** viết UCS, thiết kế hàm chi phí (phần "ăn điểm" của UCS), và lo các công cụ đo/kiểm thử chung.

| File | Việc | Hạn |
|---|---|---|
| ✏️ `ai/solvers/ucs_solver.py` | Thân `solve()` — `heapq` với `(g, counter, state)`, lazy deletion, **kiểm tra đích khi lấy ra** | Ngày 5 |
| 🆕 `ai/costs.py` | Hàm chi phí `cost_fn(state, action, next_state) -> float` (> 0), ví dụ theo năng lượng lăn (report0 mục 5.3); dict `COSTS = {"unit": ..., "energy": ...}` | Ngày 6 → chốt Ngày 8 |
| ✏️ `main.py` | Thêm `--cost unit|energy` (truyền vào `BloxorzProblem(level, cost_fn=...)`) và `--stats-out file.json` | Ngày 12 |
| ✏️ `ai/solvers/base_solver.py` | `run(timeout_s=None, max_nodes=None)`; hết giờ → `status = "timeout"` | Ngày 12 |
| 🆕 `tests/test_cross.py` | Test chéo: chi phí A\* = chi phí UCS (cùng hàm chi phí); độ dài A\* (chi phí 1) = BFS — trên **mọi** màn | Ngày 10 |
| 🆕 `scripts/check_parity.py` | Với mỗi màn × solver: ghi lời giải → chạy `./a.out <n> --play` → phải `WON` đúng số bước | Ngày 14 |
| (tùy chọn) | ML suy trọng số chi phí (Q&A câu 10): mô tả dữ liệu, mô hình, kết quả | Ngày 14 |

**Xong khi:** test UCS pass; `python main.py code/data/2.data --algo ucs --cost energy` chạy được; `test_cross.py` pass; `check_parity.py` báo khớp trên mọi màn.
**Lưu ý:** chi phí phải **> 0**. Báo giá trị **chi phí nhỏ nhất của một nước** cho TV5, vì heuristic phải nhân với số này để vẫn admissible khi chi phí không đồng nhất.

### TV4 — A\* & Màn chơi

**Vai trò:** viết A\*, thiết kế bộ màn chơi (thầy yêu cầu **≥ 10 test case**), viết kịch bản video.

| File | Việc | Hạn |
|---|---|---|
| ✏️ `ai/solvers/astar_solver.py` | Thân `solve()` — như UCS nhưng khóa heap là `f = g + h`; thử trước với `--heuristic zero` | Ngày 5 |
| 🆕 `code/data/3.data`, `4.data`, … | Màn mới, đánh số liên tiếp (game đọc `data/<số>.data`). G1–G2 trước (Ngày 6), G3 (Ngày 10), G4–G6 (Ngày 13) | Ngày 6 → 13 |
| ✏️ `tests/helpers.py` | Thêm lời giải tối ưu của màn mới vào `OPTIMAL_LENGTH` (và test cho màn đó) | theo từng màn |
| 🆕 `code/data/README.md` | Mỗi màn: nhóm (G1–G6), kích thước, số ô, công tắc, độ dài tối ưu, ảnh chụp | Ngày 14 |
| ✏️ `code/ui.cpp` (sau khi TV2 tạo) | **New Game / chọn màn**: liệt kê `data/*.data`, chọn để chơi hoặc giải | Ngày 13 |
| Kịch bản video | Theo report0 mục 8.4; chọn màn demo (thua vì ô cam, công tắc, split, chạy 4 solver) | Ngày 18 |

**Xong khi:** test A\* pass (10 / 68 bước); có **≥ 10 màn** qua `Level.validate()`, chơi tay thắng được, có lời giải tối ưu ghi lại.
**Cách làm màn nhanh:**
1. Vẽ lưới trên giấy hoặc Excel. Nhớ `x` = dòng, `y` = cột.
2. Viết file `.data`: chừa **viền 2 ô 0** quanh map; ô đích và ô cầu phải là **0** trên lưới.
3. `python3 main.py code/data/<n>.data --algo bfs` để lấy độ dài tối ưu (sau khi TV1 xong BFS).
4. `cd code && ./a.out <n>` để chơi thử.

Bộ màn cần có: màn **lớn** (≥ 15×20, nhiều công tắc) để thấy khác biệt giữa các thuật toán, và **1 màn vô nghiệm**.

### TV5 — Heuristic, Thực nghiệm & Quản lý

**Vai trò:** heuristic + chứng minh, toàn bộ phần thực nghiệm (chiếm **20 điểm** report), quản lý Notion, ráp report, nộp bài.

| File | Việc | Hạn |
|---|---|---|
| ✏️ `ai/heuristics.py` | `manhattan_heuristic` theo gợi ý trong file (min khoảng cách, ⌈d/2⌉, `max(·, 1)`, đích = 0) | Ngày 4 |
| ✏️ `ai/heuristics.py` | Heuristic nâng cao: khoảng cách trên đồ thị ô (coi mọi cầu mở) + cạnh "cổng" cho split; bản nhân `c_min` cho chi phí không đồng nhất; đăng ký trong `HEURISTICS` | Ngày 10 |
| Chứng minh | Admissible & consistent theo hàm chi phí đã chốt (report0 mục 5.4 có sẵn hướng chứng minh + phản ví dụ công thức "+1") | Ngày 14 |
| 🆕 `experiments/run_all.py` | Chạy mọi solver × màn (× heuristic, × hàm chi phí); lặp 5 lần lấy trung vị thời gian; đo bộ nhớ ở lượt riêng; ghi CSV | Ngày 6 → 14 |
| 🆕 `experiments/plot.py` | Biểu đồ: node mở rộng (trục log), thời gian, bộ nhớ, độ dài; bảng tổng hợp | Ngày 18 |
| ✏️ `requirements.txt` | Thêm `matplotlib` (chỉ dùng để vẽ biểu đồ) | Ngày 18 |
| Báo cáo | Dựng khung (Ngày 9), viết heuristic + Experiments + Planning + References + Phụ lục AI, ráp `Report.pdf`, đóng gói, nộp | Ngày 19–21 |

**Xong khi:** A\* dùng heuristic của TV5 cho đúng 10 / 68 bước và mở rộng **ít** node hơn UCS; có CSV + biểu đồ cuối cùng; nộp bài đúng quy cách.
**Lưu ý:**
- Thư mục `results/` đang bị `.gitignore`. Bản số liệu **cuối cùng** dùng cho báo cáo nên lưu vào `experiments/results_final.csv` (có commit).
- Ghi cấu hình máy (CPU, RAM, OS, phiên bản Python) vào báo cáo.

---

## 6. Bản đồ file

| File / thư mục | Trạng thái | Người phụ trách |
|---|---|---|
| `ai/level.py`, `ai/state.py`, `ai/problem.py` | ✅ có sẵn — sẽ mở rộng cho v2/split | TV1 |
| `ai/solvers/base_solver.py` | ✅ có sẵn — thêm timeout | TV3 |
| `ai/solvers/bfs_solver.py` | ⏳ TODO | TV1 |
| `ai/solvers/dfs_solver.py` | ⏳ TODO | TV2 |
| `ai/solvers/ucs_solver.py` | ⏳ TODO | TV3 |
| `ai/solvers/astar_solver.py` | ⏳ TODO | TV4 |
| `ai/heuristics.py` | ⏳ TODO (`manhattan_heuristic`) | TV5 |
| `ai/costs.py` | 🆕 chưa có | TV3 |
| `ai/solvers/ids_solver.py` | 🆕 tùy chọn | TV2 |
| `main.py` | ✅ có sẵn — thêm `--cost`, `--stats-out` | TV3 |
| `tests/test_problem.py`, `tests/helpers.py` | ✅ có sẵn — thêm test luật mới / màn mới | TV1 / TV4 |
| `tests/test_solvers.py` | ✅ có sẵn — **không cần sửa**, tự hết SKIP khi solver được cài | — |
| `tests/test_cross.py` | 🆕 chưa có | TV3 |
| `scripts/check_parity.py` | 🆕 chưa có | TV3 |
| `experiments/run_all.py`, `experiments/plot.py` | 🆕 chưa có | TV5 |
| `code/autoplay.*`, `code/main.cpp` | ✅ có sẵn — thêm luồng nền, Restart | TV2 |
| `code/ui.h`, `code/ui.cpp`, `code/imgui/` | 🆕 chưa có | TV2 (+ TV4 phần chọn màn) |
| `code/tile.*`, `code/game.cpp` | ✅ có sẵn — mở rộng v2/permanent | TV1 |
| `code/superblock.*`, `code/controls.cpp` | ✅ có sẵn — thêm split (vẽ, phím) | TV2 |
| `code/data/1.data`, `2.data` | ✅ 2 màn của game gốc | — |
| `code/data/3.data` … (≥ 10 màn), `code/data/README.md` | 🆕 chưa có | TV4 |
| `README.md`, `THIRD_PARTY_NOTICES.md` | ✅ có sẵn — cập nhật khi có tính năng mới | TV1 / TV2 |
| `Docs/report0.md`, `task.md`, `implementation_plan.md`, `report1.md` | ✅ tài liệu nhóm | TV5 cập nhật |

---

## 7. Thứ tự phụ thuộc — ai đang chờ ai

```
Ngày 1–2   Tất cả: cài môi trường, chạy được game + pytest
           TV1: chốt định dạng .data v2  ──────────────┐
Ngày 2–6   TV1 BFS · TV2 DFS · TV3 UCS · TV4 A* · TV5 heuristic   (làm song song, không chờ nhau)
           TV2: ImGui + ui.cpp  ────────────────────────┼──► TV4 thêm chọn màn (Ngày 10–13)
           TV4: cần BFS của TV1 để đo độ dài tối ưu của màn mới
Ngày 6–10  TV1: v2 + permanent + split (Python)  ──► TV2: split phía C++ ──► TV4: màn G4 có split
           TV3: chốt hàm chi phí ──► TV5: heuristic cho chi phí không đồng nhất
Ngày 10–14 TV2: nút Solve + luồng nền · TV3: --stats-out (cho panel của TV2), check_parity
           TV4: đủ ≥ 10 màn ──► TV5: chạy thực nghiệm chính thức (Ngày 14–18)
Ngày 18–21 Viết báo cáo, quay video, nộp
```

**Chuỗi dễ trễ nhất:** TV1 (định dạng v2) → TV2 (split C++) → TV4 (màn có split) → TV5 (thực nghiệm). Ai trong chuỗi này bị kẹt quá 1 ngày thì báo ngay trên nhóm.

---

## 8. Câu hỏi thường gặp

**Tôi có được sửa `ai/problem.py` không?** Chỉ TV1 sửa (hoặc qua PR có TV1 review). Solver chỉ **gọi** `successors()` / `is_goal()`.

**Solver của tôi chạy đúng trong Python nhưng game lăn sai / rơi?** Báo TV1 ngay: đó là dấu hiệu luật C++ và Python lệch nhau.

**Tại sao `pytest` báo SKIP?** Vì solver chưa được cài (`NotImplementedError`). Cài xong là test tự chạy.

**`./a.out` báo `Cannot open data/1.data`?** Bạn chưa đứng trong thư mục `code/`.

**Thấy cảnh báo `MESA` / `libEGL` khi mở game trên WSL?** Vô hại.

**Có được dùng AI không?** Được (thầy cho phép), nhưng phải hiểu từng dòng mình nộp và ghi lại cách dùng vào trang Notion "Sử dụng AI".
