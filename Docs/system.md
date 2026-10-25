# System — Giải thích toàn bộ hệ thống Bloxorz Solver (nhóm 9)

> **Đọc cho ai:** cả 5 thành viên. Đề có ghi giảng viên được quyền **vấn đáp ngẫu nhiên** (mục 5 của `AI - Project 1.pdf`), nên ai cũng phải giải thích được mọi phần của hệ thống, kể cả phần mình không code.
> **Nguồn:** đề `Docs/AI - Project 1.pdf` (mục 2 luật chơi, mục 3 yêu cầu) và mã nguồn ở commit `6d54dad` (nhánh `main`).
> **Cách đọc:** mục 1–2 cho bức tranh chung và luật chơi. Mục 3–5 đi qua **từng file** trong `ai/` và `code/`. Mục 6–7 giải thích bài toán tìm kiếm và **cách từng thuật toán chạy**, có ví dụ dò tay. Mục 8–10 là đo đạc, việc còn thiếu và câu hỏi vấn đáp.
>
> Tài liệu này chỉ giải thích và dùng mã giả. Thân các hàm `solve()` và `manhattan_heuristic` vẫn là phần nhóm tự cài (quy tắc của repo). Các con số "tham chiếu" bên dưới được tính bằng một bản cài thử nằm ngoài repo, chỉ để minh họa độ lớn. Số liệu chính thức phải lấy từ solver của nhóm.

---

## Mục lục

