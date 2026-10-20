# Danh sách task — Project 1: Bloxorz Solver (nhóm 5 thành viên)

> Bản cập nhật **20/10/2026** theo hướng đã chốt: game 3D C++/OpenGL của `chaitanya100100/Bloxorz` @ `c0ccb55` (thầy cho phép, có cite) + phần AI Python trong `ai/`. Xem `Docs/report0.md` (mục 1.1, 2.6, 6, 8) và `Docs/implementation_plan.md`.
> Thời gian tính theo **Ngày 1 → Ngày 21** (Ngày 1 = ngày cả nhóm bắt đầu). Khi đưa vào Notion, đổi "Ngày N" thành ngày thật.

---

## 1. Hướng dẫn đưa vào Notion

1. Copy **bảng tổng hợp ở mục 5** → dán vào một trang Notion → bấm `⋯` của bảng → **Turn into database**.
2. Mở từng task, dán phần **mô tả + tiêu chí hoàn thành** ở mục 6 (dòng `- [ ]` thành to-do).

| Property | Kiểu | Giá trị |
|---|---|---|
| `ID` | Text | `S1-01`, … |
| `Task` | Title | Tên task |
| `Sprint` | Select | Sprint 0 … Sprint 5 |
| `Người phụ trách` | Person | TV1 … TV5 hoặc "Tất cả" |
| `Ưu tiên` | Select | **P0** bắt buộc · **P1** cần có · **P2** mở rộng |
| `Ước lượng (h)` | Number | Giờ ước tính |
| `Hạn` | Date | Ngày N → ngày thật |
| `Phụ thuộc` | Relation (chính database) | Task phải xong trước |
| `Hạng mục điểm` | Multi-select | Game-Core · Game-Adv · BFS · DFS · UCS · A* · Report-Algo · Report-Exp · Report-Plan · Video · Nộp bài · Quản lý |
| `Trạng thái` | Status | Not started · In progress · In review · Done |

View gợi ý: Board theo `Trạng thái`, Board theo `Người phụ trách`, Timeline theo `Hạn`, Filter `Ưu tiên = P0`.

---

## 2. Phân vai 5 thành viên

Mỗi người phụ trách **một file solver**; TV5 phụ trách **heuristic và báo cáo so sánh**. Ngoài ra mỗi người nhận một mảng game/báo cáo.

| Mã | Vai trò | Solver / file chính | Mảng phụ trách thêm | Phần report | Video |
|---|---|---|---|---|---|
| **TV1** | BFS & Luật game | `ai/solvers/bfs_solver.py` | Mở rộng luật ở **cả C++ và Python**: `.data` v2, nhiều ô cầu, permanent, split (Python) | Luật chơi + Hình thức hóa + BFS | — |
| **TV2** | DFS & GUI | `ai/solvers/dfs_solver.py` | GUI 3D: Dear ImGui (nút, panel), split phía C++ (vẽ cube, phím Space) | DFS + Cài đặt & GUI | **Quay, dựng, upload** |
| **TV3** | UCS & Chi phí | `ai/solvers/ucs_solver.py` | Hàm chi phí không đồng nhất, đo đạc, test so khớp C++ ↔ Python | UCS + hàm chi phí | — |
| **TV4** | A\* & Màn chơi | `ai/solvers/astar_solver.py` | Thiết kế **≥ 10 màn**, chọn màn trong GUI, kịch bản video | A\* + mô tả bộ test | Kịch bản |
| **TV5** | Heuristic, Thực nghiệm & PM | `ai/heuristics.py` | Chứng minh admissible/consistent, `experiments/`, Notion, ráp report, nộp bài | Heuristic + Experiments + Planning + References + Phụ lục AI | — |

> **Quy tắc chung:** mọi thành viên phải hiểu **toàn bộ** hệ thống (luật, mô hình trạng thái, 4 thuật toán, heuristic, hàm chi phí) vì thầy có thể vấn đáp bất kỳ ai. **Thân thuật toán do chính người phụ trách tự viết.**

---

## 3. Đã hoàn thành — khung repo (trước Sprint 1)

Khung được dựng với sự hỗ trợ của Claude Code (ghi vào phụ lục "Sử dụng AI" của report). Các thành viên **review** phần này ở S0-02 và S0-03.

