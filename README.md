# CSAI_Group9_Lab01 — Bloxorz Solver

Đồ án Project 1 môn **CSC14003 – Cơ sở Trí tuệ Nhân tạo** (Khoa CNTT, ĐH KHTN – ĐHQG-HCM), nhóm 9.

- **Game 3D (C++/OpenGL 3.3)** trong `code/`, dựa trên [chaitanya100100/Bloxorz](https://github.com/chaitanya100100/Bloxorz) @ `c0ccb55` (tác giả Chaitanya Patel; dùng theo sự cho phép của giảng viên — xem [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)). Nhóm đã bổ sung: tự phát lại lời giải, gọi solver Python, kiểm tra biên, build đa nền tảng, **giao diện mới** (nền tối, ánh sáng, hiệu ứng, menu/HUD bằng Dear ImGui), **build Windows native** và **đóng gói `Bloxorz.exe`** — xem [Docs/report2.md](Docs/report2.md); cơ chế v2 (nhiều ô cầu, permanent, split), **thanh New Game / Restart / Pause**, **bảng AI trong màn**, **Search Visualizer**, **âm thanh + nhạc nền**, **sàn sụp và tự chơi lại khi rơi**, **theme Minecraft** — xem [Docs/report4.md](Docs/report4.md); **bóng đổ thật + bloom**, **theme Original** kiểu game gốc, **màn hình chính mới** (chọn màn có bản đồ xem trước, kỷ lục), **camera xoay/zoom bằng chuột** — xem [Docs/report5.md](Docs/report5.md).
- **Phần AI (Python)** trong `ai/`: mô phỏng chính xác luật của game, thuật toán **BFS, DFS, A\*** (UCS đang làm), heuristic Manhattan và ghi vết tìm kiếm cho Visualizer.

Tài liệu nội bộ: **[Docs/report1.md](Docs/report1.md) (bắt đầu từ đây — việc còn lại của từng thành viên)**, [Docs/play.md](Docs/play.md) (cách chạy game trong VS Code trên Windows), [Docs/report5.md](Docs/report5.md) (camera mới, bóng đổ + bloom, theme Original, màn hình chính mới), [Docs/report4.md](Docs/report4.md) (merge v2 + màn khó, sửa luật, heuristic, nâng cấp giao diện), [Docs/report3.md](Docs/report3.md) (review + merge bộ màn v1 và A\*), [Docs/system.md](Docs/system.md) (giải thích toàn bộ hệ thống và thuật toán), [Docs/report2.md](Docs/report2.md) (nâng cấp GUI + đóng gói), [Docs/report0.md](Docs/report0.md) (phân tích đề), [Docs/task.md](Docs/task.md) (phân công), [Docs/implementation_plan.md](Docs/implementation_plan.md) (kế hoạch kỹ thuật).

## Trạng thái hiện tại

| Phần | Trạng thái |
|---|---|
| Luật game trong Python (`ai/problem.py`) | ✅ Đầy đủ cả v2: nhiều ô cầu, permanent on/off, split switch (Đạt); đối chiếu C++ ↔ Python ở [report4](Docs/report4.md) |
| Game phát lại lời giải (`--play`, `--solve`) | ✅ Cả 15 màn có lời giải đều thắng trong game C++ đúng số bước |
| `BFSSolver` | ✅ Đã cài (Đạt, [Docs/bfs_walkthrough.md](Docs/bfs_walkthrough.md)) |
| `AStarSolver` | ✅ Đã cài (Sang, [Docs/astar_walkthrough.md](Docs/astar_walkthrough.md)) |
| `manhattan_heuristic` | ✅ Đã cài (TV5): Manhattan có tính cổng split, admissible + consistent, kiểm chứng trên mọi trạng thái của 16 màn |
| `DFSSolver` | ✅ Đã cài: ngăn xếp tường minh, đánh dấu đã thăm khi đẩy, kiểm tra đích khi lấy ra, thứ tự U, D, L, R |
| `UCSSolver` | ⏳ **TODO** |
| Giao diện: New Game / Restart / Pause, bảng AI trong màn, Search Visualizer, âm thanh, theme Minecraft | ✅ Xong — xem [Docs/report4.md](Docs/report4.md) |
| Bộ màn | ✅ 16 màn (15 có lời giải, 1 vô nghiệm): màn 4/6 công tắc, 200 ô dễ vỡ, màn v2 nhiều cầu/permanent (15) và split (16) — xem [code/data/README.md](code/data/README.md) |

## Cấu trúc thư mục

```
.
├── main.py                 # CLI chạy solver
├── ai/
│   ├── level.py            # đọc file .data (đúng định dạng của game C++)
│   ├── state.py            # State(x, y, orientation, bridge_mask)
│   ├── problem.py          # BloxorzProblem: luật game
│   ├── heuristics.py       # heuristic cho A* (Manhattan có tính cổng split)
│   ├── tracing.py          # ghi vết tìm kiếm cho Search Visualizer (--trace)
│   └── solvers/            # base_solver.py + bfs/dfs/ucs/astar_solver.py
├── tests/                  # pytest
├── code/                   # game C++/OpenGL (makefile, data/*.data, include/ GLAD, third_party/imgui + miniaudio)
└── Docs/                   # đề bài và tài liệu nhóm
```

## Cài đặt (WSL Ubuntu / Linux — môi trường chính của nhóm)

```bash
sudo apt update
sudo apt install -y g++ make libglfw3-dev libglm-dev libgl-dev python3 python3-pytest
```

- Windows (**khuyên dùng để chơi**): MSYS2 MinGW64 — `pacman -S make zip mingw-w64-x86_64-{gcc,glfw,glm}`, build trong shell *MSYS2 MinGW x64*. Game chạy bằng GPU thật (WSLg vẽ bằng CPU — `llvmpipe` — nên chậm).
- Windows qua **WSL 2** vẫn dùng được cho phát triển/kiểm chứng. Phần Python và test chạy được cả trên Python của Windows (`pip install -r requirements.txt`).
- macOS: `brew install glfw glm`.
- Python **≥ 3.10**, phần AI chỉ dùng thư viện chuẩn.

## Build và chạy game

```bash
cd code            # BẮT BUỘC chạy trong code/ (game đọc data/ và shader theo đường dẫn tương đối)
make               # tạo ./a.out (Windows/MSYS2: a.exe) — object nằm trong build/<linux|win64>/

./a.out                            # chế độ game: menu chọn màn, Restart, nút Solve BFS/DFS/UCS/A*
./a.out 1                          # vào thẳng màn 1 (vẫn có menu khi bấm Esc)
./a.out 1 --play solution.txt      # tự phát lại file lời giải (các nước U/D/L/R cách nhau bởi khoảng trắng)
./a.out 2 --solve astar            # gọi solver Python rồi tự phát lại: bfs | dfs | ucs | astar
./a.out 5 --visualize bfs          # mở thẳng Search Visualizer (BFS) ở màn 5
./a.out 1 --theme original         # theme Original (kiểu game gốc); còn classic, minecraft (đổi được trong Options)
./a.out --help                     # thêm các tùy chọn cho dev: --no-vsync, --no-shadows, --no-bloom, --camera, --shot, --quit-after, --mute, --ui, --keep-open
```

- Với `--play`/`--solve` (chế độ kiểm chứng): game in `WON` hoặc `GAME OVER` kèm số bước, giữ màn hình 2 giây rồi thoát (mã thoát 0 = thắng, 1 = thua). Không có hai tham số này thì chạy như một game bình thường, không tự thoát.
- Solver chạy `python3 ../main.py data/<level>.data --algo <algo> --out solution.txt` (bản đóng gói dùng `solver/main.py`). Đổi trình thông dịch bằng biến môi trường `BLOXORZ_PYTHON` (mặc định `python3`; Windows: `python`, rồi `py -3`). Trong menu, solver chạy ở luồng nền và ghi output vào `solver_log.txt`.
- Điều khiển: **mũi tên** lăn khối (bị khóa khi đang phát lại); **Space** đổi khối nhỏ đang điều khiển sau khi tách (chưa tách: đổi phép chiếu); **R** chơi lại; **P** hoặc **Esc** tạm dừng; **Enter** sang màn tiếp khi thắng; camera **C/B/N/M/X/V** (kéo chuột để xoay, lăn chuột để zoom; C bám khối không lắc); **Tab** bản đồ nhỏ; **F1** ẩn/hiện bảng phím; **F12** chụp màn hình; **Q** thoát. Khi Search Visualizer mở: **←/→** lùi/tiến một bước, **Space** chạy/dừng.
- Màn hình chính: **Play / Select Stage / Options / How to Play / Credits** (chuột hoặc ↑↓ + Enter). Trong màn: thanh **Menu / Restart / Pause** ở trên, **bảng AI** bên phải (BFS/DFS/UCS/A\*, chọn heuristic, **Visualize search**, 4 chỉ số của lần chạy gần nhất). Rơi khỏi sàn thì sàn sụp và màn tự chơi lại (như Bloxorz gốc).
- Âm thanh và nhạc nền được tổng hợp bằng code (không dùng file của game gốc vì bản quyền). Muốn thay: đặt `sounds/<tên>.wav|.mp3|.flac` cạnh file chạy (tên: `roll stand switch heavy bridge_on bridge_off fall win split merge crack click start collapse music`). Cài đặt (theme, âm lượng…) lưu ở `settings.ini`.
- Trên WSLg có thể thấy cảnh báo `MESA`/`libEGL` — vô hại.

### Đóng gói thành game app (Windows)

Trong shell *MSYS2 MinGW x64*, thư mục `code/`:

```bash
make release       # Bloxorz.exe: link tĩnh (không cần DLL), không mở console, có icon
make dist          # ../dist/win64/Bloxorz/ (exe + data/ + solver/ + licenses/) và ../dist/Bloxorz-win64.zip
```

Giải nén zip ở đâu cũng chạy được (kể cả đường dẫn có dấu). Chơi không cần cài gì; nút Solve cần Python ≥ 3.10. Linux: `make dist` tạo `bloxorz` + `Bloxorz-linux.zip` (hoặc `.tar.gz`).

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
python main.py code/data/5.data --algo bfs --trace trace.txt --stats stats.txt   # vết tìm kiếm + 4 chỉ số (game dùng)
```

Solver chưa cài sẽ được báo **"chưa cài"** (không crash). Mã thoát: 0 có lời giải · 1 vô nghiệm · 2 chưa cài · 3 lời giải sai luật · 4 lỗi đọc màn.

## Chạy test

```bash
python -m pytest -q        # hiện tại: 188 passed, 3 skipped (UCS chưa cài)
```

- `tests/test_problem.py`: luật game (bảng chuyển, rơi, ô cam, công tắc, cầu, lời giải đã biết của màn 1 và 2).
- `tests/test_solvers.py`: mỗi solver phải trả về lời giải hợp lệ khi phát lại; BFS/UCS/A\* phải tối ưu (màn 1 = **10** bước, màn 2 = **68** bước). Solver chưa cài → SKIP.
- `tests/test_levels.py`: lời giải và độ dài tối ưu của cả 14 màn theo BFS độc lập; xác nhận màn vô nghiệm, viền trống, cầu bắt buộc và tác dụng của ô dễ vỡ.
- `tests/test_hard_levels.py`: đối chiếu A* với BFS trên màn 12–14; kiểm tra đủ 16/64 cấu hình cầu và độ lớn không gian tìm kiếm, cũng như việc lời giải thực sự đi qua nhiều ô dễ vỡ.
- `tests/test_astar_solver.py`, `tests/test_astar_integration.py`: các trường hợp biên của A\* (vô nghiệm, xuất phát tại đích, mở rộng lại, bản ghi cũ) và đối chiếu heuristic với khoảng cách thật trên toàn bộ đồ thị trạng thái của màn 1, 2, 15, 16.
- `tests/test_heuristics.py`: giá trị Manhattan trên màn đồ chơi của `Docs/system.md`, phản ví dụ "+1 khi nằm", màn có cổng split.
- `tests/test_v2_switches.py`, `tests/test_split.py`, `tests/test_bfs_solver.py`: cơ chế v2 và BFS (Đạt), kèm các ca lệch C++ ↔ Python đã sửa ở report4.
- `tests/test_dfs_solver.py`: DFS đi sâu theo thứ tự U, D, L, R, chống lặp, khớp bảng chạy tay ở `Docs/system.md` mục 8.2 (22 node), giải mọi màn có lời giải, duyệt hết màn vô nghiệm, lời giải khớp cây của Search Visualizer.
- `tests/test_tracing.py`: vết tìm kiếm cho Visualizer đúng thứ tự mở rộng và không làm đổi kết quả solver.

## Mô hình trạng thái

**Tọa độ** (giống hệt game C++): `x` = chỉ số **dòng** trong file `.data`, `y` = chỉ số **cột**.
Hướng đi: `R: x+1`, `L: x−1`, `U: y+1`, `D: y−1` — trong cửa sổ 3D `R` là sang phải, nhưng **trong file text `R` là xuống một dòng**.

**Tư thế:** `Z` đứng, chiếm `(x,y)` · `X` nằm, chiếm `(x,y),(x+1,y)` · `Y` nằm, chiếm `(x,y),(x,y+1)`.

**State** = `(x, y, orientation, bridge_mask, cubes)` — `bridge_mask` có 1 bit cho mỗi **ô cầu** (bit bật = ô cầu đang mở); `cubes` là vị trí hai khối 1×1×1 khi đã tách (split), `None` khi chưa tách. Cùng vị trí nhưng khác `bridge_mask` là **hai trạng thái khác nhau**.

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
