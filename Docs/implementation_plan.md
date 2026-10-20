# Implementation plan — Bloxorz Solver (nhóm 9)

> Kế hoạch kỹ thuật cho repo `CSAI_Group9_Lab01`. Phân tích đề: `Docs/report0.md`. Phân công: `Docs/task.md`.
> Cập nhật: 20/10/2026.

## 1. Mục tiêu

1. Dùng game 3D C++/OpenGL của `chaitanya100100/Bloxorz` @ `c0ccb55` làm GUI (giảng viên cho phép; GUI phải 3D, được viết bằng ngôn ngữ khác).
2. Dựng phần AI bằng Python theo OOP: luật game cài **đầy đủ và đúng** (dùng chung), 4 solver **để TODO** cho từng thành viên tự cài.
3. Game gọi solver Python và **tự phát lại lời giải** (giảng viên: "GUI tự cho khối lăn đến đích").
4. Bổ sung những gì đề yêu cầu mà game gốc chưa có: một công tắc nhiều ô cầu, permanent, split, nút GUI, ≥ 10 màn.

## 2. Luật của game gốc (đọc từ C++, đã kiểm chứng)

| Nguồn | Nội dung |
|---|---|
| `Floor::initialize` (`tile.cpp`) | Định dạng `.data`: `n m` · lưới `n×m` (0 trống/lỗ, 1 sàn, 2 cam, 3 công tắc thường, 4 công tắc nặng) · `sx sy X/Y/Z` · `tx ty` · `k` dòng `x y a b` (công tắc thường) · `h` dòng `x y a b` (công tắc nặng). Mỗi công tắc điều khiển **một** ô. |
| `SuperBlock::update` (`superblock.cpp`) | Bảng chuyển: X: L→Z(x−1), R→Z(x+2), U→X(y+1), D→X(y−1) · Y: U→Z(y+2), D→Z(y−1), R→Y(x+1), L→Y(x−1) · Z: L→X(x−2), R→X(x+1), U→Y(y+1), D→Y(y−2). Sau khi lăn xong gọi `check_after_step`. |
| `Game::check` (`game.cpp`, mỗi frame) | Ô đầu trống: không phải đích → thua; là đích và đứng → thắng. Đứng trên ô cam → thua. X/Y: ô thứ hai trống, không phải đích → thua. Thua ưu tiên hơn thắng. |
| `Game::check_after_step` | Nếu `check` ≠ tiếp tục → **không** kích hoạt công tắc. Ngược lại, tuần tự: ô đầu là công tắc thường → toggle; ô thứ hai (X/Y) là công tắc thường → toggle; đứng trên công tắc nặng → toggle. Toggle: ô trống → tạo sàn type 1; có sàn → xóa. Frame sau `check` chạy lại → cầu dưới khối bị xóa thì thua. |

**Kiểm chứng:** BFS trên mô phỏng Python: màn 1 tối ưu 10 bước (161 trạng thái), màn 2 tối ưu 68 bước (378 trạng thái). Các lời giải `R U U R U R U R D L` và 68 bước đều thắng đúng ở nước cuối trong game thật.

## 3. Phase 1 — Khung (✅ đã xong)

| Bước | Nội dung | Kết quả |
|---|---|---|
| 1 | Commit mốc: code gốc nguyên vẹn (`Import upstream … @ c0ccb55`), `demo/` không đưa lên git | ✅ |
| 2 | `ai/level.py`, `state.py`, `problem.py` (luật đầy đủ), `heuristics.py` (TODO), `solvers/` (`BaseSolver`, `SearchResult` có thêm `peak_memory_kb` vì đề bắt buộc đo bộ nhớ; 4 solver TODO), `main.py` CLI | ✅ |
| 3 | `tests/test_problem.py` (luật) + `tests/test_solvers.py` (skip khi chưa cài) | ✅ 36 passed, 10 skipped (Windows Python 3.12 và WSL Python 3.14) |
| 4 | Game: `SuperBlock::is_moving`, `autoplay.h/.cpp` (phát lại 0.25 s/nước, `run_solver` qua `system()`, `$BLOXORZ_PYTHON`), `main.cpp` (`[level] [--solve algo \| --play file]`, giữ màn hình 1.5 s, mã thoát), khóa phím khi phát lại, `Floor::has_tile` (kiểm tra biên), `delete` thay `free` | ✅ |
| 4b | GLAD 0.1.36 cho GL 3.3 core (không extension) vào `code/include/`, thay `glad.c`; xóa `code/glad` (glad.c cũ) và `code/a.out`; `headers.h` thêm `GLM_ENABLE_EXPERIMENTAL`; makefile Linux/macOS/MSYS2 | ✅ build trên WSL Ubuntu 26.04 (GLFW 3.4, GLM 1.0.1) |
| 4c | Chạy thật: `./a.out 1 --play` → `WON, Steps : 10`; `./a.out 2 --play` → 68; nước thua → `GAME OVER`, mã 1; `--solve` khi chưa cài → báo lỗi gọn; `--solve` với solver tham chiếu (ngoài repo) → `WON` | ✅ |
| 5 | `README.md`, `THIRD_PARTY_NOTICES.md`, `CLAUDE.md` (chỉ ở máy, không đưa lên git), cập nhật `report0.md`, `task.md` | ✅ |