| ID | Nội dung | Kết quả kiểm chứng |
|---|---|---|
| D-01 | Import code gốc chaitanya @ `c0ccb55` thành commit mốc; `.gitignore`, `.gitattributes` (LF) | `git diff ffa730b..HEAD -- code/` cho thấy phần nhóm sửa |
| D-02 | `ai/`: `Level`, `State`, `BloxorzProblem` (chép đúng luật C++), `BaseSolver`, `SearchResult`; 4 solver + heuristic để TODO | — |
| D-03 | `tests/`: 36 test luật/khung **PASS**, 10 test solver **SKIP** (chờ cài) | `python -m pytest -q` |
| D-04 | `main.py` CLI (`--algo`, `--out`, `--compare`, `--memory`) | Solver chưa cài → báo "chưa cài", mã 2 |
| D-05 | Game: `autoplay`, `./a.out [level] [--solve algo | --play file]`, giữ màn hình 1.5 s, khóa phím khi phát lại, kiểm tra biên mảng | `./a.out 1 --play` → `WON, Steps : 10`; màn 2 → 68 |
| D-06 | GLAD (OpenGL 3.3 core) + `makefile` Linux/macOS/MSYS2; build trên WSL Ubuntu | `make` thành công |
| D-07 | `README.md`, `THIRD_PARTY_NOTICES.md`, `Docs/implementation_plan.md`, cập nhật `report0.md`, `task.md` | — |

---

## 4. Tải công việc dự kiến

| Thành viên | Tổng (h) | Trong đó P2 (h) | Task riêng (h) | Task "Tất cả" (h) |
|---|---:|---:|---:|---:|
| TV1 – BFS & Luật | 54 | 0 | 40 | 14 |
| TV2 – DFS & GUI | 56 | 3 | 42 | 14 |
| TV3 – UCS & Chi phí | 51 | 5 | 37 | 14 |
| TV4 – A\* & Màn chơi | 48 | 0 | 34 | 14 |
| TV5 – Heuristic, Exp & PM | 56 | 0 | 42 | 14 |
| **Tổng** | **265** | **8** | 195 | 70 |

Task "Tất cả" (14 h/người): đọc đề & tài liệu (2), cài môi trường (2), 4 buổi sprint review (4), bug bash (2), họp % đóng góp (1), đọc chéo report (1), luyện vấn đáp (2).

---

## 5. Bảng tổng hợp task (copy vào Notion)