1. [Bức tranh tổng thể](#1-bức-tranh-tổng-thể)
2. [Luật chơi và quy ước tọa độ](#2-luật-chơi-và-quy-ước-tọa-độ)
3. [Thư mục `ai/` — từng file](#3-thư-mục-ai--từng-file)
4. [Các file Python xung quanh: `main.py`, `tests/`](#4-các-file-python-xung-quanh-mainpy-tests)
5. [Thư mục `code/` — từng file](#5-thư-mục-code--từng-file)
6. [Hình thức hóa bài toán tìm kiếm](#6-hình-thức-hóa-bài-toán-tìm-kiếm)
7. [Các thuật toán tìm kiếm chạy như thế nào](#7-các-thuật-toán-tìm-kiếm-chạy-như-thế-nào)
8. [Đo đạc và thực nghiệm](#8-đo-đạc-và-thực-nghiệm)
9. [Khoảng cách so với đề — việc còn lại](#9-khoảng-cách-so-với-đề--việc-còn-lại)
10. [Câu hỏi vấn đáp thường gặp](#10-câu-hỏi-vấn-đáp-thường-gặp)

---

## 1. Bức tranh tổng thể

### 1.1. Đề bài yêu cầu gì

Đề `AI - Project 1.pdf` yêu cầu viết lại game **Bloxorz**: lăn khối hộp 1×1×2 trên lưới ô vuông sao cho khối **đứng thẳng** rơi vào lỗ đích. Sau đó viết 4 solver **BFS, DFS (hoặc IDS), UCS, A\*** tự cài bằng Python, đo hiệu năng và viết báo cáo. Thang điểm (mục 3):

| Mục đề | Điểm | Nội dung |
|---|---:|---|
| 3.1 Reimplement the Game | 20 | 10đ core (sàn, ô trống, lỗ đích, 3 tư thế, 4 hướng lăn, phát hiện rơi, phát hiện thắng) + 10đ advanced tiles (ô dễ vỡ, cầu, 3 loại công tắc soft/heavy/split, tách/gộp khối, cầu toggle và permanent). Có GUI, có nút New Game, Restart và nút gọi từng solver |
| 3.2 BFS | 10 | Ghi **Search Time, Memory Usage, Expanded Nodes, Solution Length** (áp dụng cho mọi solver) |
| 3.3 DFS | 10 | Chống lặp trạng thái (visited/hashing). Được thay bằng IDS nếu ghi rõ |
| 3.4 UCS | 10 | **Định nghĩa và lý giải hàm chi phí.** Chi phí đều bằng 1 thì UCS ≡ BFS, nên cần chi phí **không đồng nhất** |
| 3.5 A\* | 10 | Heuristic phù hợp + chứng minh **admissible / consistent theo đúng hàm chi phí đã chọn** |
| 3.6 Report | 30 | Mô tả thuật toán (10) + thực nghiệm có bảng, biểu đồ, nhận xét (20) + phân công + tài liệu tham khảo |
| 3.7 Video | 10 | Demo GUI + chơi tay + chạy solver |

### 1.2. Kiến trúc: hai nửa nói chuyện qua dòng lệnh

Theo buổi Q&A với giảng viên (ghi trong `Docs/report0.md` mục 1.1): GUI phải là **3D**, được viết bằng ngôn ngữ khác, nhưng **thuật toán phải bằng Python**. Vì vậy hệ thống gồm hai nửa độc lập:

```
┌──────────────────────── code/  (C++ / OpenGL 3.3) ────────────────────────┐
│  Game 3D: vẽ, nhận phím, luật chơi bản C++, menu ImGui, phát lại lời giải  │
│                                                                            │
│  Bấm nút "A*" ──► run_solver() ──► chạy lệnh:                              │
│        python ../main.py data/1.data --algo astar --out solution.txt       │
└──────────────────────────────────┬─────────────────────────────────────────┘
                                   │ (tiến trình con, mã thoát 0..4)
┌──────────────────────────────────▼──────── Python ────────────────────────┐
│  main.py  ──► ai/level.py   (đọc file .data)                               │
│           ──► ai/problem.py (luật game bản Python = mô hình bài toán)      │
│           ──► ai/solvers/*  (BFS / DFS / UCS / A*)                         │
│           ──► kiểm tra lời giải bằng cách phát lại theo luật               │
│           ──► ghi "R U U R ..." vào solution.txt                           │
└──────────────────────────────────┬─────────────────────────────────────────┘
                                   │
   code/ đọc solution.txt ──► AutoPlayer gửi từng phím ──► khối tự lăn tới đích
```

**Hệ quả quan trọng nhất: luật game nằm ở HAI nơi.** Bản C++ (`code/game.cpp`, `tile.cpp`, `superblock.cpp`) dùng để chơi và vẽ. Bản Python (`ai/problem.py`, `ai/level.py`) dùng để tìm kiếm. Nếu hai bản lệch nhau, solver tìm ra lời giải "đúng trong Python" nhưng khối rơi trong game. Quy tắc nhóm: sửa luật thì sửa **cả hai**, thêm test vào `tests/test_problem.py`, rồi phát lại bằng `./a.out <n> --play <file>`.

### 1.3. Đối chiếu yêu cầu đề ↔ file ↔ trạng thái

| Yêu cầu (mục đề) | File đáp ứng | Trạng thái |
|---|---|---|
| GUI 3D, chơi được (3.1) | `code/*` | ✅ |
| Core: sàn/ô trống/lỗ đích, 3 tư thế, 4 hướng, rơi, thắng (3.1) | C++: `superblock.cpp`, `game.cpp`, `tile.cpp` · Python: `ai/problem.py` | ✅ (test: `tests/test_problem.py`) |
| Fragile (2.5) | `Game::check` · `BloxorzProblem.check` | ✅ |
| Soft switch (2.5) | "công tắc thường", ô loại 3 | ✅ |
| Heavy switch (2.5) | ô loại 4, chỉ kích hoạt khi đứng | ✅ |
| Cầu **toggle** (2.5) | `Floor::toggle` · `bridge_mask` | ✅ |
| Cầu **permanent**, một công tắc → **nhiều** ô cầu (2.5) | — | ⏳ chưa có (game nền chỉ có toggle, 1 công tắc ↔ 1 ô) |
| **Split switch**, tách/gộp 2 khối lập phương (2.5) | — | ⏳ chưa có |
| Định dạng màn text/JSON dùng chung cho mọi solver (3.1 F.Y.I) | `code/data/*.data`, đọc bởi `Floor::initialize` và `Level.from_text` | ✅ (mới có 2 màn, Q&A yêu cầu ≥ 10) |
| Trạng thái chứa cấu hình cầu (3.1 F.Y.I) | `State.bridge_mask` | ✅ |
| Nút New Game / Restart / Solve (3.1) | `code/hud.cpp` (menu chọn màn, Retry, 4 nút solver) | ✅ |
| BFS / DFS / UCS / A\* (3.2–3.5) | `ai/solvers/*_solver.py` | ⏳ khung có sẵn, thân `solve()` là TODO |
| Heuristic (3.5) | `ai/heuristics.py` | ⏳ `manhattan_heuristic` TODO |
| Hàm chi phí không đồng nhất (3.4) | `BloxorzProblem(cost_fn=...)` | ⏳ có chỗ cắm, chưa có hàm, CLI chưa chọn được (mục 7.3.6) |
| Ghi 4 chỉ số (3.2) | `ai/solvers/base_solver.py` (`SearchResult`, `BaseSolver.run`) | ✅ khung đo đã có |

---

## 2. Luật chơi và quy ước tọa độ

Phần này cần nắm **trước** khi đọc code. Phần lớn lỗi khi viết solver đến từ việc nhầm trục.

### 2.1. Tọa độ: `x` là DÒNG, `y` là CỘT

Code giữ nguyên quy ước của game C++ gốc:

- `x` = chỉ số **dòng** trong file `.data`, `y` = chỉ số **cột**.
- Hướng đi: **`R`: x+1**, **`L`: x−1**, **`U`: y+1**, **`D`: y−1**.
- Trong cửa sổ 3D, `R` trông như sang phải. Nhưng **khi nhìn file text, `R` là đi XUỐNG một dòng, `U` là sang PHẢI một cột.**

Ba tư thế (biến `along` trong C++, `Orientation` trong Python), lấy ô "neo" `(x, y)` là ô có chỉ số nhỏ nhất:

| Code | Đề (mục 2.2) | Ô bị chiếm |
|---|---|---|
| `Z` | Standing | `(x, y)` |
| `X` | Lying, dọc theo trục x (= "vertically" nếu nhìn file text) | `(x, y)` và `(x+1, y)` |
| `Y` | Lying, dọc theo trục y (= "horizontally" nếu nhìn file text) | `(x, y)` và `(x, y+1)` |

Vì ô neo luôn là ô nhỏ nhất, mỗi vị trí vật lý của khối có **đúng một** cách biểu diễn. Nhờ vậy không có hai trạng thái trùng nhau mà khác tên.

### 2.2. Bảng chuyển trạng thái (đề nhấn mạnh ở mục 3.1 F.Y.I)

Đề yêu cầu chú ý "orientation transition table": mỗi cặp (tư thế, hướng) cho ra đúng một (tư thế mới, vị trí mới). Bảng trong code (`TRANSITIONS` ở `ai/problem.py`, sao từ `SuperBlock::update` ở `code/superblock.cpp`):

| Tư thế hiện tại | L | R | U | D |
|---|---|---|---|---|
| `Z (x,y)` đứng | `X (x−2, y)` | `X (x+1, y)` | `Y (x, y+1)` | `Y (x, y−2)` |
| `X (x,y)` nằm | `Z (x−1, y)` | `Z (x+2, y)` | `X (x, y+1)` | `X (x, y−1)` |
| `Y (x,y)` nằm | `Y (x−1, y)` | `Y (x+1, y)` | `Z (x, y+2)` | `Z (x, y−1)` |

Đọc theo ba kiểu chuyển động của đề (mục 2.3):

- **Đứng → đổ xuống:** nằm trên hai ô theo hướng lăn. Ví dụ `Z(4,4)` + `R` → `X(5,4)` chiếm `(5,4),(6,4)`.
- **Nằm dọc theo hướng lăn → dựng đứng:** ví dụ `X(4,4)` + `R` → `Z(6,4)`.
- **Nằm vuông góc với hướng lăn → lăn ngang, vẫn nằm, dịch 1 ô:** ví dụ `X(4,4)` + `U` → `X(4,5)`.

Hai tính chất của bảng (test `test_transition_table` kiểm tra cả hai):

1. Sau mỗi lần lăn, tập ô mới **không giao** tập ô cũ. Hệ quả: khối không bao giờ "đứng yên trên công tắc qua nhiều lượt"; công tắc bị đè cuối nước đi luôn là công tắc vừa được đè mới.
2. Mỗi ô mới cách một ô cũ **tối đa 2** (Manhattan). Đây là nền tảng của heuristic ở mục 7.4.

### 2.3. Loại ô và định dạng `.data`

| Số trong lưới | Hằng Python (`ai/level.py`) | Hằng C++ (`tile.cpp`) | Ý nghĩa |
|---:|---|---|---|
| 0 | `EMPTY` | `NULL` | Ô trống / lỗ. **Ô đích và ô cầu lúc đầu cũng phải là 0** |
| 1 | `NORMAL` | `normal` | Sàn thường |
| 2 | `FRAGILE` | `fragile` | Ô cam dễ vỡ: đứng lên thì thua, nằm thì an toàn |
| 3 | `NORMAL_SWITCH` | `normal_bridge` | Công tắc thường (= **soft switch** của đề): bị đè bởi bất kỳ phần nào của khối |
| 4 | `HEAVY_SWITCH` | `heavy_bridge` | Công tắc nặng (= **heavy switch**): chỉ kích hoạt khi khối **đứng** trên nó |

> ⚠️ Tên trong C++ dễ gây hiểu nhầm: `normal_bridge`/`heavy_bridge` là **loại ô công tắc**, không phải ô cầu. Ô cầu không có loại riêng. Nó là một ô 0 được công tắc "bật" thành sàn type 1.

Định dạng file (đọc theo token, cách nhau bởi khoảng trắng bất kỳ). Ví dụ `code/data/1.data`, có chú thích:

```
13 13                           ← n = 13 dòng, m = 13 cột
0 0 0 0 0 0 0 0 0 0 0 0 0       ← dòng x = 0
0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 1 1 1 3 0 0 0 0 0 0 0       ← x = 2: (2,5) là công tắc thường
0 0 1 1 1 1 0 0 0 0 0 0 0
0 0 1 1 1 1 0 0 0 0 0 0 0
0 0 1 1 1 1 0 1 1 1 0 0 0       ← x = 5: (5,6) = 0 là ô cầu (đóng lúc đầu)
0 0 0 0 0 1 1 1 1 1 1 0 0       ← x = 6: (6,4) = 0 là ô cầu
0 0 0 0 0 1 1 0 1 1 1 0 0       ← x = 7: (7,7) = 0 là lỗ đích
0 0 0 0 0 1 1 1 1 1 1 0 0
0 0 0 0 0 1 1 1 2 1 1 0 0       ← x = 9: (9,8) là ô cam
0 0 0 0 0 0 4 1 1 1 3 0 0       ← x = 10: (10,6) công tắc nặng, (10,10) công tắc thường
0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0
3 3 Z                           ← bắt đầu ở (3,3), tư thế Z (có thể bắt đầu nằm: X/Y)
7 7                             ← ô đích (7,7)
2                               ← có 2 công tắc thường:
2 5 6 4                         ←   công tắc tại (2,5) bật/tắt ô cầu (6,4)
10 10 9 11                      ←   công tắc tại (10,10) bật/tắt ô cầu (9,11)
1                               ← có 1 công tắc nặng:
10 6 5 6                        ←   công tắc tại (10,6) bật/tắt ô cầu (5,6)
```

`code/data/2.data` là màn 19×14, có 3 công tắc nặng và rất nhiều ô cam, lời giải tối ưu **68 bước**.

### 2.4. Thắng, thua, công tắc: thứ tự xử lý

Đây là chỗ tinh tế nhất. Python phải chép **đúng thứ tự** của C++:

1. **Lăn** theo bảng 2.2.
2. **Kiểm tra thắng/thua** (`Game::check` / `BloxorzProblem.check`):
   - Ô neo không có sàn (ngoài lưới, ô 0, cầu đang đóng): nếu không phải ô đích thì **thua**; nếu là ô đích và đang đứng thì **thắng**.
   - Đứng (`Z`) trên ô cam → **thua**.
   - Tư thế `X`/`Y` mà ô thứ hai không có sàn và không phải ô đích → **thua**.
   - **Thua được ưu tiên hơn thắng.** Nằm vắt qua lỗ đích thì **không thắng, không thua** (lỗ đích đóng vai trò như sàn khi khối nằm), đúng mục 2.4 của đề.
3. Nếu nước đi vừa thắng hoặc thua thì **dừng, KHÔNG kích hoạt công tắc.**
4. Ngược lại, kích hoạt công tắc **tuần tự**: ô neo là công tắc thường → đảo cầu; (X/Y) ô thứ hai là công tắc thường → đảo cầu; đứng trên công tắc nặng → đảo cầu.
5. **Kiểm tra lại**: nếu cầu ngay dưới khối vừa bị đóng thì khối rơi (thua). Trong C++, việc này xảy ra ở frame kế tiếp vì `Game::check` chạy mỗi frame. Python mô phỏng bằng cách gọi `check` thêm một lần.

---

## 3. Thư mục `ai/` — từng file

Package Python ≥ 3.10, **chỉ dùng thư viện chuẩn** (đề cho phép thư viện hỗ trợ nhưng thuật toán phải tự cài, mục 3).

### 3.1. `ai/__init__.py`

Gom các tên dùng nhiều để viết `from ai import BloxorzProblem, Level, State, ...`. Xuất: `ACTIONS`, `BloxorzProblem`, `Level`, `Orientation`, `Outcome`, `State`, `Switch`, `unit_cost`.

### 3.2. `ai/state.py` — biểu diễn trạng thái

| Thành phần | Ý nghĩa |
|---|---|
| `ACTIONS = ("U", "D", "L", "R")` | Thứ tự hành động **cố định**, nên BFS/DFS cho kết quả giống nhau mỗi lần chạy (tái lập được, quan trọng cho thực nghiệm công bằng) |
| `Orientation(str, Enum)` | `X`, `Y`, `Z`, trùng tên hằng trong `superblock.cpp` |
| `State(x, y, orientation, bridge_mask)` | `@dataclass(frozen=True, slots=True)` |
| `State.cells()` | Trả về 1 hoặc 2 ô mà khối đang chiếm |

**Vì sao `frozen=True`?** Trạng thái bất biến thì băm (hash) được, nên dùng làm khóa `dict`/`set`. Đây chính là cơ chế "visited set or hashing" mà đề yêu cầu ở mục 3.3. **Vì sao `slots=True`?** Mỗi object nhỏ hơn (đo được 64 byte/`State`), giúp tiết kiệm bộ nhớ khi BFS lưu hàng nghìn trạng thái.

**`bridge_mask` là gì?** Một số nguyên, mỗi bit ứng với **một công tắc**. Bit `i` = số lần công tắc `i` đã được kích hoạt, lấy mod 2. Đề mục 3.1 F.Y.I: *"the state must include the open/closed status of every bridge … states with the same block placement but different bridge configurations are different states"*. Ví dụ `State(1,2,Z,0)` và `State(1,2,Z,1)` là hai trạng thái khác nhau (test `test_bridge_state_is_part_of_state`). Nếu visited chỉ lưu vị trí khối, solver sẽ cắt nhầm các lời giải buộc phải "đi bấm công tắc rồi quay lại chỗ cũ".

### 3.3. `ai/level.py` — dữ liệu tĩnh của màn chơi

| Thành phần | Chức năng |
|---|---|
| Hằng `EMPTY, NORMAL, FRAGILE, NORMAL_SWITCH, HEAVY_SWITCH` (0–4) và `TILE_NAMES` | Loại ô (bảng 2.3) |
| `Switch(kind, pos, target, bit)` | Một công tắc: loại `"normal"`/`"heavy"`, vị trí, ô cầu nó điều khiển, chỉ số bit trong `bridge_mask` |
| `Level.__init__` | Lưu lưới dạng tuple (bất biến), điểm bắt đầu, đích. Gộp công tắc **giống `std::map` của C++** (khai báo trùng vị trí thì dòng sau ghi đè). Đánh số bit theo thứ tự: công tắc thường trước, công tắc nặng sau. Dựng `bridge_bits[ô cầu] = mask các công tắc điều khiển ô đó` |
| `Level.from_file` / `from_text` | Đọc file `.data` theo token, giống hệt `Floor::initialize`. Báo lỗi tiếng Việt khi file thiếu dữ liệu hoặc tư thế không phải X/Y/Z |
| `Level.validate()` | Bắt các màn mà game C++ sẽ xử lý sai: đích ngoài lưới hoặc khác 0, loại ô lạ, công tắc trỏ ra ngoài, ô công tắc không đúng loại, **ô cầu lúc đầu phải là 0** (vì C++ bật cầu luôn tạo ô type 1), ô cầu trùng ô đích, công tắc thiếu dòng ánh xạ |
| `in_bounds(x, y)` | Ô có nằm trong lưới không |
| `has_tile(x, y, mask)` | **Hàm quan trọng nhất**: ô có sàn không, với cấu hình cầu `mask`. Ngoài lưới = không có sàn |
| `tile_type(x, y, mask)` | Loại ô hiện tại (`EMPTY` nếu không có sàn; cầu đang mở được tính là `NORMAL`) |
| `switch_at(kind, x, y)` | Tra công tắc theo loại và vị trí |

**`has_tile` tính cầu bằng phép XOR chẵn lẻ.** `present = (ô gốc khác 0)`. Lấy `bits = bridge_bits[(x,y)]` (các công tắc điều khiển ô này). Nếu số bit bật của `mask & bits` là **lẻ** thì đảo `present`. Ví dụ màn 1: ô cầu `(6,4)` do công tắc bit 0 (tại `(2,5)`) điều khiển. Với `mask = 0` thì ô đóng; đè công tắc một lần → `mask = 1` → ô mở; đè lần nữa → `mask = 0` → đóng lại. Cách này cũng đúng khi **nhiều công tắc cùng điều khiển một ô** (test `test_bridge_vanishing_under_block_loses`).

### 3.4. `ai/problem.py` — mô hình bài toán (luật game bản Python)

| Thành phần | Chức năng |
|---|---|
| `TRANSITIONS` | Bảng 2.2 dưới dạng `dict`: `(tư thế, hướng) → (tư thế mới, dx, dy)` |
| `Outcome` | `CONTINUE` / `WON` / `LOST` (tương ứng `-1 / 1 / 0` của `Game::check`) |
| `CostFn` | Kiểu hàm chi phí `(state, action, next_state) -> float` |
| `unit_cost` | Hàm chi phí mặc định: mọi nước = 1 |
| `BloxorzProblem(level, cost_fn=unit_cost)` | Bài toán tìm kiếm. **`cost_fn` là chỗ cắm hàm chi phí không đồng nhất cho UCS/A\*** |
| `initial_state()` | `State(sx, sy, tư_thế_đầu, 0)`: lúc đầu mọi cầu đóng |
| `check(state)` | Bản sao `Game::check` (mục 2.4, bước 2) |
| `_toggle(mask, kind, x, y)` | `mask ^ (1 << bit)`: đảo bit của công tắc |
| `simulate(state, action)` | **Thực hiện một nước đi** theo đúng thứ tự mục 2.4: lăn → `check` → (nếu `CONTINUE`) kích hoạt công tắc tuần tự trên mask đang thay đổi → nếu mask đổi thì `check` lại. Trả `(trạng thái mới, Outcome)` |
| `actions(state)` | Các hướng **không làm thua**, theo thứ tự U, D, L, R |
| `result(state, action)` | Trạng thái kế tiếp; ném `ValueError` nếu nước đó làm thua |
| `successors(state)` | **Hàm solver dùng chính:** danh sách `(action, next_state, cost)` của các nước hợp lệ. Gọi `simulate` một lần mỗi hướng (không tính hai lần như khi gọi `actions` rồi `result`) |
| `is_goal(state)` | `check(state) is WON`: đứng trên lỗ đích |
| `step_cost(s, a, s2)` | Gọi `cost_fn` |
| `replay(moves)` | Phát lại chuỗi nước đi từ đầu, dừng ngay khi thắng/thua (giống game). Nhận list hoặc chuỗi `"R U U R"` |
| `is_valid_solution(moves)` | `True` nếu thắng **đúng ở nước cuối cùng** (không thắng sớm, không thua giữa chừng) |
| `path_cost(moves)` | Tổng chi phí của chuỗi nước đi theo `cost_fn` |

**Nước đi "illegal" của đề được xử lý thế nào?** Đề mục 2.3: nước đi làm khối chạm ô trống hoặc ra ngoài là **illegal**. Trong code, đó là các nước có `Outcome.LOST`. `successors()` và `actions()` **lọc bỏ** chúng, nên solver không bao giờ sinh ra trạng thái "đã rơi".

**Ví dụ dò tay: lời giải tối ưu màn 1 `R U U R U R U R D L`** (lấy từ `simulate` thật):

| # | Nước | Trước | Sau | Ô chiếm sau nước đi | Ghi chú |
|---:|---|---|---|---|---|
| 1 | R | `(3,3,Z)` | `(4,3,X)` | (4,3),(5,3) | đứng → đổ xuống |
| 2 | U | `(4,3,X)` | `(4,4,X)` | (4,4),(5,4) | lăn ngang |
| 3 | U | `(4,4,X)` | `(4,5,X)` | (4,5),(5,5) | lăn ngang |
| 4 | R | `(4,5,X)` | `(6,5,Z)` | (6,5) | dựng đứng |
| 5 | U | `(6,5,Z)` | `(6,6,Y)` | (6,6),(6,7) | đổ xuống |
| 6 | R | `(6,6,Y)` | `(7,6,Y)` | (7,6),**(7,7)** | nằm vắt qua lỗ đích: không thắng, không thua |
| 7 | U | `(7,6,Y)` | `(7,8,Z)` | (7,8) | dựng đứng |
| 8 | R | `(7,8,Z)` | `(8,8,X)` | (8,8),**(9,8)** | nằm trên ô cam: an toàn |
| 9 | D | `(8,8,X)` | `(8,7,X)` | (8,7),(9,7) | lăn ngang |
| 10 | L | `(8,7,X)` | `(7,7,Z)` | (7,7) | dựng đứng trên lỗ đích → **WON** |

Lời giải này không đụng công tắc nào nên `bridge_mask` luôn bằng 0.

### 3.5. `ai/heuristics.py`

| Hàm | Trạng thái | Ý nghĩa |
|---|---|---|
| `zero_heuristic` | ✅ có sẵn | `h = 0`: luôn admissible. **A\* với h = 0 chính là UCS**, dùng để đối chứng |
| `manhattan_heuristic` | ⏳ TODO (TV5) | Docstring đã có gợi ý. Thiết kế và chứng minh ở mục 7.4 |
| `HEURISTICS` | ✅ | Bảng tên → hàm, để `main.py --heuristic zero|manhattan` chọn |

### 3.6. `ai/solvers/__init__.py`

`SOLVERS = {"bfs": BFSSolver, "dfs": DFSSolver, "ucs": UCSSolver, "astar": AStarSolver}`. `main.py` và test tạo solver theo tên qua bảng này. Thêm solver mới (ví dụ IDS) thì đăng ký ở đây.

### 3.7. `ai/solvers/base_solver.py` — khung chung và 4 chỉ số đo

**`SearchResult`**: kết quả một lần chạy, đủ 4 chỉ số đề yêu cầu (mục 3.2) cộng thêm vài chỉ số phụ:

| Trường | Chỉ số đề | Ý nghĩa |
|---|---|---|
| `time_ms` | **Search Time** | Thời gian chạy `solve()` (ms), đo bằng `time.perf_counter()` |
| `peak_memory_kb` | **Memory Usage** | Bộ nhớ đỉnh (KB) đo bằng `tracemalloc`. `None` nếu không bật đo (giải thích kỹ ở mục 7.1.4) |
| `nodes_expanded` | **Expanded Nodes** | Số node đã lấy ra khỏi frontier **và sinh con**. Node "cũ" bị bỏ qua trong hàng đợi ưu tiên không được tính |
| `solution_length` (property) | **Solution Length** | `len(path)`: số nước đi |
| `path` | | Danh sách `"U/D/L/R"` hoặc `None` |
| `cost` | | Tổng chi phí theo `cost_fn`. Khác `solution_length` khi chi phí không đồng nhất |
| `nodes_generated` | | Số node con đã sinh |
| `max_frontier` | | Kích thước frontier lớn nhất (một cách nhìn khác về bộ nhớ) |
| `status` | | `"ok"`, `"no_solution"`, `"not_implemented"` (`main.py` có thể đặt thêm `"invalid"`) |
| `extra` | | Chỗ chứa thông tin phụ (ví dụ thông báo lỗi) |

**`BaseSolver`**: lớp trừu tượng. Lớp con chỉ cần cài `solve()`.

| Phương thức | Chức năng |
|---|---|
| `reset_counters()` | Đặt `nodes_expanded`, `nodes_generated`, `max_frontier` về 0 |
| `solve()` (abstract) | Thuật toán tìm kiếm. Lớp con cập nhật các bộ đếm rồi `return self.make_result(path)` |
| `run(measure_memory=False)` | Bọc `solve()`: reset bộ đếm → (bật `tracemalloc` nếu cần) → bấm giờ → gọi `solve()` → ghi `time_ms`, `peak_memory_kb`. Dùng `try/finally` để luôn tắt `tracemalloc`, kể cả khi solver ném lỗi |
| `make_result(path)` | Đóng gói `SearchResult` từ path và bộ đếm. Tự tính `cost` bằng `problem.path_cost(path)` |
| `reconstruct_path(parent, state)` | Truy vết từ trạng thái đích về gốc qua `parent[state] = (trạng_thái_cha, hành_động)`, đảo ngược lại thành danh sách hành động |

**Vì sao dùng `parent` dict thay vì lưu cả đường đi trong mỗi node?** Lưu cả path trong mỗi node tốn O(độ sâu) bộ nhớ cho từng node. `parent` chỉ tốn một cặp `(cha, hành động)` cho mỗi trạng thái, đồng thời kiêm luôn vai trò tập visited.

### 3.8. `ai/solvers/bfs_solver.py`, `dfs_solver.py`, `ucs_solver.py`, `astar_solver.py`

Bốn file có cùng cấu trúc: lớp kế thừa `BaseSolver`, có `name`, docstring gợi ý cách cài, và `solve()` đang `raise NotImplementedError` (mỗi thành viên tự cài: TV1 BFS, TV2 DFS, TV3 UCS, TV4 A\*). `AStarSolver` nhận thêm tham số `heuristic` (mặc định `manhattan_heuristic`). Cách các thuật toán chạy được giải thích ở mục 7.

---

## 4. Các file Python xung quanh: `main.py`, `tests/`

### 4.1. `main.py` — CLI, cầu nối với game

```bash
python main.py code/data/1.data --algo bfs                  # chạy 1 solver
python main.py code/data/2.data --algo astar --out sol.txt  # ghi lời giải ra file (game dùng cách này)
python main.py code/data/1.data --compare --memory          # chạy cả 4, in bảng, đo thêm bộ nhớ
python main.py code/data/1.data --algo astar --heuristic zero
```

| Hàm | Chức năng |
|---|---|
| `make_solver(name, problem, heuristic)` | Tạo solver theo tên; riêng A\* thì truyền thêm heuristic |
| `run_one(...)` | Chạy solver một lượt để đo thời gian. Nếu `--memory` thì chạy **lượt thứ hai riêng** để đo bộ nhớ (vì `tracemalloc` làm chậm, đo chung sẽ sai thời gian). Bắt `NotImplementedError` → `status = "not_implemented"`. **Phát lại lời giải theo luật** (`is_valid_solution`); sai luật → `status = "invalid"` |
| `describe(result)` | In kết quả một solver (số bước, chi phí, node, frontier, thời gian, bộ nhớ, lời giải) |
| `print_table(results)` | Bảng so sánh cho `--compare` |
| `main(argv)` | Đặt UTF-8 cho console Windows, đọc tham số, đọc màn, chạy, ghi `--out` (chỉ khi có lời giải) |

**Mã thoát** (game C++ đọc mã này để hiện thông báo): `0` có lời giải · `1` vô nghiệm · `2` solver chưa cài · `3` lời giải sai luật · `4` lỗi đọc màn.

> ⚠️ Hiện `main.py` luôn tạo `BloxorzProblem(level)` với **chi phí mặc định = 1**. Khi TV3 có hàm chi phí không đồng nhất, cần thêm tham số (ví dụ `--cost`) để UCS/A\* dùng được hàm đó từ CLI và từ nút trong game (mục 7.3.6).

### 4.2. `tests/` — kiểm thử bằng pytest

Chạy: `python -m pytest -q` → hiện tại **36 passed, 10 skipped** (10 test solver bị SKIP vì solver chưa cài).

| File | Nội dung |
|---|---|
| `tests/helpers.py` | `SOLUTIONS` (lời giải đã kiểm chứng trong game thật: màn 1, 10 bước; màn 2, 68 bước), `OPTIMAL_LENGTH = {1: 10, 2: 68}`, `load_problem(n)`, `make_level(rows, ...)` để tạo màn nhỏ từ danh sách chuỗi ngay trong test |
| `tests/test_problem.py` | Kiểm tra **luật Python khớp C++**: đọc màn 1, 2; validate báo lỗi; **12 ô của bảng chuyển** + tính chất "không giao, chạm cạnh"; lời giải đã biết thắng đúng ở nước cuối; nước thắng không kích hoạt công tắc; rơi khỏi lưới; ra ngoài biên; đứng trên ô cam thua, nằm an toàn; thắng khi đứng trên đích; nằm vắt qua đích không thắng/thua; công tắc thường đảo cầu khi nằm đè; công tắc nặng chỉ kích hoạt khi đứng; cầu biến mất dưới khối → thua; `bridge_mask` là một phần của trạng thái |
| `tests/test_solvers.py` | Mỗi solver × mỗi màn: tìm được lời giải, **phát lại hợp lệ**, `nodes_expanded > 0`. BFS/UCS/A\*/A\*(h=0) phải **tối ưu** (10 và 68 bước). Kiểm tra khung `BaseSolver.run` đo được thời gian + bộ nhớ, `reconstruct_path`, CLI báo "chưa cài" mà không crash, `--compare` không crash, file màn hỏng → mã 4 |

### 4.3. Các file cấu hình

- `pyproject.toml`: tên dự án, `requires-python >= 3.10`, cấu hình pytest (`pythonpath = ["."]`, `testpaths = ["tests"]`).
- `requirements.txt`: chỉ có `pytest>=7`, vì phần AI không cần thư viện ngoài.

---

## 5. Thư mục `code/` — từng file

Game 3D C++/OpenGL 3.3, gốc từ repo `chaitanya100100/Bloxorz` @ `c0ccb55` (ghi nguồn trong `README.md`, `THIRD_PARTY_NOTICES.md`). Nhóm đã thêm: phát lại lời giải, gọi solver Python, kiểm tra biên lưới, giao diện mới, menu ImGui, đóng gói. File chia thành 5 nhóm.

### 5.1. Nhóm LUẬT (phải khớp với `ai/`)

#### `game.h` / `game.cpp` — lớp `Game`: thắng, thua, công tắc

| Thành phần | Chức năng | Bản Python tương ứng |
|---|---|---|
| `gameover`, `won`, `level`, `number_of_steps` | Trạng thái ván | |
| `initialize(level)` | Đặt lại ván, nạp `data/<level>.data` vào `stage`, đặt khối ở vị trí đầu. Gọi lại được (Restart, đổi màn) | `BloxorzProblem.initial_state` |
| `check()` | Thắng/thua (mục 2.4, bước 2). **Chạy mỗi frame** trong `update_game` (main.cpp). Trả `0` thua, `1` thắng, `-1` tiếp tục | `BloxorzProblem.check` |
| `check_after_step()` | Gọi **một lần sau mỗi nước lăn xong**: tăng số bước, `check()`; nếu chưa thắng/thua thì kích hoạt công tắc tuần tự | phần sau `check` trong `simulate` |
| `gameover_fun()`, `won_fun()` | In `GAME OVER` / `WON` và số bước ra console. Chế độ `--play`/`--solve` dùng dòng này để kiểm chứng | |
| `get_level()`, `get_steps()` | HUD hiển thị | |

#### `tile.h` / `tile.cpp` — `Tile` và `Floor` (mặt sàn)

- **`Tile`**: một ô, chỉ chứa `tile_type` (1–4). Ô trống là con trỏ `NULL`.
- **`Floor`** (biến toàn cục `stage`): toàn bộ màn chơi.

| Thành phần | Chức năng |
|---|---|
| `tiles[n][m]`, `n`, `m`, `sx, sy, along`, `tx, ty` | Lưới, điểm bắt đầu, tư thế đầu, ô đích |
| `normal_bridge`, `heavy_bridge` (`map<int,int>`) | Ánh xạ `x*m+y` của công tắc → `a*m+b` của ô cầu. **Mỗi công tắc đúng 1 ô cầu** (giới hạn của game nền) |
| `initialize(filename)` | Đọc file `.data` (định dạng 2.3). Đặt biên phép chiếu song song `leftt/rightt/topp/bottomm` theo kích thước màn |
| `has_tile(i, j)` | Có sàn không. **Nhóm thêm kiểm tra biên**: bản gốc truy cập ngoài mảng khi khối ra mép lưới |
| `toggle(a, b)` | **Luật cầu**: ô trống → tạo `Tile(1)`; có sàn → xóa. Phần nhóm thêm chỉ ghi thời điểm để vẽ hiệu ứng |
| `normal_bridge_toggle`, `heavy_bridge_toggle` | Tra map rồi gọi `toggle` |
| `draw(now)`, `draw_transparent(now)`, `tile_style`, `break_tile`, `make_ghost` | **Chỉ hình ảnh**: vẽ ô theo loại (màu cam, vòng công tắc, chữ X), khung đích phát sáng, cột sáng trên đích, ô cầu trồi lên, ô bị xóa rơi và mờ dần, ô cam vỡ khi thua. Không ảnh hưởng luật |
| `get_initial_pos`, `get_initial_index`, `get_along`, `get_tile_type` | Truy vấn cho `SuperBlock` và `Game` |

#### `superblock.h` / `superblock.cpp` — lớp `SuperBlock` (khối 1×1×2)

| Thành phần | Chức năng |
|---|---|
| `ix, iy, along` | **Vị trí logic** (ô neo + tư thế), dùng cho luật |
| `pos, real_pos, dim` | Vị trí và kích thước để vẽ, vị trí để camera bám theo |
| `move(key)` | Bắt đầu một lần lăn (đặt `rot_dir`). Bỏ qua nếu khối đang lăn |
| `update(dt)` | Tăng góc quay theo **thời gian thật** (`roll_speed = 450°/s`, bản gốc cộng 5° mỗi frame nên FPS thấp thì lăn chậm). Khi đủ 90°: cập nhật `ix, iy, along` theo **bảng chuyển 2.2**, rồi gọi `game.check_after_step()` |
| `draw(M)` | Ma trận quay quanh cạnh đáy có easing, hiệu ứng rơi khi mở màn, chìm vào lỗ khi thắng, lật và rơi khi thua |
| `draw_shadow`, `land_dust`, `start_sink`, `start_fall` | Hiệu ứng: bóng mềm, bụi khi chạm sàn, chìm/rơi |
| `is_moving()` | `AutoPlayer` dùng để chờ khối đứng yên mới gửi nước tiếp theo |

Hằng toàn cục: `X=0, Y=1, Z=2` (tư thế), `U=1, D=2, R=3, L=4, NO=0` (hướng lăn).

### 5.2. Nhóm CẦU NỐI AI

#### `autoplay.h` / `autoplay.cpp`

**`AutoPlayer`** (biến `autoplayer`): phát lại lời giải.

| Phương thức | Chức năng |
|---|---|
| `load(path)` | Đọc file lời giải: các ký tự `U/D/L/R`, cách nhau bởi khoảng trắng hoặc viết liền. Ký tự lạ → lỗi. Đường dẫn UTF-8 (mở được thư mục có dấu như "Cơ sở AI") |
| `start(now)`, `stop()` | Bắt đầu / dừng phát lại |
| `update(now)` | Gọi mỗi frame: nếu khối đang lăn thì chờ; nếu khối đã **đứng yên đủ `autoplay_delay = 0.25 s`** thì gửi phím của nước kế tiếp vào `superblock.move()` |
| `is_active`, `total`, `played` | HUD hiển thị "Replay 3/10" |

**`run_solver(level, algo, out_path, quiet)`**: gọi solver Python.

1. Chỉ chấp nhận `bfs|dfs|ucs|astar`, nên không chèn được lệnh shell qua tên thuật toán.
2. Đặt `PYTHONIOENCODING=utf-8` (output solver có tiếng Việt).
3. Chọn script: `solver/main.py` (bản đóng gói) nếu có, ngược lại `../main.py` (chạy trong repo).
4. Chọn trình thông dịch: biến môi trường `BLOXORZ_PYTHON`, mặc định `python3` (Linux/macOS) hoặc `python` rồi `py -3` (Windows). Mã 9009/127 (không tìm thấy lệnh) → thử cái tiếp theo.
5. Chạy lệnh và chờ xong. Chế độ `quiet` (GUI) ghi output vào `solver_log.txt` và trên Windows không mở cửa sổ console (`CreateProcessW` + `CREATE_NO_WINDOW`).
6. Đổi mã thoát thành `SolverStatus` (`0..4` trùng mã của `main.py`, `5` không có Python, `6` lỗi khác). `solver_status_text()` cho câu thông báo hiện trên HUD.

### 5.3. Nhóm ỨNG DỤNG (vòng lặp, phím, camera)

#### `main.cpp` — điểm vào và vòng lặp chính

- **Đối tượng toàn cục:** `stage` (Floor), `superblock`, `game`, `window`, `Matrices`.
- **Hai chế độ chạy:**
  - **CLI** (`--play <file>` hoặc `--solve <algo>`): kiểm chứng luật. Tự phát lại, in `WON`/`GAME OVER`, giữ màn hình 2 s rồi thoát với mã 0 (thắng) / 1 (thua).
  - **Tương tác** (không có hai tham số trên, ví dụ double-click `Bloxorz.exe`): có menu, Restart, Solve chạy nền, không tự thoát.
- **Tham số:** `[level]`, `--solve`, `--play`, và tùy chọn cho dev: `--no-vsync`, `--no-menu`, `--camera`, `--shot <giây> <file.png>`, `--quit-after`, `--help`.
- **Thao tác ứng dụng** (khai báo trong `app.h`, HUD và phím tắt dùng chung):

| Hàm | Chức năng |
|---|---|
| `available_levels()` | Quét `data/1.data … data/99.data` (2 giây/lần) để menu hiện danh sách màn. Thêm màn chỉ cần thả file vào `data/` |
| `app_input_locked()` | Khóa phím mũi tên khi đang phát lại, đang chạy solver, ván đã kết thúc, đang mở menu hoặc đang hiệu ứng mở màn |
| `app_start_level(lv)` | **New Game / chọn màn**: `game.initialize`, reset autoplay, hiệu ứng mở màn, tính lại phép chiếu và camera |
| `app_restart()`, `app_next_level()` | **Restart** (phím R / nút Retry) và sang màn kế |
| `app_solve(algo)` | Nút Solve: chạy `run_solver` trong **`std::thread` riêng** để cửa sổ không treo khi Python tìm kiếm lâu |
| `poll_solver()` | Mỗi frame kiểm tra luồng solver xong chưa. Thành công và màn chưa đổi → nạp `solution.txt`, chơi lại màn từ đầu, phát lại sau hiệu ứng mở màn. Thất bại → hiện thông báo lỗi |

- **Mỗi frame:** `update_game` (camera → hiệu ứng → autoplay → `superblock.update` → `game.check` → nếu kết thúc thì `on_game_end`: khối chìm vào lỗ kèm pháo sáng, hoặc ô cam vỡ và khối rơi), rồi `render_frame` (nền → cảnh 3D gồm lượt vẽ đục rồi lượt trong suốt → bản đồ nhỏ góc trái dưới → HUD).
- **Chi tiết nền tảng:** gợi ý chạy trên GPU rời cho laptop 2 GPU (`NvOptimusEnablement`); đọc lại tham số dòng lệnh dạng UTF-16 trên Windows; `chdir_to_exe_if_needed()` để chạy được từ shortcut.

#### `app.h`

Khai báo trạng thái cấp ứng dụng (`level`, `cli_mode`, `menu_open`, `game_result`, FPS, `solve_state`, nhãn replay…) và các hàm `app_*` ở trên.

#### `controls.h` / `controls.cpp` — bàn phím, chuột, phép chiếu

- `keyboard()`: **mũi tên lăn khối** (xử lý khi **nhả phím**, và chỉ khi `!app_input_locked()`); `C V B X N M` đổi camera; `Space` đổi phép chiếu phối cảnh ↔ song song; `Tab` bản đồ nhỏ; `F1` bảng phím; `F12` chụp màn hình; `R` chơi lại; `Enter` sang màn kế (thắng) hoặc chơi lại (thua); `Esc` mở/đóng menu (chế độ CLI: thoát); `A S W D T G`, `I J K L` di chuyển/xoay camera tự do. Khi menu ImGui đang nhận phím thì game nhường phím cho menu.
- `keyboardChar()`: `Q` thoát.
- `mouseButton()`, `scroll()`, `cursor_pos_callback()`: điều khiển camera bằng chuột.
- `reshapeWindow()`: tính lại phép chiếu phối cảnh (FOV 75°) và song song khi đổi kích thước cửa sổ. `ortho_fit()`: phép chiếu song song giữ đúng tỉ lệ, đặt màn chơi ở giữa.

#### `camera.h` / `camera.cpp` — 6 chế độ camera (từ bản gốc)

`C` follow (mặc định, bám theo khối mượt) · `V` nhìn thẳng từ trên (song song) · `B` tower (nhìn chéo từ một góc cao) · `X` helicopter (di chuyển và xoay tự do bằng phím) · `N` kéo chuột · `M` xoay theo vị trí chuột. Nhóm sửa để tốc độ camera **không phụ thuộc FPS** (nhân theo `dt`).

### 5.4. Nhóm HIỂN THỊ (thuần hình ảnh, không chứa luật)

| File | Chức năng |
|---|---|
| `render.h` / `render.cpp` | `Renderer`: **một mesh hộp đơn vị [0,1]³ dùng chung** cho mọi ô, khối và hạt. `draw_box()` chỉ ghi hộp vào lô; `flush()` gửi cả lô lên GPU và vẽ bằng **một lệnh instanced** (thay vì mỗi ô một lệnh như bản gốc). `BoxStyle` mô tả màu, hoa văn (`PAT_FRAGILE`, `PAT_SWITCH`, `PAT_HEAVY`, `PAT_SHADOW`), độ phát sáng, độ trong suốt. `draw_background()` vẽ nền gradient có sao |
| `shaders.h` | Mã GLSL **nhúng thẳng** vào chương trình (bản đóng gói không cần file shader đi kèm): shader hộp có ánh sáng, viền tối và gờ sáng ở mép, hoa văn theo loại ô, sương mù theo khoảng cách; shader nền có sao lấp lánh |
| `colors.h` / `colors.cpp` | Bảng màu: màu cơ bản của bản gốc + bảng màu nền tối (`FLOOR_COLOR`, `BRIDGE_COLOR`, `FRAGILE_COLOR`, `SWITCH_ACCENT`, `HEAVY_ACCENT`, `BLOCK_COLOR`, `GOAL_COLOR`, `DUST_COLOR`). `col(id)` đổi sang `glm::vec3` |
| `effects.h` / `effects.cpp` | `Effects`: hiệu ứng mở màn (ô rơi xuống lần lượt, khối rơi từ trên cao), bụi khi khối chạm sàn, pháo sáng khi thắng; các hàm easing. Tắt được trong menu (máy yếu) |
| `hud.h` / `hud.cpp` | Giao diện 2D bằng **Dear ImGui** vẽ đè lên cảnh 3D: bảng thông số góc trên trái (LEVEL, MOVES, MODE: Manual / Solving / Replay x/y, FPS, GPU), thanh gợi ý phím, thông báo solver ("Solving level 1 with A\*…" hoặc lỗi), bảng kết thúc (LEVEL COMPLETE / GAME OVER + nút **Retry**, **Next**, **Menu**), **menu chính** (Resume, **SELECT LEVEL**, **AI SOLVER: BFS / DFS / UCS / A\***, tùy chọn Mini-map/Effects/Hints, Quit). Đây là phần đáp ứng yêu cầu "New Game, Restart, buttons for invoking the solvers" của đề |
| `screenshot.h` / `screenshot.cpp` | Ghi framebuffer ra **PNG** bằng bộ ghi PNG tối giản tự viết (zlib dạng "stored", CRC32, Adler-32), không cần thư viện ngoài. Dùng cho F12 và `--shot` (chụp ảnh cho báo cáo) |
| `noedit.h` / `noedit.cpp` | Tiện ích: `error_callback` của GLFW, `quit`, `utf8_fopen` / `read_text_file` (mở file theo đường dẫn UTF-8; trên Windows đổi sang UTF-16 và dùng `_wfopen`) |
| `headers.h` | Include chung (STL, GLAD, GLFW, GLM), struct `GLMatrices`, khai báo biến toàn cục `Matrices`, `proj_type`, `window` |

### 5.5. Build, đóng gói, thư viện bên thứ ba

| File / thư mục | Chức năng |
|---|---|
| `makefile` | Build trên Linux/WSL, macOS (Homebrew), Windows (MSYS2 MinGW64). `make` → `a.out`/`a.exe` (bản dev, có console); `make release` → `Bloxorz.exe` link tĩnh, không console, có icon; `make dist` → `../dist/<nền tảng>/Bloxorz/` (exe + `data/` + `solver/` chứa `main.py` và `ai/` + licenses) và file zip; `make clean`. Object để riêng theo nền tảng trong `build/<linux|win64>/` để build WSL và MSYS2 trên cùng thư mục không lẫn nhau. **Phải dùng tab** |
| `bloxorz.rc` | Tài nguyên Windows: icon cửa sổ (`GLFW_ICON`) và thông tin phiên bản của `Bloxorz.exe` |
| `assets/bloxorz.ico`, `assets/bloxorz_256.png` | Icon do nhóm tự sinh |
| `assets/make_icon.py` | Script Python (chỉ thư viện chuẩn) vẽ icon isometric "khối đỏ đứng trên sàn 3×3" và ghi file `.ico`/`.png` |
| `assets/README_dist.txt` | README đi kèm bản đóng gói (cách chơi, phím, cách dùng nút Solve) |
| `glad.c`, `include/glad/glad.h`, `include/KHR/khrplatform.h` | GLAD: bộ nạp hàm OpenGL 3.3 core (sinh bằng glad 0.1.36) |
| `third_party/imgui/` | Dear ImGui v1.91.9 (MIT): core + backend GLFW và OpenGL3, giữ nguyên bản gốc |

### 5.6. Dữ liệu và file sinh ra khi chạy

| File | Ý nghĩa |
|---|---|
| `data/1.data`, `data/2.data` | Hai màn chơi (mục 2.3), dùng chung cho game C++ và solver Python |
| `a.out`, `a.exe`, `bloxorz`, `Bloxorz.exe` | File chạy đã build (bị `.gitignore` bỏ qua) |
| `solution.txt` | Lời giải solver vừa ghi, game đọc để phát lại (gitignore) |
| `solver_log.txt` | Output của lần chạy solver gần nhất từ GUI (gitignore). Hiện chứa "[bfs] chưa cài" vì solver chưa có |
| `sol1.txt` | File lời giải màn 1 để thử `--play` (chưa commit) |
| `screenshot_*.png` | Ảnh chụp F12 (gitignore) |
| `readme.MD` | README của tác giả gốc (tính năng, phím, các chế độ camera) |

### 5.7. Ví dụ đầu-cuối: bấm nút "A\*" trên menu

```
hud.cpp main_menu: bấm "A*"                     → app_solve("astar")         (main.cpp)
  └─ std::thread: run_solver(1, "astar", "solution.txt", quiet=true)       (autoplay.cpp)
       └─ python ../main.py data/1.data --algo astar --out solution.txt
            main.py: Level.from_file → BloxorzProblem(level) → AStarSolver(...).run()
                     → is_valid_solution(path)? → ghi "R U U R ..." → exit 0
  └─ solver_done = true
mỗi frame: poll_solver() thấy SOLVER_OK      → autoplayer.load("solution.txt")
                                             → app_start_level(1)  (chơi lại từ đầu + hiệu ứng mở màn)
                                             → pending_autoplay = true
hết hiệu ứng mở màn                          → autoplayer.start()
mỗi frame: autoplayer.update()               → khối đứng yên ≥ 0.25 s → superblock.move(phím)
           superblock.update(dt)             → đủ 90°: cập nhật ix, iy, along → game.check_after_step()
           game.check()                      → 1 (thắng) → on_game_end: khối chìm + pháo sáng
hud.cpp end_panel                            → "LEVEL COMPLETE — 10 moves"  [Retry] [Next] [Menu]
```

Nếu solver chưa cài: `main.py` thoát mã 2 → HUD hiện "A\*: This algorithm is not implemented yet (TODO in ai/solvers)".

---

## 6. Hình thức hóa bài toán tìm kiếm

Đề mục 3.6 yêu cầu báo cáo mô tả: *state representation (including orientation and bridge configuration), actions, transition model, goal test, cost function, heuristics, and how you avoid revisiting states*. Ánh xạ sang code:

| Thành phần | Định nghĩa | Trong code |
|---|---|---|
| **Trạng thái** | `(x, y, orientation, bridge_mask)` | `State` |
| **Trạng thái đầu** | Ô và tư thế bắt đầu trong file, mọi cầu đóng (`mask = 0`) | `initial_state()` |
| **Hành động** | `{U, D, L, R}`, chỉ những hướng không làm rơi | `actions()`, `successors()` |
| **Mô hình chuyển** | Lăn theo bảng 2.2 → kiểm tra → công tắc → kiểm tra lại (mục 2.4) | `simulate()`, `result()` |
| **Kiểm tra đích** | Khối đứng (`Z`) trên lỗ đích | `is_goal()` |
| **Chi phí** | Mặc định 1/nước; UCS cần hàm không đồng nhất (mục 7.3) | `cost_fn`, `step_cost()` |
| **Heuristic** | Cận dưới chi phí còn lại (mục 7.4) | `ai/heuristics.py` |
| **Chống lặp** | Visited/parent dict theo `State` (băm được); với UCS/A\* là `best_g` | trong `solve()` của từng solver |

**Kích thước không gian trạng thái.** Với `T` ô có thể đứng và `k` công tắc: tối đa khoảng `3T · 2^k` trạng thái. Màn 1 (3 công tắc) có **1028** trạng thái đạt được từ trạng thái đầu (đếm bằng cách duyệt hết, không dừng ở đích). Số này nhỏ nên mọi thuật toán đều chạy gần như tức thời. Muốn thấy khác biệt rõ giữa các thuật toán trong thực nghiệm thì cần màn **lớn** và **nhiều công tắc**.

---

## 7. Các thuật toán tìm kiếm chạy như thế nào

### 7.0. Khung chung cho cả bốn thuật toán

Cả bốn đều là **tìm kiếm trên đồ thị** (graph search) và chỉ khác nhau ở **cấu trúc frontier**:

```
frontier ← { trạng thái đầu }
đã_thấy  ← { trạng thái đầu }              (parent dict: vừa chống lặp, vừa truy vết)
lặp khi frontier còn phần tử:
    s ← LẤY_RA(frontier)                    ← BFS: đầu hàng đợi; DFS: đỉnh ngăn xếp;
                                              UCS: g nhỏ nhất; A*: g + h nhỏ nhất
    nếu s là đích: truy vết parent → trả về đường đi
    nodes_expanded += 1
    với mỗi (a, s', cost) trong successors(s):
        nodes_generated += 1
        nếu s' chưa thấy (hoặc có g tốt hơn — UCS/A*): ghi parent[s'] = (s, a); ĐƯA_VÀO(frontier, s')
    cập nhật max_frontier
trả về "không có lời giải"
```

Ba khái niệm cần phân biệt khi viết báo cáo:

- **Sinh (generate):** tạo ra trạng thái con khi gọi `successors()`.
- **Mở rộng (expand):** lấy một trạng thái ra khỏi frontier và sinh con của nó. Đây là chỉ số "Expanded Nodes" của đề.
- **Thời điểm kiểm tra đích:** khi **sinh** (sớm hơn, vẫn tối ưu với BFS) hay khi **lấy ra** (bắt buộc với UCS/A\* để tối ưu). Các thành viên phải **thống nhất cách đếm** và ghi rõ trong báo cáo, nếu không bảng so sánh sẽ không công bằng.

### 7.1. BFS — Breadth-First Search (TV1, đề mục 3.2)

#### 7.1.1. Ý tưởng và mã giả

Duyệt **theo từng tầng**: xét hết các trạng thái cách gốc 1 nước, rồi 2 nước, rồi 3 nước… Frontier là **hàng đợi FIFO** (`collections.deque`).

```
BFS(problem):
    s0 ← initial_state;  parent ← {s0: None};  queue ← [s0]
    while queue:
        s ← queue.pop_left()
        if is_goal(s): return reconstruct(parent, s)
        expanded += 1
        for (a, s', _) in successors(s):
            if s' ∉ parent:                  ← đánh dấu đã thấy KHI ĐẨY VÀO, không phải khi lấy ra
                parent[s'] ← (s, a);  queue.push_right(s')
        max_frontier ← max(max_frontier, |queue|)
    return None
```

Đánh dấu khi đẩy vào hàng đợi giúp một trạng thái không bị đẩy vào nhiều lần (frontier không chứa trùng).

#### 7.1.2. Ví dụ: 4 tầng đầu của màn 1

| Tầng (số nước) | Các trạng thái mới | Giải thích |
|---:|---|---|
| 0 | `(3,3,Z,mask=0)` | xuất phát |
| 1 | `(3,4,Y,0)`, `(4,3,X,0)` | từ `(3,3,Z)` chỉ có U và R hợp lệ (L, D làm rơi) |
| 2 | `(2,4,Y,1)`, `(4,4,Y,0)`, `(4,4,X,0)`, `(4,2,X,0)` | `(2,4,Y)` nằm đè lên công tắc `(2,5)` → **mask = 1**, cầu `(6,4)` mở |
| 3 | `(2,3,Z,1)`, `(3,4,Y,1)`, `(4,3,Z,0)`, `(5,4,Y,0)`, `(4,5,X,0)`, `(3,4,Z,0)`, `(3,2,Z,0)` | `(3,4,Y,1)` trùng **vị trí** với `(3,4,Y,0)` ở tầng 1 nhưng **khác mask**, nên là trạng thái mới |

Số trạng thái mỗi tầng tăng dần (1, 2, 4, 7, 10, 16, 24, …). Lời giải nằm ở tầng 10. Vì BFS xét hết tầng 9 rồi mới tới tầng 10, lời giải đầu tiên tìm được chắc chắn **ngắn nhất về số nước**.

#### 7.1.3. Tính chất

- **Đầy đủ** (không gian hữu hạn) và **tối ưu theo số nước đi** (mọi nước chi phí bằng nhau).
- Thời gian và bộ nhớ `O(b^d)`, với `b ≤ 4` (hệ số nhánh) và `d` là độ dài lời giải. Thực tế bị chặn bởi số trạng thái `|S|`.

#### 7.1.4. "Memory usage trong BFS" là gì?

**Định nghĩa trong đồ án:** đề yêu cầu đo *Memory Usage* (mục 3.2) và ghi rõ là *peak memory* (mục 3.6). Đó là **lượng bộ nhớ lớn nhất mà thuật toán chiếm cùng một lúc trong khi tìm kiếm**, không phải tổng bộ nhớ đã từng cấp phát, cũng không phải bộ nhớ của cả chương trình Python.

**Đo bằng gì trong code:** `BaseSolver.run(measure_memory=True)` gọi `tracemalloc.start()` trước `solve()`, rồi lấy `tracemalloc.get_traced_memory()[1]`, tức **đỉnh** số byte Python cấp phát kể từ lúc bắt đầu đo. Kết quả được đổi sang KB và ghi vào `SearchResult.peak_memory_kb`. Vì đo từ lúc `solve()` bắt đầu, phần đã có sẵn (đối tượng `Level`, `BloxorzProblem`, trình thông dịch) **không bị tính**. Chỉ tính phần do thuật toán tạo ra.

**Với BFS, bộ nhớ đỉnh gồm những gì?**

| Thành phần | Kích thước |
|---|---|
| `parent` dict (kiêm tập visited) | Một mục cho **mỗi trạng thái đã thấy**, tăng suốt quá trình tìm và không bao giờ giảm |
| Hàng đợi `deque` (frontier) | Nhiều nhất bằng độ rộng của tầng lớn nhất (`max_frontier`) |
| Các object `State` | 64 byte mỗi object (nhờ `slots=True`), cộng chi phí của khóa dict |
| Danh sách tạm do `successors()` trả về | Nhỏ, sống ngắn |

**Vì sao BFS tốn bộ nhớ nhất trong các thuật toán "mù"?** BFS phải giữ **toàn bộ tầng đang xét** trong hàng đợi và **mọi trạng thái đã thấy** trong `parent`. Số trạng thái mỗi tầng tăng theo cấp số `b^d`, nên bộ nhớ cũng tăng theo `b^d`. DFS lý thuyết chỉ cần `O(b·m)` (một nhánh), nhưng khi có visited toàn cục thì cũng thành `O(|S|)`. Đây là điểm đáng thảo luận trong báo cáo.

**Độ lớn tham chiếu** (bản BFS cài thử ngoài repo, Python 3.12, kiểm tra đích khi lấy ra):

| Màn | Độ dài | Node mở rộng | Trạng thái đã thấy | Frontier lớn nhất | Bộ nhớ đỉnh |
|---|---:|---:|---:|---:|---:|
| 1 | 10 | 160 | 207 | 47 | ≈ 26 KB |
| 2 | 68 | 377 | 385 | 14 | ≈ 50 KB |

Màn 2 có lời giải dài hơn nhiều nhưng frontier lại nhỏ vì màn hẹp, gần như chỉ có một đường. Bộ nhớ chủ yếu do `parent` dict quyết định. Số liệu của nhóm sẽ hơi khác tùy cách cài.

**Lưu ý khi đo và báo cáo:**

- **Đo bộ nhớ ở lượt chạy riêng** (`main.py --memory` đã làm vậy). `tracemalloc` làm Python chậm đi nhiều, nên đo chung với thời gian sẽ sai số liệu thời gian.
- `max_frontier` là chỉ số **đếm phần tử**, còn `peak_memory_kb` là chỉ số **byte**. Có thể báo cáo cả hai: `max_frontier` không phụ thuộc phiên bản Python, `peak_memory_kb` đúng với yêu cầu đề.
- Chạy nhiều lần lấy trung vị cho thời gian; bộ nhớ đỉnh thường ổn định giữa các lần chạy.

### 7.2. DFS — Depth-First Search (TV2, đề mục 3.3)

#### 7.2.1. Ý tưởng và mã giả

Đi **sâu nhất có thể** theo một nhánh, hết đường mới quay lui. Frontier là **ngăn xếp LIFO**. Dùng **stack tường minh**, không đệ quy: giới hạn đệ quy mặc định của Python (~1000) sẽ bị vượt trên màn lớn.

```
DFS(problem):
    s0 ← initial_state;  parent ← {s0: None};  stack ← [s0]
    while stack:
        s ← stack.pop()
        if is_goal(s): return reconstruct(parent, s)
        expanded += 1
        for (a, s', _) in reversed(successors(s)):   ← đẩy NGƯỢC để lấy ra theo thứ tự U, D, L, R
            if s' ∉ parent:
                parent[s'] ← (s, a);  stack.push(s')
    return None
```

**Chống lặp (đề bắt buộc):** tập `parent`/visited theo `State`. Không có nó, DFS sẽ lăn qua lại giữa hai trạng thái mãi mãi (ví dụ `U` rồi `D` rồi `U`…).

#### 7.2.2. Tính chất

- **Đầy đủ** khi có visited (không gian hữu hạn). **Không tối ưu**: lời giải thường dài hơn BFS rất nhiều.
- **Thứ tự hành động ảnh hưởng mạnh** tới độ dài lời giải và số node. Phải cố định thứ tự (U, D, L, R) và ghi rõ trong báo cáo.
- **Bộ nhớ:** lý thuyết `O(b·m)` (m = độ sâu tối đa), nhưng visited toàn cục đẩy lên `O(|S|)`.
- **Phương án IDS** (đề cho phép, phải ghi rõ): chạy DFS giới hạn độ sâu 0, 1, 2, … Được tối ưu theo số nước (như BFS) với bộ nhớ nhỏ, đổi lại phải mở rộng lại các tầng nông nhiều lần. Chống lặp phải làm theo **đường đi hiện tại** hoặc lưu "độ sâu nhỏ nhất đã gặp". Dùng visited toàn cục kiểu DFS sẽ làm IDS **mất tính tối ưu**. Nếu cài IDS thì tạo `ai/solvers/ids_solver.py` và đăng ký trong `SOLVERS`.

### 7.3. UCS — Uniform-Cost Search (TV3, đề mục 3.4)

#### 7.3.1. Ý tưởng và mã giả

Luôn mở rộng trạng thái có **tổng chi phí đường đi `g` nhỏ nhất**. Frontier là **hàng đợi ưu tiên** (`heapq`).

```
UCS(problem):
    s0 ← initial_state;  best_g ← {s0: 0};  parent ← {s0: None}
    heap ← [(0, counter++, s0)]                ← counter để phá hòa, tránh so sánh hai State
    while heap:
        (g, _, s) ← heap.pop_min()
        if g > best_g[s]: continue              ← bản "cũ" đã có bản tốt hơn → bỏ qua, KHÔNG tính expanded
        if is_goal(s): return reconstruct(parent, s)     ← kiểm tra đích KHI LẤY RA
        expanded += 1
        for (a, s', c) in successors(s):
            g' ← g + c
            if g' < best_g.get(s', ∞):
                best_g[s'] ← g';  parent[s'] ← (s, a);  heap.push((g', counter++, s'))
    return None
```

Hai điểm bắt buộc để UCS đúng:

- **Kiểm tra đích khi lấy ra**, không phải khi sinh. Khi sinh, đích có thể đang ở `g` chưa tốt nhất; một đường rẻ hơn tới đích có thể vẫn còn nằm trong heap.
- **Lazy deletion:** `heapq` không hỗ trợ "giảm khóa". Khi tìm được `g` tốt hơn cho `s'`, ta đẩy bản mới vào heap và để bản cũ lại; lúc lấy bản cũ ra thì bỏ qua (`g > best_g[s]`).

**Tính chất:** đầy đủ và **tối ưu theo tổng chi phí** khi mọi chi phí ≥ ε > 0.

#### 7.3.2. "Hàm chi phí không đồng nhất để cho ra bộ trọng số thích hợp" nghĩa là gì?

**Hàm chi phí** `c(s, a, s')` gán cho **mỗi nước đi** một con số dương: "đi nước này tốn bao nhiêu". Trong code đó là `cost_fn` truyền vào `BloxorzProblem`. UCS (và A\*) tìm đường có **tổng** các con số này nhỏ nhất.

- **Đồng nhất (uniform):** mọi nước đều tốn 1 (`unit_cost`). Khi đó tổng chi phí = số nước đi, và đề nói rõ **UCS suy biến thành BFS** (mục 3.4): cùng lời giải, chỉ chậm hơn vì dùng heap.
- **Không đồng nhất (non-uniform):** các **loại** nước đi khác nhau có giá khác nhau. Ví dụ: lăn lên ô cam đắt hơn vì rủi ro; dựng khối đứng lên đắt hơn đổ xuống vì tốn sức hơn.
- **"Bộ trọng số"** là tập các con số gán cho từng loại nước đi, ví dụ `{đổ xuống: 1.118, lăn ngang: 1.207, dựng đứng: 1.618}` hoặc `{nước thường: 1, nước chạm ô cam: 3}`.
- **"Thích hợp"** nghĩa là bộ trọng số thỏa 4 điều kiện:
  1. **Dương thật sự** (≥ ε > 0). Chi phí 0 hoặc âm làm UCS/A\* mất tính đúng đắn hoặc lặp vô hạn.
  2. **Có lý giải** viết được trong báo cáo (đề: *"as long as you give a reasonable explanation"*): vật lý, rủi ro, sở thích người chơi…
  3. **Thực sự làm UCS khác BFS** trên bộ test. Nếu trọng số gần đều nhau, UCS vẫn ra đúng lời giải của BFS và phần này mất ý nghĩa (xem 7.3.4).
  4. **A\* dùng cùng hàm chi phí** và heuristic phải **admissible theo hàm chi phí đó** (đề mục 3.5: *"your heuristic must remain admissible with respect to your chosen cost function"*). Khi đó `cost(A*) = cost(UCS)` trên mọi màn, là một phép kiểm tra chéo tốt.

Theo Q&A (câu 10, `report0.md`), nhóm **được dùng ML để suy ra trọng số**, miễn trình bày rõ.

#### 7.3.3. Hai hướng thiết kế (chi tiết ở `report0.md` mục 5.3)

**(a) Theo năng lượng vật lý.** Muốn lăn khối quanh một cạnh đáy, phải nâng trọng tâm lên điểm cao nhất rồi mới đổ sang. Độ nâng trọng tâm `Δh`: đổ xuống ≈ 0.118, lăn ngang ≈ 0.207, dựng đứng ≈ 0.618. Đặt `cost = 1 + Δh`: **đổ xuống 1.118, lăn ngang 1.207, dựng đứng 1.618.** Số 1 đại diện cho "một thao tác của người chơi", phần `Δh` đại diện cho công vật lý.

**(b) Theo rủi ro / tình huống.** Ví dụ: nước đi khiến khối chạm ô cam `+2` (rủi ro làm vỡ nếu thao tác sai); nước kích hoạt công tắc rẻ hơn nhưng **vẫn > 0**; về sau có split thì di chuyển khi đang tách đắt hơn.

#### 7.3.4. Phân tích quan trọng: vì sao bộ trọng số năng lượng gần như không đổi lời giải

Mỗi nước đi thuộc một trong ba loại: **đổ** (Z → nằm), **dựng** (nằm → Z), **lăn ngang** (nằm → nằm). Trên một đường đi từ trạng thái **đứng** tới đích **đứng**, mỗi lần đổ phải được "trả" bằng đúng một lần dựng. Vậy **số lần đổ = số lần dựng = k**. Gọi `r` là số lần lăn ngang, `L = 2k + r` là độ dài:

```
chi_phí = 1.118·k + 1.618·k + 1.207·r = 1.207·L + 0.322·k
```

Nghĩa là UCS năng lượng **gần như chỉ tối ưu độ dài**, chỉ khác ở chỗ ưu tiên ít cặp đổ–dựng khi hòa. Muốn một đường **dài hơn 1 nước** thắng, nó phải có **ít hơn ít nhất 4 cặp đổ–dựng** (vì 1.207 / 0.322 ≈ 3.75). Rất hiếm gặp:

- Trên màn 1 và màn 2, UCS năng lượng cho **đúng lời giải của BFS** (10 và 68 bước, chi phí 13.036 và 88.516).
- Thử 40.000 màn ngẫu nhiên nhỏ (lưới tới 5×7), không màn nào UCS năng lượng chọn đường khác BFS.

**Kết luận cho TV3:** nếu dùng (a) thì cần (i) tăng độ chênh giữa các trọng số (ví dụ `λ` lớn trong `1 + λ·Δh`), và/hoặc (ii) thiết kế màn có hai đường mà một đường nhiều cặp đổ–dựng hơn hẳn, và/hoặc (iii) kết hợp với (b). Trình bày phép phân tích này trong báo cáo là một điểm cộng: nó cho thấy nhóm hiểu **vì sao** kết quả UCS giống hay khác BFS.

(Nếu bắt đầu ở tư thế nằm thì số lần dựng = số lần đổ + 1. Lập luận không đổi.)

#### 7.3.5. Ví dụ dò tay: chi phí phạt ô cam làm UCS khác BFS

Màn đồ chơi 3×4 (dựng bằng `make_level` trong test). Bắt đầu `Z` tại `(2,0)`, đích `(2,2)`:

```
        y=0 y=1 y=2 y=3
x=0      1   1   1   1
x=1      1   2   2   1        2 = ô cam
x=2      S   1   ◎   1        S = xuất phát (2,0), ◎ = lỗ đích (2,2)
```

Hàm chi phí: `c = 1`, cộng thêm `2` nếu **sau nước đi** có ô nào của khối nằm trên ô cam.

| Thuật toán | Lời giải | Số nước | Tổng chi phí (phạt ô cam) |
|---|---|---:|---:|
| BFS (tối ưu số nước) | `L U U R` | **4** | 1 + 3 + 3 + 1 = 8 |
| UCS (tối ưu chi phí) | `U U L D R` | 5 | 1 + 1 + 1 + 3 + 1 = **7** |

Dò từng nước (lấy từ `simulate` thật):

```
BFS:  L: (2,0,Z) → (0,0,X) chiếm (0,0),(1,0)          chi phí 1
      U: (0,0,X) → (0,1,X) chiếm (0,1),(1,1)=cam      chi phí 3
      U: (0,1,X) → (0,2,X) chiếm (0,2),(1,2)=cam      chi phí 3
      R: (0,2,X) → (2,2,Z)  WON                       chi phí 1      tổng 8
UCS:  U: (2,0,Z) → (2,1,Y) chiếm (2,1),(2,2)=lỗ đích  chi phí 1  (nằm vắt qua đích: hợp lệ)
      U: (2,1,Y) → (2,3,Z)                            chi phí 1
      L: (2,3,Z) → (0,3,X) chiếm (0,3),(1,3)          chi phí 1
      D: (0,3,X) → (0,2,X) chiếm (0,2),(1,2)=cam      chi phí 3
      R: (0,2,X) → (2,2,Z)  WON                       chi phí 1      tổng 7
```

Diễn biến hàng đợi ưu tiên của UCS (vài bước đầu; `g` = chi phí tích lũy; hòa thì phá theo thứ tự đẩy vào):

| Lần lấy ra | Trạng thái lấy ra (g) | Sinh con (g mới) | Ghi chú |
|---:|---|---|---|
| 1 | `(2,0,Z)` (0) | `(2,1,Y)` (1), `(0,0,X)` (1) | |
| 2 | `(2,1,Y)` (1) | `(2,3,Z)` (2), `(1,1,Y)` (4) | `(1,1,Y)` chạm ô cam nên g tăng 3 |
| 3 | `(0,0,X)` (1) | `(0,1,X)` (4) | BFS đi tiếp nhánh này; UCS thấy nó đắt nên để sau |
| 4 | `(2,3,Z)` (2) | `(0,3,X)` (3) | nhánh "vòng phải" rẻ hơn |
| 5 | `(0,3,X)` (3) | `(0,2,X)` (6) | |
| … | các trạng thái g = 4, 5, 6 | | `(0,2,X)` được tới với g = 6 (qua `(0,1,X)` thì g = 7, không tốt hơn) |
| cuối | `(2,2,Z)` (7) | — | lấy ra thì là đích → dừng, chi phí 7 |

Bài học: BFS chỉ "nhìn" số nước nên đi thẳng qua 2 lần chạm ô cam. UCS chấp nhận thêm 1 nước để chỉ chạm ô cam 1 lần. Đây chính là điều đề muốn thấy khi nói *"rolling onto a fragile tile is expensive because it is risky"*.

#### 7.3.6. Cắm hàm chi phí vào code

- Viết hàm `(state, action, next_state) -> float` và truyền vào `BloxorzProblem(level, cost_fn=...)`. `successors()` tự trả chi phí, `make_result` tự tính `cost`.
- **Việc còn thiếu:** `main.py` luôn dùng `unit_cost`, nên nút UCS trong game hiện tại sẽ cho kết quả giống BFS. Cần thêm tham số chọn hàm chi phí (ví dụ `--cost unit|energy|fragile`). Có thể để UCS và A\* mặc định dùng hàm không đồng nhất khi được gọi từ game.
- `tests/test_solvers.py` hiện kiểm tra UCS/A\* với **chi phí 1** (độ dài 10 và 68). Khi thêm hàm chi phí mới, giữ test đó và thêm test riêng, ví dụ `cost(A*) == cost(UCS)` với hàm mới, cùng màn đồ chơi 7.3.5 (UCS phải ra chi phí 7).

### 7.4. A\* Search (TV4 solver, TV5 heuristic; đề mục 3.5)

#### 7.4.1. Ý tưởng và mã giả

Giống UCS nhưng ưu tiên theo **`f(s) = g(s) + h(s)`**: `g` là chi phí đã đi, `h` là **ước lượng** chi phí còn lại tới đích. Nhờ `h`, A\* "hướng" về phía đích thay vì loang đều mọi phía như UCS.

```
A*(problem, h):
    giống UCS, chỉ đổi khóa heap: (g + h(s'), h(s'), counter++, s')   ← hòa f thì ưu tiên h nhỏ (gần đích hơn)
    vẫn giữ best_g + lazy deletion; vẫn kiểm tra đích KHI LẤY RA
```

- `h = 0` → A\* **chính là UCS** (`zero_heuristic`; test `astar_zero` dùng điều này để đối chứng).
- `h` **admissible** (không bao giờ đánh giá quá chi phí thật) → A\* **tối ưu**.
- `h` **consistent** (`h(s) ≤ c(s, a, s') + h(s')` với mọi nước, và `h(đích) = 0`) → mỗi trạng thái chỉ cần mở rộng **một lần**. Consistent kéo theo admissible.

#### 7.4.2. Thiết kế heuristic theo gợi ý của đề

Đề đưa ra 3 ý tưởng (không bắt buộc). Phân tích từng ý tưởng với **chi phí 1**:

**① Manhattan chia 2.** Gọi `d(s)` = khoảng cách Manhattan **nhỏ nhất** từ các ô khối chiếm tới ô đích. Theo tính chất 2 của bảng chuyển (mục 2.2), mỗi nước đi làm `d` giảm **tối đa 2**. Ở đích `d = 0`, vậy cần ít nhất `⌈d/2⌉` nước. Nếu chưa ở đích thì cần ít nhất 1 nước. Ta có:

```
h(s) = 0                       nếu s là đích
h(s) = max(⌈d(s)/2⌉, 1)        nếu không
```

- **Admissible:** mọi đường tới đích có ít nhất `h(s)` nước, mỗi nước tốn 1.
- **Consistent:** sau một nước, `d` giảm tối đa 2 nên `⌈d/2⌉` giảm tối đa 1, suy ra `h(s) ≤ 1 + h(s')`. Với nước đi vào đích: trạng thái liền trước có `d ≤ 2` nên `h = 1 = c`, và `h(đích) = 0`.

**② Thành phần theo tư thế ("chưa đứng thì cần thêm 1 nước").** ⚠️ **Cẩn thận: công thức `⌈d/2⌉ + 1` khi khối đang nằm KHÔNG admissible.** Phản ví dụ (đã kiểm chứng bằng `replay`): lưới 4 dòng × 1 cột, khối `X` tại `(1,0)` chiếm `(1,0),(2,0)`, đích `(3,0)`. Ta có `d = 1`, công thức cho `⌈1/2⌉ + 1 = 2`, nhưng chỉ cần **1** nước (`R` → đứng tại `(3,0)`). Dùng dạng `max(…, 1)` như ① thì an toàn.

**③ Khoảng cách "điểm" trên lưới 4-kề (bài toán nới lỏng).** Bỏ qua tư thế, ô trống và ô cam, coi khối là một điểm đi trên lưới gồm mọi ô **có thể** có sàn (kể cả cầu coi như mở), tính đường đi ngắn nhất tới đích bằng một lần BFS ngược từ đích. Cách này chặt hơn Manhattan vì tính đến "tường" ô trống. ⚠️ **Phải chia 2 (lấy trần).** Khoảng cách điểm chưa chia **không** phải cận dưới số nước, vì một nước lăn đưa khối đi được 2 ô. Phản ví dụ (đã kiểm chứng): khối đứng tại `(0,0)`, đích `(3,0)` trên một hành lang thẳng. Khoảng cách điểm là 3, nhưng `R R` là thắng sau **2** nước. Dùng `max(⌈dist/2⌉, 1)` thì vẫn admissible và consistent, lập luận giống ①.

**Cầu và công tắc không phá admissible.** Heuristic coi mọi cầu là mở (lạc quan hơn thực tế), nên vẫn không đánh giá quá. Riêng **split switch** (nếu sau này cài) dịch chuyển tức thời khối tới vị trí khác, nên `d` có thể giảm rất nhiều trong một nước và heuristic có thể **mất admissible**. Cách sửa (thêm "cạnh cổng dịch chuyển" chi phí 0 khi tính khoảng cách) ở `report0.md` mục 5.4.

#### 7.4.3. Khi dùng chi phí không đồng nhất

Heuristic phải admissible **theo chi phí đang dùng**:

- **Cách đơn giản:** nhân số nước tối thiểu với **chi phí nhỏ nhất của một nước** `c_min`: `h = c_min · n(s)` với `n(s) = max(⌈d/2⌉, 1)`. Chi phí phạt ô cam (7.3.5) có `c_min = 1` nên heuristic chi phí 1 vẫn dùng được nguyên vẹn.
- **Chặt hơn cho chi phí năng lượng:** nước vào đích **luôn là một nước dựng đứng** (đổ tạo khối nằm, lăn ngang giữ khối nằm), nên `h = c_dựng + c_min · (n(s) − 1)` = `1.618 + 1.118 · (n − 1)`. Chứng minh admissible và consistent ở `report0.md` mục 5.4.
- **Bẫy:** nếu có nước "rẻ" (ví dụ bấm công tắc tốn 0.5) thì `c_min` phải là giá trị nhỏ nhất **thật sự**, nếu không heuristic mất admissible.
- **Kiểm chứng:** với cùng hàm chi phí, `cost(A*)` phải **bằng** `cost(UCS)` trên mọi màn. Khác nhau nghĩa là heuristic không admissible hoặc có bug.

#### 7.4.4. Ví dụ dò tay: 6 lần lấy ra đầu tiên của A\* trên màn 1

Chi phí 1, `h = max(⌈d/2⌉, 1)`, đích `(7,7)`:

| Lần | Lấy ra (f = g + h) | Sinh con: g / h / f | Ghi chú |
|---:|---|---|---|
| 1 | `(3,3,Z)` f = 0 + 4 | `(3,4,Y)` 1/3/**4**, `(4,3,X)` 1/3/**4** | `d = 8` → `h = 4` |
| 2 | `(3,4,Y)` f = 4 | `(2,4,Y,mask=1)` 2/4/6, `(4,4,Y)` 2/3/5 | đè công tắc (2,5) xa đích → f = 6, bị xếp sau |
| 3 | `(4,3,X)` f = 4 | `(4,4,X)` 2/3/5, `(4,2,X)` 2/4/6 | |
| 4 | `(4,4,Y)` f = 5 | `(5,4,Y)` 3/2/5, `(4,3,Z)` 3/4/7 | hòa với `(4,4,X)` cả f = 5 lẫn h = 3 → lấy cái được đẩy vào trước |
| 5 | `(5,4,Y)` f = 5 | `(5,3,Z)` 4/3/7 | hòa f = 5 với `(4,4,X)` nhưng h = 2 < 3 nên được lấy trước; con của nó có f = 7, nhánh này bắt đầu "đắt" |
| 6 | `(4,4,X)` f = 5 | `(4,5,X)` 3/2/**5**, `(3,4,Z)` 3/4/7 | `(4,5,X)` chính là trạng thái sau nước 3 của lời giải tối ưu (bảng 3.4) |

So với BFS (loang hết tầng 1, 2, 3… theo mọi phía), A\* gần như đi thẳng theo hướng lời giải và để các nhánh xa đích (như nhánh đè công tắc `(2,5)`) lại phía sau. Đó là lý do A\* với heuristic tốt **mở rộng ít node hơn** BFS/UCS nhưng vẫn tối ưu.

### 7.5. Bảng so sánh lý thuyết (dùng cho báo cáo)

| Thuật toán | Frontier | Đầy đủ | Tối ưu | Thời gian | Bộ nhớ | Ghi chú |
|---|---|---|---|---|---|---|
| BFS | Hàng đợi FIFO | Có (hữu hạn) | Theo số nước | `O(b^d)` | `O(b^d)` | Mốc tối ưu số nước |
| DFS + visited | Ngăn xếp LIFO | Có (hữu hạn) | **Không** | `O(\|S\|)` | `O(\|S\|)` do visited | Lời giải dài, phụ thuộc thứ tự hành động |
| IDS (tùy chọn) | DFS giới hạn sâu | Có | Theo số nước | `O(b^d)`, mở lại nhiều | `O(b·d)` | Tiết kiệm bộ nhớ, chậm hơn |
| UCS | Heap theo `g` | Có (c ≥ ε) | Theo **tổng chi phí** | `O(b^{1+⌊C*/ε⌋})` | như thời gian | ≡ BFS khi chi phí đều |
| A\* | Heap theo `g + h` | Có | Có (h admissible) | Tùy chất lượng `h` | Lưu mọi node đã sinh | Ít node nhất nếu `h` tốt |

---

## 8. Đo đạc và thực nghiệm

Đề mục 3.6 *Experiments* (20 điểm): bộ test **cố định**, chia nhóm theo độ khó (cơ bản, có ô cam, có công tắc, kết hợp), đo **4 chỉ số** cho từng thuật toán, trình bày bằng **bảng + biểu đồ + nhận xét**.

| Chỉ số đề | Lấy từ đâu | Cách đo đúng |
|---|---|---|
| Search time | `SearchResult.time_ms` | `time.perf_counter()` chỉ bao quanh `solve()`; chạy nhiều lần, lấy trung vị; không in log trong vòng lặp |
| Memory usage (peak) | `SearchResult.peak_memory_kb` | Lượt chạy riêng có `tracemalloc` (`--memory`) |
| Expanded nodes | `SearchResult.nodes_expanded` | Thống nhất định nghĩa giữa 4 solver (mục 7.0) |
| Solution length | `SearchResult.solution_length` | Kèm `cost` cho UCS/A\* khi chi phí không đồng nhất |

Lệnh nhanh: `python main.py code/data/<n>.data --compare --memory` in bảng 4 thuật toán. Thực nghiệm chính thức nên có script riêng (ví dụ `experiments/run_all.py`) chạy mọi màn × mọi thuật toán, ghi CSV (thư mục `results/` đã được gitignore), rồi vẽ biểu đồ (trục log cho số node).

**Gợi ý nhận xét cần kiểm chứng bằng số liệu thật, không viết trước:** BFS và A\* (cùng chi phí 1) cho cùng độ dài nhưng A\* mở rộng ít node hơn; DFS có thể nhanh trên vài màn nhưng lời giải dài; UCS chỉ khác BFS khi bộ trọng số đủ chênh và màn có nhiều đường (mục 7.3.4); số trạng thái tăng theo `2^k` khi có nhiều công tắc.

---

## 9. Khoảng cách so với đề — việc còn lại

| Việc | Mục đề | Ai (theo `Docs/task.md`) |
|---|---|---|
| Cài thân `solve()` của BFS / DFS / UCS / A\* | 3.2–3.5 | TV1 / TV2 / TV3 / TV4 |
| Cài `manhattan_heuristic` (hoặc heuristic tốt hơn) + chứng minh | 3.5 | TV5 |
| Hàm chi phí không đồng nhất + lý giải + tham số chọn trong `main.py` | 3.4 | TV3 |
| Split switch (tách/gộp 2 khối lập phương), cầu permanent, một công tắc → nhiều ô cầu: sửa **cả** C++ và Python, mở rộng định dạng `.data` | 3.1 (advanced, 10đ) | TV1 (+ TV2 phần vẽ) |
| Tối thiểu 10 màn chia nhóm độ khó | Q&A, 3.6 | TV4 |
| Script thực nghiệm, CSV, biểu đồ | 3.6 | TV5 |
| Báo cáo PDF, video YouTube công khai có phụ đề/thuyết minh | 3.6, 3.7 | Cả nhóm |

Khi thêm split switch, `State` phải mở rộng (vị trí hai khối lập phương, có thể thêm khối đang điều khiển), hành động có thể thành 8 (`(khối_i, hướng)`), và heuristic phải xử lý dịch chuyển tức thời. Chi tiết các phương án ở `report0.md` mục 3.2–3.3.

---

## 10. Câu hỏi vấn đáp thường gặp

**Hỏi: Trạng thái của bài toán gồm những gì? Vì sao cần `bridge_mask`?**
Đáp: `(x, y, tư thế, bridge_mask)`. Đề yêu cầu cấu hình cầu là một phần trạng thái: cùng vị trí khối nhưng cầu khác nhau thì các nước đi hợp lệ khác nhau. Thiếu `bridge_mask` thì visited sẽ cắt nhầm lời giải phải quay lại chỗ cũ sau khi bấm công tắc.

**Hỏi: Nước đi không hợp lệ được xử lý ở đâu?**
Đáp: `BloxorzProblem.simulate` trả `Outcome.LOST`; `successors()` và `actions()` lọc bỏ các nước đó, nên solver không bao giờ sinh trạng thái "đã rơi".

**Hỏi: Vì sao BFS tối ưu mà DFS thì không?**
Đáp: BFS xét hết các trạng thái cách gốc `k` nước rồi mới tới `k+1`, nên lời giải đầu tiên gặp là ngắn nhất. DFS đi theo nhánh đầu tiên tới cùng và trả về lời giải đầu tiên gặp, không quan tâm độ dài.

**Hỏi: Khi nào UCS giống BFS?**
Đáp: Khi mọi nước có cùng chi phí. Với bộ trọng số năng lượng, UCS cũng gần như giống BFS vì `chi_phí = 1.207·L + 0.322·k` (mục 7.3.4).

**Hỏi: Vì sao UCS phải kiểm tra đích khi lấy ra chứ không phải khi sinh?**
Đáp: Lúc sinh, đích có thể đang đi theo đường chưa rẻ nhất; một đường rẻ hơn có thể vẫn đang nằm trong heap. Lấy ra theo `g` nhỏ nhất thì lần đầu lấy ra đích là lúc `g` đã tối ưu.

**Hỏi: Admissible và consistent khác nhau thế nào? Heuristic của nhóm có cả hai không?**
Đáp: Admissible: `h(s)` ≤ chi phí thật còn lại. Consistent: `h(s) ≤ c(s,a,s') + h(s')` và `h(đích) = 0`. Consistent kéo theo admissible. `max(⌈d/2⌉, 1)` có cả hai với chi phí 1 vì mỗi nước làm `d` giảm tối đa 2 (mục 7.4.2).

**Hỏi: Vì sao không cộng "+1 nếu đang nằm" vào heuristic?**
Đáp: Không admissible: khối nằm cách đích 1 ô có thể dựng đứng vào đích chỉ bằng 1 nước, trong khi công thức cho 2 (phản ví dụ ở 7.4.2).

**Hỏi: "Memory usage" đo cái gì, đo thế nào?**
Đáp: Bộ nhớ **đỉnh** do thuật toán cấp phát trong lúc tìm kiếm, đo bằng `tracemalloc` (`get_traced_memory()[1]`) trong `BaseSolver.run(measure_memory=True)`, ở một lượt chạy riêng để không làm sai thời gian. Với BFS, phần lớn là `parent` dict (mọi trạng thái đã thấy) và hàng đợi (mục 7.1.4).

**Hỏi: Làm sao biết luật Python khớp với game C++?**
Đáp: Sao chép đúng thứ tự `Game::check` / `check_after_step`, có 36 test luật trong `tests/test_problem.py`, và phát lại lời giải trong game thật bằng `./a.out <n> --play <file>` (màn 1 thắng sau 10 bước, màn 2 sau 68 bước). `main.py` cũng tự phát lại mọi lời giải trước khi ghi ra file.

**Hỏi: GUI viết bằng C++, có vi phạm "source code must be written in Python" không?**
Đáp: Theo Q&A với giảng viên (câu 7), GUI được viết bằng ngôn ngữ khác, **thuật toán phải bằng Python**. Toàn bộ phần tìm kiếm nằm trong `ai/` (Python, chỉ thư viện chuẩn, tự cài). Game C++ chỉ gọi `main.py` rồi phát lại kết quả. Mã nguồn game gốc được dùng theo phép giảng viên và ghi nguồn đầy đủ.