## 4. Phase 2 — Việc của nhóm (theo `Docs/task.md`)

### 4.1. Bốn solver + heuristic (Sprint 1)

- Mỗi người cài `solve()` trong file của mình theo docstring; không đổi chữ ký hàm.
- Tiêu chí: `pytest` hết SKIP; BFS/UCS/A\* = 10 / 68 bước; `./a.out 2 --solve <algo>` → `WON`.

### 4.2. Định dạng `.data` v2 (Sprint 0–2) — cần chốt ở task S0-04

Yêu cầu: **tương thích ngược** (file v1 đọc như cũ), parser C++ và Python đọc giống hệt nhau. Phác thảo (sau phần v1):

```
ext 1
switch <normal|heavy> <x> <y> <toggle|on|off> <số ô>   # thay cho dòng v1 nếu muốn nhiều ô / permanent
<a1> <b1> <a2> <b2> ...
split <x> <y> <x1> <y1> <x2> <y2>                      # split switch + 2 ô dịch chuyển
```

Ảnh hưởng tới trạng thái:
- **Nhiều ô cầu:** không đổi `State` — ô cầu mở khi (ban đầu) XOR (parity các bit công tắc điều khiển nó), đã hỗ trợ trong `Level.bridge_bits`.
- **Permanent `on`/`off`:** bit = "đã kích hoạt ít nhất một lần"; ô cầu do công tắc permanent điều khiển có trạng thái cố định sau lần kích hoạt đầu. Cần quy tắc rõ khi một ô bị cả toggle lẫn permanent điều khiển (đề xuất: cấm trong `validate()`).
- **Split:** `State` thêm trường cho 2 cube (ví dụ `cubes: tuple[(x1,y1),(x2,y2)] | None`, chuẩn hóa thứ tự) và 8 hành động `(cube, hướng)`; luật cube theo `report0.md` mục 9.1 (#5–#11). C++: `SuperBlock` cần chế độ hai cube, phím Space, vẽ hai khối 1×1×1.

### 4.3. GUI (Sprint 1–3)

- Dear ImGui (MIT, backend `imgui_impl_glfw` + `imgui_impl_opengl3`): New Game/chọn màn, Restart, Solve BFS/DFS/UCS/A\*, panel số liệu (đọc từ `main.py --stats-out`).
- Gọi solver trong `std::thread` để cửa sổ không treo; dùng lại `run_solver` và `AutoPlayer` có sẵn.
- Restart: gọi lại `game.initialize(level)` + reset `game_result`, `Game::gameover/won`, `autoplayer`.

### 4.4. Màn chơi (≥ 10) và thực nghiệm

- Nhóm G1–G6 theo `report0.md` mục 7.1; mỗi màn qua `Level.validate()`, chơi tay thắng được, ghi lời giải tối ưu.
- `experiments/run_all.py` → `results/results.csv`; `plot.py` → biểu đồ; đo bộ nhớ ở lượt riêng (`--memory` trong `main.py` đã làm như vậy).
- Test so khớp: phát lại lời giải của mọi solver bằng `./a.out <n> --play` trên mọi màn.

## 5. Rủi ro và cách giảm

| Rủi ro | Cách giảm |
|---|---|
| Luật C++ và Python lệch nhau | Quy tắc sửa cả hai + test + `--play`; test so khớp toàn bộ màn (S3-05) |
| Thành viên không build được C++ | README hướng dẫn WSL; task S0-02 bắt buộc ngày 2 |
| Split khó, trễ | Bắt đầu sớm (Sprint 2), chia logic (TV1) và vẽ (TV2); có người hỗ trợ dự phòng |
| Màn quá nhỏ → thực nghiệm không có gì để phân tích | G5/G6 có màn lớn, nhiều công tắc |
| `system()` chặn cửa sổ khi giải lâu | Luồng nền (S3-03) + timeout trong `BaseSolver` (S3-10) |
| Repo gốc không có license | Thầy đã cho phép; cite đầy đủ ở README, THIRD_PARTY_NOTICES, References |

## 6. Kiểm chứng (chạy lại bất cứ lúc nào)

```bash
python -m pytest -q                                    # Phase 1: 36 passed, 10 skipped
python main.py code/data/1.data --compare              # 4 thuật toán: "chưa cài" cho tới khi nhóm cài
cd code && make && echo "R U U R U R U R D L" > /tmp/s1 && ./a.out 1 --play /tmp/s1   # WON, Steps : 10
```