| ID | Task | Sprint | Người phụ trách | Ưu tiên | Ước lượng (h) | Hạn | Phụ thuộc | Hạng mục điểm |
|---|---|---|---|---|---:|---|---|---|
| S0-01 | Đọc đề, report0 (1.1, 2.6, 3.7, 6), README; chơi thử game Coolmath và game của nhóm | Sprint 0 | Tất cả | P0 | 2 | Ngày 1 | — | Quản lý |
| S0-02 | Cài môi trường, build game, chạy `./a.out 1 --play` và `pytest` trên máy mình | Sprint 0 | Tất cả | P0 | 2 | Ngày 2 | S0-01 | Game-Core |
| S0-03 | Đọc chéo `ai/problem.py` với `code/game.cpp`, `superblock.cpp` (xác nhận luật) | Sprint 0 | TV1 | P0 | 2 | Ngày 2 | S0-01 | Game-Core, Report-Algo |
| S0-04 | Chốt cú pháp `.data` v2 (nhiều ô cầu, permanent, split) vào implementation_plan | Sprint 0 | TV1 | P0 | 3 | Ngày 2 | S0-03 | Game-Adv |
| S0-05 | Dựng Notion board, lịch họp, kênh liên lạc | Sprint 0 | TV5 | P1 | 1 | Ngày 1 | — | Quản lý |
| S0-06 | Ôn UCS/A*/admissible/consistent, phác thảo hàm chi phí | Sprint 0 | TV3 | P1 | 2 | Ngày 2 | — | UCS, A* |
| S1-01 | Cài `BFSSolver` | Sprint 1 | TV1 | P0 | 3 | Ngày 4 | S0-02 | BFS |
| S1-02 | Cài `DFSSolver` (stack, visited, thứ tự cố định) | Sprint 1 | TV2 | P0 | 3 | Ngày 4 | S0-02 | DFS |
| S1-03 | Cài `UCSSolver` (heap + counter, lazy deletion, goal khi pop) | Sprint 1 | TV3 | P0 | 4 | Ngày 5 | S0-02 | UCS |
| S1-04 | Cài `AStarSolver` (thử trước với `zero_heuristic`) | Sprint 1 | TV4 | P0 | 4 | Ngày 5 | S0-02 | A* |
| S1-05 | Cài `manhattan_heuristic` + bản nháp chứng minh admissible | Sprint 1 | TV5 | P0 | 3 | Ngày 4 | S0-02 | A* |
| S1-06 | Hàm chi phí không đồng nhất (bản nháp), cắm qua `cost_fn` | Sprint 1 | TV3 | P0 | 3 | Ngày 6 | S0-06 | UCS |
| S1-07 | Prototype Dear ImGui trong game: build được + nút Restart | Sprint 1 | TV2 | P0 | 5 | Ngày 6 | S0-02 | Game-Core, Video |
| S1-08 | Màn G1/G2 (3–4 màn) dạng `.data`, kiểm tra bằng `validate` + BFS | Sprint 1 | TV4 | P0 | 4 | Ngày 6 | S1-01 | Report-Exp |
| S1-09 | Khung `experiments/run_all.py` (mọi solver × màn → CSV) | Sprint 1 | TV5 | P1 | 3 | Ngày 6 | S1-01 | Report-Exp |
| S1-10 | Python `Level` đọc `.data` v2 + test | Sprint 1 | TV1 | P0 | 5 | Ngày 6 | S0-04 | Game-Adv |
| S1-11 | Sprint 1 review: `./a.out 2 --solve` với 4 thuật toán, `pytest` hết SKIP | Sprint 1 | Tất cả | P0 | 1 | Ngày 6 | S1-01…S1-05 | Quản lý |
| S2-01 | C++ đọc `.data` v2: nhiều ô cầu/công tắc, permanent on/off | Sprint 2 | TV1 | P0 | 5 | Ngày 8 | S1-10 | Game-Adv |
| S2-02 | Python: permanent + nhiều ô cầu trong `problem.py` + test | Sprint 2 | TV1 | P0 | 3 | Ngày 8 | S1-10 | Game-Adv |
| S2-03 | Python: split (State 2 cube, 8 hành động, gộp, luật cube) + test | Sprint 2 | TV1 | P0 | 6 | Ngày 10 | S2-02 | Game-Adv |
| S2-04 | C++: split (tách, vẽ 2 cube, phím Space, gộp) khớp Python | Sprint 2 | TV2 | P0 | 8 | Ngày 10 | S2-01 | Game-Adv, Video |
| S2-05 | (Mở rộng) Cài IDS để so sánh với DFS | Sprint 2 | TV2 | P2 | 3 | Ngày 10 | S1-02 | DFS |
| S2-06 | Chốt hàm chi phí cuối + lý giải | Sprint 2 | TV3 | P0 | 3 | Ngày 8 | S1-06 | UCS, Report-Algo |
| S2-07 | Test chéo: cost A* = UCS, độ dài A*(unit) = BFS trên mọi màn | Sprint 2 | TV3 | P0 | 3 | Ngày 10 | S1-03, S1-04, S1-05 | A*, UCS |
| S2-08 | Heuristic có tính split (cổng dịch chuyển) + bản cho chi phí không đồng nhất | Sprint 2 | TV5 | P0 | 5 | Ngày 10 | S2-03 | A* |
| S2-09 | Màn G3 (công tắc/cầu, có permanent) | Sprint 2 | TV4 | P0 | 4 | Ngày 10 | S2-01 | Report-Exp |
| S2-10 | Dựng khung report (Overleaf/Word) theo dàn ý 8.3 | Sprint 2 | TV5 | P1 | 2 | Ngày 9 | — | Report-Plan |
| S2-11 | Sprint 2 review: chơi tay + giải màn có split/permanent | Sprint 2 | Tất cả | P0 | 1 | Ngày 10 | S2-01…S2-10 | Quản lý |
| S3-01 | GUI: nút Restart, Solve BFS/DFS/UCS/A*, panel số liệu | Sprint 3 | TV2 | P0 | 6 | Ngày 13 | S1-07 | Game-Core, Video |
| S3-02 | `main.py --stats-out`: ghi số liệu (JSON) cho panel GUI | Sprint 3 | TV3 | P1 | 2 | Ngày 12 | — | Video |
| S3-03 | Chạy solver ở luồng nền, cửa sổ không treo | Sprint 3 | TV2 | P0 | 3 | Ngày 14 | S3-01 | Video |
| S3-04 | Màn G4 (split), G5 (lớn/khó), G6 (vô nghiệm, open field) → tổng ≥ 10 | Sprint 3 | TV4 | P0 | 6 | Ngày 13 | S2-04, S2-09 | Report-Exp |
| S3-05 | Test so khớp luật C++ ↔ Python trên mọi màn (phát lại bằng `--play`) | Sprint 3 | TV3 | P0 | 4 | Ngày 14 | S3-04 | Game-Core, Game-Adv |
| S3-06 | `run_all.py` hoàn chỉnh: lặp, median, memory tách lượt, timeout | Sprint 3 | TV5 | P0 | 5 | Ngày 14 | S1-09 | Report-Exp |
| S3-07 | Chứng minh admissible & consistent hoàn chỉnh | Sprint 3 | TV5 | P0 | 3 | Ngày 14 | S2-08 | A*, Report-Algo |
| S3-08 | (Mở rộng) Suy trọng số chi phí bằng ML (Q&A câu 10) | Sprint 3 | TV3 | P2 | 5 | Ngày 14 | S2-06 | UCS |
| S3-09 | GUI: New Game / chọn màn từ `data/` | Sprint 3 | TV4 | P0 | 4 | Ngày 13 | S1-07 | Game-Core |
| S3-10 | Thêm timeout / giới hạn node vào `BaseSolver.run` | Sprint 3 | TV3 | P1 | 3 | Ngày 12 | S1-03 | Report-Exp |
| S3-11 | Ghi lời giải tối ưu + ảnh minh họa từng màn (`code/data/README.md`) | Sprint 3 | TV4 | P1 | 3 | Ngày 14 | S3-04 | Report-Exp |
| S3-12 | Sprint 3 review: bấm 4 nút Solve trên mọi màn | Sprint 3 | Tất cả | P0 | 1 | Ngày 14 | S3-01…S3-11 | Quản lý |
| S4-01 | Bug bash: chơi chéo màn, ghi bug lên Notion | Sprint 4 | Tất cả | P0 | 2 | Ngày 15 | S3-12 | Game-Core, Game-Adv |
| S4-02 | Sửa bug luật (C++ và Python) + test hồi quy | Sprint 4 | TV1 | P0 | 4 | Ngày 16 | S4-01 | Game-Core, Game-Adv |
| S4-03 | Sửa bug GUI + đánh bóng (chú thích loại ô, màu, HUD) | Sprint 4 | TV2 | P1 | 4 | Ngày 17 | S4-01 | Video |
| S4-04 | Chạy thực nghiệm chính thức, ghi cấu hình máy | Sprint 4 | TV5 | P0 | 3 | Ngày 17 | S4-02, S3-06 | Report-Exp |
| S4-05 | `plot.py`: biểu đồ (trục log) + bảng | Sprint 4 | TV5 | P0 | 4 | Ngày 18 | S4-04 | Report-Exp |
| S4-06 | Phân tích UCS vs BFS (độ dài vs chi phí) | Sprint 4 | TV3 | P0 | 3 | Ngày 18 | S4-04 | Report-Exp |
| S4-07 | Phân tích A* với các heuristic (h = 0, Manhattan, nâng cao) | Sprint 4 | TV4 | P0 | 3 | Ngày 18 | S4-04 | Report-Exp |
| S4-08 | Kịch bản video + chuẩn bị màn demo | Sprint 4 | TV4 | P0 | 2 | Ngày 18 | S4-03 | Video |
| S4-09 | Cập nhật README (định dạng v2, nút GUI, thực nghiệm) | Sprint 4 | TV1 | P0 | 2 | Ngày 17 | S4-02 | Nộp bài |
| S4-10 | Sprint 4 review + code freeze | Sprint 4 | Tất cả | P0 | 1 | Ngày 18 | S4-01…S4-09 | Quản lý |
| S5-01 | Họp chốt % đóng góp | Sprint 5 | Tất cả | P0 | 1 | Ngày 19 | S4-10 | Report-Plan |
| S5-02 | Report: luật chơi + hình thức hóa bài toán | Sprint 5 | TV1 | P0 | 5 | Ngày 19 | S4-10 | Report-Algo |
| S5-03 | Report: BFS | Sprint 5 | TV1 | P0 | 2 | Ngày 19 | S1-01 | Report-Algo |
| S5-04 | Report: DFS (/IDS) + Cài đặt & GUI | Sprint 5 | TV2 | P0 | 4 | Ngày 19 | S4-03 | Report-Algo |
| S5-05 | Report: UCS + hàm chi phí | Sprint 5 | TV3 | P0 | 4 | Ngày 19 | S2-06 | Report-Algo |
| S5-06 | Report: A* + mô tả bộ test | Sprint 5 | TV4 | P0 | 4 | Ngày 19 | S3-04 | Report-Algo, Report-Exp |
| S5-07 | Report: heuristic + chương Experiments | Sprint 5 | TV5 | P0 | 5 | Ngày 19 | S4-05 | Report-Exp |
| S5-08 | Report: Planning, References (cite chaitanya), Phụ lục dùng AI | Sprint 5 | TV5 | P0 | 3 | Ngày 19 | S5-01 | Report-Plan |
| S5-09 | Quay + dựng video, phụ đề, upload YouTube (Public) | Sprint 5 | TV2 | P0 | 6 | Ngày 20 | S4-08 | Video |
| S5-10 | Ráp report, xuất `Report.pdf` (chèn link video) | Sprint 5 | TV5 | P0 | 4 | Ngày 20 | S5-02…S5-08 | Report-Plan |
| S5-11 | Đọc chéo report | Sprint 5 | Tất cả | P0 | 1 | Ngày 20 | S5-10 | Report-Plan |
| S5-12 | Kiểm tra cài đặt sạch trên máy khác (clone → apt → make → pytest) | Sprint 5 | TV3 | P0 | 1 | Ngày 20 | S4-09 | Nộp bài |
| S5-13 | Luyện vấn đáp | Sprint 5 | Tất cả | P0 | 2 | Ngày 21 | S5-10 | Quản lý |
| S5-14 | Đóng gói (`Source/` = gốc repo, bỏ `demo/`) và nộp | Sprint 5 | TV5 | P0 | 1 | Ngày 21 | S5-09, S5-11, S5-12 | Nộp bài |

