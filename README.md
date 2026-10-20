# CSAI_Group9_Lab01 — Bloxorz Solver

Đồ án Project 1 môn **CSC14003 – Cơ sở Trí tuệ Nhân tạo** (Khoa CNTT, ĐH KHTN – ĐHQG-HCM), nhóm 9.

- **Game 3D (C++/OpenGL 3.3)** trong `code/`, dựa trên [chaitanya100100/Bloxorz](https://github.com/chaitanya100100/Bloxorz) @ `c0ccb55` (tác giả Chaitanya Patel; dùng theo sự cho phép của giảng viên — xem [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)). Nhóm đã bổ sung: tự phát lại lời giải, gọi solver Python, kiểm tra biên, build đa nền tảng.
- **Phần AI (Python)** trong `ai/`: mô phỏng chính xác luật của game và khung cho các thuật toán **BFS, DFS, UCS, A\***.

Tài liệu nội bộ: **[Docs/report1.md](Docs/report1.md) (bắt đầu từ đây — việc còn lại của từng thành viên)**, [Docs/report0.md](Docs/report0.md) (phân tích đề), [Docs/task.md](Docs/task.md) (phân công), [Docs/implementation_plan.md](Docs/implementation_plan.md) (kế hoạch kỹ thuật).

## Trạng thái hiện tại

| Phần | Trạng thái |
|---|---|
| Luật game trong Python (`ai/problem.py`) | ✅ Cài đầy đủ, 36 test PASS |
| Game phát lại lời giải (`--play`, `--solve`) | ✅ Đã kiểm chứng: màn 1 thắng sau 10 bước, màn 2 sau 68 bước |
| `BFSSolver`, `DFSSolver`, `UCSSolver`, `AStarSolver` | ⏳ **TODO** — mỗi thành viên tự cài |
| `manhattan_heuristic` | ⏳ **TODO** |
| Nút GUI (New Game / Restart / Solve), split switch, permanent, ≥ 10 màn | ⏳ Kế hoạch — xem `Docs/task.md` |

## Cấu trúc thư mục

```
.
├── main.py                 # CLI chạy solver
├── ai/
│   ├── level.py            # đọc file .data (đúng định dạng của game C++)
│   ├── state.py            # State(x, y, orientation, bridge_mask)
│   ├── problem.py          # BloxorzProblem: luật game
│   ├── heuristics.py       # heuristic cho A*
│   └── solvers/            # base_solver.py + bfs/dfs/ucs/astar_solver.py
├── tests/                  # pytest
├── code/                   # game C++/OpenGL (makefile, data/*.data, include/ GLAD)
└── Docs/                   # đề bài và tài liệu nhóm
```

## Cài đặt (WSL Ubuntu / Linux — môi trường chính của nhóm)

```bash
sudo apt update
sudo apt install -y g++ make libglfw3-dev libglm-dev libgl-dev python3 python3-pytest
```

- Windows: dùng **WSL 2** (Windows 11 có sẵn WSLg để mở cửa sổ OpenGL). Phần Python và test chạy được cả trên Python của Windows (`pip install -r requirements.txt`).
- macOS: `brew install glfw glm`. Windows thuần: MSYS2 MinGW64 (`pacman -S make mingw-w64-x86_64-{gcc,glfw,glm}`).
- Python **≥ 3.10**, phần AI chỉ dùng thư viện chuẩn.

## Build và chạy game

```bash
cd code            # BẮT BUỘC chạy trong code/ (game đọc data/ và shader theo đường dẫn tương đối)
make               # tạo ./a.out (Windows/MSYS2: a.exe)

./a.out 1                          # chơi màn 1 bằng tay (mặc định: màn 2)
./a.out 1 --play solution.txt      # tự phát lại file lời giải (các nước U/D/L/R cách nhau bởi khoảng trắng)
./a.out 2 --solve astar            # gọi solver Python rồi tự phát lại: bfs | dfs | ucs | astar
```

- Khi thắng/thua, game in `WON` hoặc `GAME OVER` kèm số bước, giữ màn hình 1.5 giây rồi thoát (mã thoát 0 = thắng, 1 = thua).
- `--solve` chạy `python3 ../main.py data/<level>.data --algo <algo> --out solution.txt`. Đổi trình thông dịch bằng biến môi trường `BLOXORZ_PYTHON` (mặc định `python3`, trên Windows là `python`).
- Điều khiển: **phím mũi tên** để lăn (bị khóa khi đang tự phát lại); camera **X/C/V/B/N/M**; **Space** đổi phép chiếu; **Q/Esc** thoát.
- Trên WSLg có thể thấy cảnh báo `MESA`/`libEGL` — vô hại.

Thử nhanh với lời giải đã biết của màn 1:

```bash
cd code
echo "R U U R U R U R D L" > /tmp/sol1.txt
./a.out 1 --play /tmp/sol1.txt     # → WON, Steps : 10
```

## Chạy solver bằng dòng lệnh

```bash
python main.py code/data/1.data --algo bfs                  # in số bước, node mở rộng, frontier max, thời gian
python main.py code/data/2.data --algo astar --out sol.txt  # ghi lời giải "R U U R ..." ra file
python main.py code/data/1.data --compare --memory          # so sánh cả 4 thuật toán (+ bộ nhớ đỉnh)
python main.py code/data/1.data --algo astar --heuristic zero
```

Solver chưa cài sẽ được báo **"chưa cài"** (không crash). Mã thoát: 0 có lời giải · 1 vô nghiệm · 2 chưa cài · 3 lời giải sai luật · 4 lỗi đọc màn.

## Chạy test

```bash
python -m pytest -q        # hiện tại: 36 passed, 10 skipped (solver chưa cài)
```

- `tests/test_problem.py`: luật game (bảng chuyển, rơi, ô cam, công tắc, cầu, lời giải đã biết của màn 1 và 2).
- `tests/test_solvers.py`: mỗi solver phải trả về lời giải hợp lệ khi phát lại; BFS/UCS/A\* phải tối ưu (màn 1 = **10** bước, màn 2 = **68** bước). Solver chưa cài → SKIP.

## Mô hình trạng thái

**Tọa độ** (giống hệt game C++): `x` = chỉ số **dòng** trong file `.data`, `y` = chỉ số **cột**.
Hướng đi: `R: x+1`, `L: x−1`, `U: y+1`, `D: y−1` — trong cửa sổ 3D `R` là sang phải, nhưng **trong file text `R` là xuống một dòng**.

**Tư thế:** `Z` đứng, chiếm `(x,y)` · `X` nằm, chiếm `(x,y),(x+1,y)` · `Y` nằm, chiếm `(x,y),(x,y+1)`.

**State** = `(x, y, orientation, bridge_mask)` — `bridge_mask` có 1 bit cho mỗi công tắc (số lần đã kích hoạt, mod 2). Cùng vị trí nhưng khác `bridge_mask` là **hai trạng thái khác nhau**.

| Tư thế | L | R | U | D |
|---|---|---|---|---|
| `Z (x,y)` | `X (x−2, y)` | `X (x+1, y)` | `Y (x, y+1)` | `Y (x, y−2)` |
| `X (x,y)` | `Z (x−1, y)` | `Z (x+2, y)` | `X (x, y+1)` | `X (x, y−1)` |
| `Y (x,y)` | `Y (x−1, y)` | `Y (x+1, y)` | `Z (x, y+2)` | `Z (x, y−1)` |

**Luật** (chép từ `Game::check` / `check_after_step`):
- Thua nếu ô nào của khối không có sàn (ngoài lưới, ô 0, cầu đang tắt) — trừ lỗ đích; thua nếu **đứng** trên ô cam (fragile). Thua được ưu tiên hơn thắng.
- Thắng khi **đứng** trên lỗ đích. Nằm vắt qua lỗ đích không thắng, không thua.
- Sau một nước **không** thắng/thua, kích hoạt công tắc theo thứ tự: ô đầu là công tắc thường → bật/tắt; ô thứ hai là công tắc thường → bật/tắt; **đứng** trên công tắc nặng → bật/tắt. Nếu cầu dưới khối vừa tắt → rơi.

**Định dạng `.data`:** `n m` → lưới `n×m` (0 trống/lỗ, 1 sàn, 2 cam, 3 công tắc thường, 4 công tắc nặng) → `sx sy X|Y|Z` → `tx ty` (đích) → `k` + `k` dòng `x y a b` (công tắc thường tại `(x,y)` bật/tắt ô `(a,b)`) → `h` + `h` dòng cho công tắc nặng. Ô cầu và ô đích phải là 0 trên lưới.

## Phân công gợi ý

Mỗi người một file solver, một người làm heuristic và báo cáo so sánh (chi tiết: [Docs/task.md](Docs/task.md)).

| Thành viên | File chính | Phụ trách thêm |
|---|---|---|
| TV1 | `ai/solvers/bfs_solver.py` | Mở rộng luật (định dạng v2, nhiều ô cầu, permanent, split) ở cả C++ và Python |
| TV2 | `ai/solvers/dfs_solver.py` | GUI: nút New Game/Restart/Solve, vẽ split; quay video |
| TV3 | `ai/solvers/ucs_solver.py` | Hàm chi phí không đồng nhất, đo đạc, test so khớp C++ ↔ Python |
| TV4 | `ai/solvers/astar_solver.py` | Thiết kế ≥ 10 màn, chọn màn trong GUI |
| TV5 | `ai/heuristics.py` | Chứng minh heuristic, thực nghiệm + biểu đồ, ráp báo cáo |

**Quy tắc:** sửa luật thì sửa **cả** `code/*.cpp` và `ai/problem.py`, thêm test, rồi chạy `./a.out <n> --play <lời giải>` để so khớp.

## Ghi nguồn

- Game gốc: Chaitanya Patel, [chaitanya100100/Bloxorz](https://github.com/chaitanya100100/Bloxorz), commit `c0ccb55b273f24abdee4eb4adc0620e016d748ad`. Video demo gốc: xem thư mục `demo/` của repo gốc.
- Thư viện: GLFW, GLM, GLAD — chi tiết trong [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