---

## 6. Chi tiết từng task

> Task code chỉ được tính **Done** khi: merge qua Pull Request có **1 người khác review**, `python -m pytest -q` pass, và nếu đụng luật thì đã chạy `./a.out <n> --play <lời giải>` để so khớp C++ ↔ Python.

### Sprint 0 — Khởi động (Ngày 1–2)

#### S0-01 · Đọc tài liệu + chơi thử — *Tất cả · P0 · 2h*
- [ ] Đọc `AI - Project 1.pdf`, `report0.md` mục 1.1, 2.6, 3.7, 6 và `README.md`
- [ ] Chơi thử Bloxorz trên Coolmath (màn có fragile, switch, split) và game của nhóm (`./a.out 1`, `./a.out 2`)
- [ ] Ghi câu hỏi vào trang "Câu hỏi" trên Notion

#### S0-02 · Cài môi trường — *Tất cả · P0 · 2h*
- [ ] Làm theo README: WSL Ubuntu + `apt install g++ make libglfw3-dev libglm-dev libgl-dev python3-pytest`
- [ ] `cd code && make && ./a.out 1 --play <file lời giải màn 1>` → in `WON`
- [ ] `python -m pytest -q` → 36 passed, 10 skipped
- [ ] Báo lỗi cài đặt (nếu có) lên Notion để README được sửa

#### S0-03 · Đọc chéo luật Python ↔ C++ — *TV1 · P0 · 2h*
- [ ] Đối chiếu `BloxorzProblem.check/simulate` với `Game::check`, `Game::check_after_step`, `SuperBlock::update`
- [ ] Xác nhận (hoặc báo sai lệch) cho từng dòng của bảng chuyển trong report0 mục 2.6

#### S0-04 · Chốt cú pháp `.data` v2 — *TV1 · P0 · 3h*
- [ ] Tương thích ngược: file v1 hiện tại vẫn đọc được
- [ ] Mô tả: công tắc nhiều ô cầu, chế độ `toggle | on | off`, split switch (vị trí + 2 ô dịch chuyển)
- [ ] Ghi vào `Docs/implementation_plan.md` (Phase 2) + 1 file ví dụ; cả nhóm đồng ý

#### S0-05 · Notion — *TV5 · P1 · 1h*
- [ ] Database task theo mục 1, có đủ property và 4 view; lịch họp; kênh liên lạc

#### S0-06 · Ôn lý thuyết + phác thảo chi phí — *TV3 · P1 · 2h*
- [ ] Ghi chú 1 trang về điều kiện tối ưu UCS/A*; đề xuất sơ bộ hàm chi phí (report0 mục 5.3)

### Sprint 1 — Bốn solver + heuristic cơ bản (Ngày 2–6)

#### S1-01 / S1-02 / S1-03 / S1-04 · Cài BFS / DFS / UCS / A\* — *TV1 / TV2 / TV3 / TV4 · P0*
- [ ] Chỉ sửa thân `solve()` trong file của mình; làm theo docstring/gợi ý có sẵn
- [ ] Cập nhật `nodes_expanded`, `nodes_generated`, `max_frontier`; trả về `self.make_result(path)`
- [ ] `python -m pytest tests/test_solvers.py -q` → test của thuật toán mình không còn SKIP và PASS
- [ ] `python main.py code/data/2.data --algo <algo>` và `cd code && ./a.out 2 --solve <algo>` → `WON`
- [ ] BFS/UCS/A\*: đúng **10** bước (màn 1) và **68** bước (màn 2); DFS: lời giải hợp lệ

#### S1-05 · `manhattan_heuristic` — *TV5 · P0 · 3h*
- [ ] Cài theo gợi ý trong `ai/heuristics.py` (min khoảng cách, ⌈d/2⌉, max với 1, h(goal) = 0)
- [ ] Bản nháp chứng minh admissible + consistent (chi phí 1); test: A\* = BFS về độ dài trên màn 1, 2

#### S1-06 · Hàm chi phí bản nháp — *TV3 · P0 · 3h*
- [ ] Hàm `cost_fn(state, action, next_state)` > 0 trong một module mới (ví dụ `ai/costs.py`)
- [ ] Thêm tùy chọn chọn hàm chi phí cho `main.py` (ví dụ `--cost unit|energy`)

#### S1-07 · Prototype Dear ImGui — *TV2 · P0 · 5h*
- [ ] Thêm Dear ImGui (MIT) vào `code/` (backend `imgui_impl_glfw` + `imgui_impl_opengl3`), cập nhật makefile + THIRD_PARTY_NOTICES
- [ ] Một cửa sổ ImGui có nút **Restart** hoạt động; game vẫn chạy bình thường

#### S1-08 · Màn G1/G2 — *TV4 · P0 · 4h*
- [ ] 3–4 màn mới trong `code/data/` (đánh số tiếp 3, 4, …), qua `Level.validate()`
- [ ] Chơi tay thắng được trong game; ghi độ dài tối ưu (BFS) vào `code/data/README.md`

#### S1-09 · Khung thực nghiệm — *TV5 · P1 · 3h*
- [ ] `experiments/run_all.py`: chạy mọi solver × màn qua `SOLVERS`, ghi `results/results.csv`

#### S1-10 · Python đọc `.data` v2 — *TV1 · P0 · 5h*
- [ ] `Level.from_text` đọc phần mở rộng (S0-04); v1 vẫn chạy; test cho file v2 hợp lệ và lỗi

#### S1-11 · Sprint 1 review — *Tất cả · P0 · 1h*
- [ ] Demo `./a.out 2 --solve` cho 4 thuật toán; `pytest` không còn SKIP solver

### Sprint 2 — Luật nâng cao (Ngày 6–10)

#### S2-01 · C++ đọc `.data` v2 — *TV1 · P0 · 5h*
- [ ] `Floor::initialize` đọc phần mở rộng; một công tắc bật/tắt nhiều ô; permanent `on`/`off`
- [ ] Màn v1 cũ vẫn chơi như trước (`--play` lời giải cũ vẫn `WON`)

#### S2-02 · Python permanent + nhiều ô cầu — *TV1 · P0 · 3h*
- [ ] `bridge_mask` biểu diễn đúng cả toggle lẫn permanent; test cho từng chế độ
- [ ] Màn thử: lời giải solver phát lại trong game vẫn `WON`

#### S2-03 · Python split — *TV1 · P0 · 6h*
- [ ] Mở rộng `State` (vị trí 2 cube) + 8 hành động `(cube, hướng)` khi đang tách; tự gộp khi 4-kề
- [ ] Luật cube theo report0 mục 9.1 (#5–#11); test cho tách, gộp, cube trên soft/heavy/fragile/goal

#### S2-04 · C++ split — *TV2 · P0 · 8h*
- [ ] Đứng trên split switch → 2 cube tại vị trí định sẵn; Space đổi cube; tô sáng cube đang điều khiển
- [ ] Tự gộp khi kề nhau; luật khớp Python (S2-03); autoplay phát lại được lời giải có split

#### S2-05 · (Mở rộng) IDS — *TV2 · P2 · 3h*
- [ ] `ai/solvers/ids_solver.py`, đăng ký trong `SOLVERS`; độ dài = BFS trên mọi màn

#### S2-06 · Chốt hàm chi phí — *TV3 · P0 · 3h*
- [ ] Công thức + bảng giá trị + lý giải + chứng minh mọi chi phí > 0; ghi `c_min` cho TV5

#### S2-07 · Test chéo — *TV3 · P0 · 3h*
- [ ] Test tự động: cost(A\*) == cost(UCS) với cùng hàm chi phí; len(A\*, unit) == len(BFS) trên mọi màn

#### S2-08 · Heuristic cho split + chi phí không đồng nhất — *TV5 · P0 · 5h*
- [ ] Khoảng cách trên đồ thị ô (coi mọi cầu mở) + cạnh "cổng" 0 cho split; nhân `c_min` khi chi phí không đồng nhất
- [ ] Test chéo S2-07 vẫn đúng

#### S2-09 · Màn G3 — *TV4 · P0 · 4h*
- [ ] 2–3 màn có công tắc thường/nặng, toggle **và** permanent; có màn bắt buộc quay lại ô cũ với cầu khác

#### S2-10 · Khung report — *TV5 · P1 · 2h*
- [ ] Template theo dàn ý report0 mục 8.3, chia quyền cho cả nhóm

#### S2-11 · Sprint 2 review — *Tất cả · P0 · 1h*
- [ ] Demo chơi tay + `--solve` trên màn có split và permanent

### Sprint 3 — GUI nút bấm + đủ 10 màn (Ngày 10–14)

#### S3-01 · Nút Solve + panel — *TV2 · P0 · 6h*
- [ ] Nút Restart, Solve BFS/DFS/UCS/A\*; bấm Solve → khối tự lăn tới đích
- [ ] Panel: số bước, chi phí, node mở rộng, thời gian, bộ nhớ (đọc từ S3-02)

#### S3-02 · `--stats-out` — *TV3 · P1 · 2h*
- [ ] `main.py --stats-out stats.json` ghi mọi trường của `SearchResult`

#### S3-03 · Solver chạy nền — *TV2 · P0 · 3h*
- [ ] Gọi solver trong `std::thread` (hoặc tương đương), hiển thị "Đang giải…", cửa sổ không treo

#### S3-04 · Màn G4/G5/G6 — *TV4 · P0 · 6h*
- [ ] Tổng **≥ 10 màn** chia nhóm G1–G6 (report0 mục 7.1); có màn lớn, màn vô nghiệm

#### S3-05 · So khớp luật C++ ↔ Python — *TV3 · P0 · 4h*
- [ ] Script: với mỗi màn và mỗi solver, ghi lời giải → `./a.out <n> --play` → phải `WON` đúng số bước
- [ ] Ghi kết quả vào README

#### S3-06 · Thực nghiệm hoàn chỉnh — *TV5 · P0 · 5h*
- [ ] Lặp 5 lần lấy median thời gian; đo bộ nhớ ở lượt riêng; timeout; CSV đủ cột

#### S3-07 · Chứng minh heuristic — *TV5 · P0 · 3h*
- [ ] Mệnh đề + chứng minh admissible & consistent theo hàm chi phí đã chốt; phản ví dụ cho công thức "+1"

#### S3-08 · (Mở rộng) ML cho trọng số chi phí — *TV3 · P2 · 5h*
- [ ] Mô tả dữ liệu, mô hình đơn giản, trọng số thu được và lý giải (Q&A câu 10)

#### S3-09 · Chọn màn — *TV4 · P0 · 4h*
- [ ] Nút New Game mở danh sách màn trong `data/`, chọn để chơi/giải

#### S3-10 · Timeout / giới hạn node — *TV3 · P1 · 3h*
- [ ] `BaseSolver.run(timeout_s=..., max_nodes=...)`, `status = "timeout"`; test

#### S3-11 · Tài liệu màn — *TV4 · P1 · 3h*
- [ ] `code/data/README.md`: kích thước, số ô, công tắc, lời giải tối ưu, ảnh chụp từng màn

#### S3-12 · Sprint 3 review — *Tất cả · P0 · 1h*
- [ ] Bấm 4 nút Solve trên mọi màn, khối tự lăn tới đích

### Sprint 4 — Thực nghiệm & bug bash (Ngày 14–18)

#### S4-01 · Bug bash — *Tất cả · P0 · 2h*
- [ ] Mỗi người chơi tay ≥ 4 màn do người khác tạo; ghi bug (bước tái hiện) lên Notion

#### S4-02 / S4-03 · Sửa bug luật / GUI — *TV1 / TV2*
- [ ] Bug luật: sửa cả C++ và Python + test hồi quy · Bug GUI: sửa + chú thích loại ô, màu, HUD

#### S4-04 / S4-05 · Thực nghiệm chính thức + biểu đồ — *TV5 · P0*
- [ ] Chạy trên bản code sau bug bash; ghi CPU/RAM/OS/phiên bản Python
- [ ] Biểu đồ: node mở rộng (log), thời gian, bộ nhớ, độ dài; bảng tổng hợp

#### S4-06 / S4-07 · Phân tích — *TV3 / TV4 · P0 · 3h*
- [ ] UCS vs BFS: khi nào lời giải khác nhau, vì sao · A\*: so sánh các heuristic, số node tiết kiệm được

#### S4-08 · Kịch bản video — *TV4 · P0 · 2h*
- [ ] Theo report0 mục 8.4; chọn màn demo: thua (fragile), công tắc, split, chạy 4 solver

#### S4-09 · README — *TV1 · P0 · 2h*
- [ ] Cập nhật định dạng v2, nút GUI, cách chạy thực nghiệm

#### S4-10 · Sprint 4 review — *Tất cả · P0 · 1h*
- [ ] Chốt `results.csv` + biểu đồ; **code freeze**

### Sprint 5 — Report, video, đóng gói (Ngày 18–21)

#### S5-01 · % đóng góp — *Tất cả · P0 · 1h*
- [ ] Dựa trên Notion (task Done, giờ thực tế) và lịch sử commit; cả 5 người đồng ý

#### S5-02 … S5-08 · Viết report — *mỗi người phần của mình (mục 2)*
- [ ] Đúng dàn ý report0 mục 8.3; có hình, bảng, pseudocode; ghi rõ DFS hay IDS
- [ ] References có repo chaitanya (link + commit `c0ccb55`), Dear ImGui, GLFW, GLM, GLAD
- [ ] Phụ lục AI: mỗi người ghi trung thực công cụ + prompt chính (kể cả phần khung D-01…D-07)

#### S5-09 · Video — *TV2 · P0 · 6h*
- [ ] Phủ đủ 5đ GUI + 5đ Feature; phụ đề/thuyết minh; YouTube **Public**; mở được ở tab ẩn danh

#### S5-10 · Ráp report — *TV5 · P0 · 4h*
- [ ] Trang đầu có thông tin nhóm + link video; mục lục; đánh số hình/bảng; xuất `Report.pdf`

#### S5-11 · Đọc chéo — *Tất cả · P0 · 1h*
- [ ] Mỗi người đọc ít nhất một chương không phải của mình

#### S5-12 · Cài đặt sạch — *TV3 · P0 · 1h*
- [ ] Clone repo trên máy khác → làm đúng README từ đầu → `make`, `pytest`, `./a.out 1 --solve astar` chạy được

#### S5-13 · Luyện vấn đáp — *Tất cả · P0 · 2h*
- [ ] Mỗi người trình bày 5 phút một phần không phải của mình
- [ ] Câu hỏi luyện: vì sao state phải có `bridge_mask`? BFS tối ưu vì sao? DFS không tối ưu vì sao? UCS ≡ BFS khi chi phí 1? Chứng minh heuristic? Split ảnh hưởng heuristic thế nào? Tọa độ `R` trong file `.data` là gì?

#### S5-14 · Đóng gói & nộp — *TV5 · P0 · 1h*
- [ ] `StudentID1_..._StudentID5/Source/` = nội dung repo (bỏ `demo/`, `.git/`, `*.o`, `a.out`) + `Report.pdf`
- [ ] < 20MB (nếu không, dùng link Drive); nộp trước deadline vài giờ

---

## 7. Mốc quan trọng & đường găng

| Mốc | Ngày | Điều kiện đạt |
|---|---|---|
| **M0 – Khung repo** | ✅ đã xong | Build + `--play` + test luật PASS |
| **M1 – Cả nhóm chạy được** | Ngày 2 | 5/5 người build, chạy game và `pytest`; cú pháp `.data` v2 đã chốt |
| **M2 – Đủ 4 solver** | Ngày 6 | `pytest` hết SKIP; `--solve` chạy được với 4 thuật toán |
| **M3 – Đủ luật của đề** | Ngày 10 | Nhiều ô cầu, permanent, split chạy ở cả C++ và Python |
| **M4 – GUI đầy đủ + ≥ 10 màn** | Ngày 14 | Nút Solve trên GUI; ≥ 10 màn; test so khớp C++ ↔ Python |
| **M5 – Code freeze** | Ngày 18 | Số liệu và biểu đồ cuối cùng |
| **M6 – Nộp bài** | Ngày 21 | PDF + video + zip đã nộp |

**Đường găng:**

```
S0-04 → S1-10 → S2-01 → S2-04 → S3-04 → S3-05 → S4-02 → S4-04 → S4-05 → S5-07 → S5-10 → S5-14
GUI:    S1-07 → S3-01 → S3-03 → S4-03 → S4-08 → S5-09 ──────────────────────────────────┘
```

Chuỗi chính đi qua **TV1** (luật v2) → **TV2** (split C++) → **TV4** (màn) → **TV5** (thực nghiệm). Nếu tới Ngày 8 S2-01 chưa xong, TV3 hỗ trợ TV1; nếu tới Ngày 9 S2-04 chưa xong, TV4 hỗ trợ TV2.

---

## 8. Quy tắc làm việc nhóm

1. **Mỗi task một branch, một PR** (`feature/<ID>-<tên>`), có ít nhất 1 reviewer.
2. **Luật ở hai nơi:** sửa luật thì sửa cả `code/*.cpp` và `ai/problem.py`, thêm test, chạy `--play` để so khớp.
3. **Không sửa** thân solver của người khác; góp ý qua review.
4. Cập nhật trạng thái trên Notion khi bắt đầu / gửi review / hoàn thành; ghi **giờ thực tế**.
5. Task bị kẹt quá **1 ngày** → báo ngay trong nhóm.
6. Dùng AI thì ghi công cụ + prompt chính vào trang Notion chung để làm phụ lục report.
