# Report 0 — Phân tích đồ án Project 1: Bloxorz Solver

> **Môn học:** CSC14003 — Cơ sở Trí tuệ Nhân tạo (Khoa CNTT, ĐH KHTN, ĐHQG-HCM)
> **Tài liệu gốc:** `AI - Project 1.pdf`
> **Mục đích của tài liệu này:** đọc kỹ đề, phát biểu lại bài toán một cách chặt chẽ, liệt kê đầy đủ yêu cầu và thang điểm, đề xuất thiết kế kỹ thuật, lập kế hoạch thực hiện theo sprint và tổng hợp các lưu ý/bẫy cần tránh. Đây là "report 0", tức tài liệu định hướng nội bộ của nhóm **trước khi** bắt tay vào code, không phải báo cáo nộp cuối kỳ.

> **Cập nhật 20/10/2026 — hướng thực hiện đã chốt.** Nhóm dùng game 3D C++/OpenGL của repo [`chaitanya100100/Bloxorz`](https://github.com/chaitanya100100/Bloxorz) (commit `c0ccb55`) làm nền, theo sự cho phép của giảng viên (xem mục 1.1). Phần AI viết bằng Python trong `ai/`; game C++ gọi solver Python và tự phát lại lời giải. Các mục thay đổi so với bản đầu: **1.1–1.2** (Q&A và hướng đi), **2.6** (luật thực tế của game nền và khác biệt so với đề), **3.7** (mô hình trạng thái đã cài), **4.3** (GUI 3D), **6** (kiến trúc thật của repo), **7.1** (tối thiểu 10 màn, số liệu nền), **8** (kế hoạch 5 người), **9.6** (lưu ý kỹ thuật mới). Kế hoạch chi tiết từng bước xem `Docs/implementation_plan.md`, danh sách task xem `Docs/task.md`.

---

## Mục lục

1. [Tổng quan đồ án](#1-tổng-quan-đồ-án)
2. [Phát biểu chi tiết bài toán](#2-phát-biểu-chi-tiết-bài-toán)
3. [Mô hình hóa thành bài toán tìm kiếm](#3-mô-hình-hóa-thành-bài-toán-tìm-kiếm)
4. [Yêu cầu của đồ án và thang điểm](#4-yêu-cầu-của-đồ-án-và-thang-điểm)
5. [Phân tích từng thuật toán và hướng thiết kế](#5-phân-tích-từng-thuật-toán-và-hướng-thiết-kế)
6. [Kiến trúc phần mềm (đã chốt)](#6-kiến-trúc-phần-mềm-đã-chốt)
7. [Thiết kế bộ test và thực nghiệm](#7-thiết-kế-bộ-test-và-thực-nghiệm)
8. [Kế hoạch thực hiện theo Sprint](#8-kế-hoạch-thực-hiện-theo-sprint)
9. [Các lưu ý quan trọng](#9-các-lưu-ý-quan-trọng)
10. [Checklist trước khi nộp](#10-checklist-trước-khi-nộp)

---

## 1. Tổng quan đồ án

| Mục | Nội dung |
|---|---|
| Tên đồ án | **Bloxorz Solver** |
| Hình thức | Làm **theo nhóm** |
| Thời lượng | Khoảng **3 tuần** |
| Ngôn ngữ bắt buộc | **Python** cho thuật toán tìm kiếm (GUI được viết bằng ngôn ngữ khác — Q&A câu 7) |
| Sản phẩm phải nộp | Mã nguồn (có **GUI 3D**) + `README.md` + `requirements.txt` + **Report.pdf** (có link video YouTube) |
| Trọng tâm học thuật | Mô hình hóa bài toán tìm kiếm, cài đặt **BFS, DFS/IDS, UCS, A\***, thiết kế **hàm chi phí** và **heuristic chấp nhận được**, thực nghiệm so sánh |

**Bloxorz** là trò chơi giải đố: người chơi lăn một khối hộp chữ nhật kích thước **1 × 1 × 2** trên một mặt sàn gồm các ô vuông. Mục tiêu là làm cho khối **đứng thẳng** rơi vào **lỗ đích**. Điểm thú vị là khối không phải một điểm: tùy theo đang **đứng** hay **nằm**, nó chiếm 1 hoặc 2 ô và tương tác khác nhau với mặt sàn.

Vì sao trò chơi này phù hợp để luyện thuật toán tìm kiếm?

- **Quan sát đầy đủ (fully observable):** toàn bộ bàn chơi hiển thị rõ ràng.
- **Tất định (deterministic):** mỗi lần lăn cho ra đúng một trạng thái kế tiếp, không có yếu tố ngẫu nhiên hay đối thủ.
- **Hướng (orientation) của khối là một phần của trạng thái**, không chỉ tọa độ → bài toán rất hợp để thiết kế và phân tích heuristic.
- Khi thêm công tắc/cầu, **trạng thái của các cây cầu cũng là một phần của trạng thái** → không gian trạng thái phình to theo cấp số nhân, khiến sự khác biệt giữa các thuật toán thể hiện rõ.

### 1.1. Kết luận từ buổi Q&A với giảng viên

| # | Câu hỏi | Trả lời | Ảnh hưởng tới nhóm |
|---:|---|---|---|
| 1 | Test case dùng nguồn có sẵn? | Được | Được dựng lại màn kinh điển, ghi nguồn |
| 2, 9 | Số test case / số màn? | **Tối thiểu 10** | Bộ màn G1–G6 (mục 7.1) phải có ≥ 10 màn |
| 3 | Báo cáo tiếng Anh hay Việt? | Tùy, nhưng thống nhất | Viết toàn bộ bằng một ngôn ngữ |
| 4 | Bấm Solve thì GUI hiện gì? | **GUI cho khối tự lăn đến đích** | Bắt buộc có phát lại lời giải (đã có `autoplay`) |
| 5 | Dùng source game có sẵn? | **Được, cite đầy đủ** | Dùng repo chaitanya làm nền |
| 6 | Tiêu chí GUI? | **3D** | GUI Pygame 2D không còn phù hợp → dùng OpenGL 3D |
| 7 | GUI viết bằng ngôn ngữ khác? | Được, **thuật toán phải bằng Python** | GUI C++, solver Python |
| 10 | Dùng ML suy ra trọng số chi phí? | Được, trình bày rõ | Một hướng mở rộng cho hàm chi phí UCS |
| 11 | Agent biết đầy đủ thông tin? | Được, giữ giả thiết fully observable | Mô hình trạng thái hiện tại không đổi |

### 1.2. Hướng thực hiện đã chốt

- **Game/GUI:** C++/OpenGL 3.3 từ `chaitanya100100/Bloxorz` @ `c0ccb55` (không có license — dùng theo phép của giảng viên, cite trong `THIRD_PARTY_NOTICES.md`, `README.md` và mục References). Nhóm sửa/bổ sung trong `code/`.
- **AI:** package Python `ai/` (OOP) mô phỏng **chính xác** luật của game C++; 4 solver do nhóm tự cài.
- **Cầu nối:** game chạy `python ../main.py data/<n>.data --algo <algo> --out solution.txt` rồi tự phát lại lời giải (`./a.out <n> --solve <algo>`).
- **Môi trường build chính:** WSL Ubuntu (có WSLg để mở cửa sổ OpenGL); Python ≥ 3.10.

---

## 2. Phát biểu chi tiết bài toán

### 2.1. Bàn chơi (stage)

Một màn chơi là một lưới hữu hạn gồm `R` hàng × `C` cột ô vuông. Mỗi ô thuộc một trong các loại:

| Loại ô | Ý nghĩa |
|---|---|
| **Floor** (sàn thường) | Ô rắn, khối có thể nằm/đứng trên đó. |
| **Void** (ô trống) | Không có sàn. Nếu **bất kỳ phần nào** của khối nằm trên void → khối rơi → nước đi **không hợp lệ**. |
| **Goal** (lỗ đích) | Ô đích **duy nhất**. Chỉ thắng khi khối **đứng thẳng** trên ô này. |
| **Fragile** (ô dễ vỡ, màu cam) | Chỉ chịu được khối khi khối **nằm** (trọng lượng được phân bố). Khối **đứng** trên ô fragile → ô vỡ → rơi → thua. |
| **Bridge** (cầu) | Ô có trạng thái **mở** (như sàn thường) hoặc **đóng** (như void), được điều khiển bởi công tắc. |
| **Soft switch** (công tắc mềm, hình tròn/bát giác) | Kích hoạt khi **bất kỳ phần nào** của khối đè lên (đứng hay nằm đều được). |
| **Heavy switch** (công tắc nặng, hình chữ X) | Chỉ kích hoạt khi khối **đứng** trên nó. Nằm đè lên **không** kích hoạt. |
| **Split switch** (công tắc tách, hình "( )") | Chỉ kích hoạt khi khối **đứng** trên nó. Khối bị **dịch chuyển tức thời** tới các vị trí định sẵn và **tách thành hai khối lập phương 1×1×1**. |

Khối bắt đầu ở trạng thái **đứng** trên một ô xuất phát cho trước.

### 2.2. Ba hướng (orientation) của khối

Quy ước tọa độ `(r, c)` với `r` là hàng (tăng dần xuống dưới), `c` là cột (tăng dần sang phải). Khối luôn ở **đúng một** trong ba hướng:

| Hướng | Ký hiệu | Các ô bị chiếm (lấy `(r, c)` làm ô "neo") |
|---|---|---|
| Đứng (Standing) | `S` | `(r, c)` |
| Nằm ngang (Lying horizontally) | `H` | `(r, c)` và `(r, c+1)` |
| Nằm dọc (Lying vertically) | `V` | `(r, c)` và `(r+1, c)` |

> Quy ước ô neo luôn là ô **trên cùng / trái nhất** giúp mỗi vị trí vật lý có **đúng một** biểu diễn, tránh trùng lặp trạng thái.

### 2.3. Luật di chuyển và bảng chuyển trạng thái

Mỗi lượt, người chơi lăn khối theo một trong bốn hướng **Up / Down / Left / Right**. Có ba kiểu chuyển động:

1. **Khối đứng** → đổ xuống, nằm trên **hai ô** theo hướng lăn.
2. **Khối nằm dọc theo hướng lăn** → **dựng đứng** lên một ô.
3. **Khối nằm vuông góc với hướng lăn** → **lăn ngang**, vẫn nằm, dịch đi một ô.

Bảng chuyển trạng thái đầy đủ (đề bài nhấn mạnh: *cài đặt và unit-test bảng này thật sớm sẽ tiết kiệm rất nhiều thời gian debug*):

| Trạng thái hiện tại | Up | Down | Left | Right |
|---|---|---|---|---|
| `S (r,c)` | `V (r-2, c)` | `V (r+1, c)` | `H (r, c-2)` | `H (r, c+1)` |
| `H (r,c)` | `H (r-1, c)` | `H (r+1, c)` | `S (r, c-1)` | `S (r, c+2)` |
| `V (r,c)` | `S (r-1, c)` | `S (r+2, c)` | `V (r, c-1)` | `V (r, c+1)` |

Nhận xét quan trọng (đã kiểm chứng bằng script): **với mọi cặp (hướng khối, hướng lăn), tập ô sau khi lăn hoàn toàn rời khỏi tập ô trước khi lăn**. Hệ quả: một công tắc bị đè ở cuối nước đi luôn là công tắc "vừa được đè lên mới", nên quy tắc *"kích hoạt khi kết thúc nước đi mà khối đè lên công tắc"* là nhất quán, không có trường hợp "đứng yên trên công tắc qua nhiều lượt".

**Nước đi không hợp lệ** nếu sau khi lăn, bất kỳ ô nào khối chiếm:
- nằm ngoài bàn chơi, hoặc
- là void, hoặc
- là cầu đang **đóng**, hoặc
- là ô fragile trong khi khối **đứng**.

### 2.4. Điều kiện thắng

Thắng khi và chỉ khi khối **nguyên vẹn (1×1×2)** ở trạng thái **đứng** trên ô **goal**. Khối nằm vắt qua ô goal **không** thắng (và cũng không rơi — ô goal đóng vai trò như sàn khi khối nằm).

### 2.5. Cơ chế nâng cao

**Công tắc và cầu.** Mỗi công tắc liên kết với **một hoặc nhiều** ô cầu. Khi kích hoạt, công tắc tác động lên cầu theo một trong hai kiểu:

- **Toggle:** mỗi lần kích hoạt đảo trạng thái mở ↔ đóng của các cầu liên kết.
- **Permanent:** đặt các cầu về một trạng thái cố định (ví dụ "luôn mở" hoặc "luôn đóng") sau khi kích hoạt — kích hoạt lại không thay đổi gì thêm.

**Tách khối (split).** Khi khối đứng trên split switch:
- khối được dịch chuyển tới **hai vị trí định sẵn** và tách thành **hai khối lập phương 1×1×1**;
- người chơi điều khiển **từng khối một** (ví dụ bấm phím **Space** để đổi khối đang điều khiển);
- khi hai khối lập phương nằm ở **hai ô kề nhau**, chúng **tự gộp lại** thành khối 1×1×2 (nằm ngang hoặc dọc);
- một khối lập phương **có thể** kích hoạt soft switch, nhưng **không thể** kích hoạt heavy switch và **không thể** hoàn thành màn chơi.

### 2.6. Luật thực tế của game nền (chaitanya100100/Bloxorz) và khác biệt so với đề

Đọc từ `code/tile.cpp` (`Floor::initialize`), `code/superblock.cpp` (`SuperBlock::update`) và `code/game.cpp` (`Game::check`, `Game::check_after_step`). Toàn bộ luật này đã được chép sang `ai/problem.py` và kiểm chứng bằng test.

**Tọa độ và quy đổi.** Game lưu `tiles[x][y]`: `x` là chỉ số **dòng** trong file `.data`, `y` là chỉ số **cột**. Hướng đi: `R: x+1`, `L: x−1`, `U: y+1`, `D: y−1`. Trong cửa sổ 3D, trục x hiển thị theo chiều ngang nên `R` nhìn thấy là sang phải; nhưng **khi đọc file text, `R` là đi xuống một dòng, `U` là sang phải một cột**. Quy đổi với ký hiệu ở mục 2.2–2.3: `x ≡ r`, `y ≡ c`; tư thế `Z ≡ S` (đứng), `X ≡ V` (chiếm `(x,y),(x+1,y)`), `Y ≡ H` (chiếm `(x,y),(x,y+1)`).

**Bảng chuyển theo tọa độ của code** (`SuperBlock::update`):

| Tư thế | L | R | U | D |
|---|---|---|---|---|
| `Z (x,y)` | `X (x−2, y)` | `X (x+1, y)` | `Y (x, y+1)` | `Y (x, y−2)` |
| `X (x,y)` | `Z (x−1, y)` | `Z (x+2, y)` | `X (x, y+1)` | `X (x, y−1)` |
| `Y (x,y)` | `Y (x−1, y)` | `Y (x+1, y)` | `Z (x, y+2)` | `Z (x, y−1)` |

**Định dạng `.data`** (đọc theo token): `n m` → lưới `n×m` (0 trống/lỗ, 1 sàn, 2 fragile, 3 công tắc thường, 4 công tắc nặng) → `sx sy along` (vị trí đầu + tư thế X/Y/Z, **có thể bắt đầu ở tư thế nằm**) → `tx ty` (đích, ô lưới = 0) → `k` + `k` dòng `x y a b` (công tắc thường tại `(x,y)` bật/tắt ô `(a,b)`) → `h` + `h` dòng cho công tắc nặng.

**Luật thắng/thua** (`Game::check`, chạy **mỗi frame**): ô đầu trống → nếu không phải đích thì thua, nếu là đích và đang đứng thì thắng; đứng trên fragile → thua; tư thế X/Y mà ô thứ hai trống và không phải đích → thua. **Thua được ưu tiên hơn thắng.** Nằm vắt qua lỗ đích **không thua, không thắng**.

**Công tắc** (`Game::check_after_step`): nếu nước đi vừa thắng/thua thì **không kích hoạt công tắc**. Ngược lại, kích hoạt **tuần tự**: ô đầu là công tắc thường → toggle; (X/Y) ô thứ hai là công tắc thường → toggle; đứng trên công tắc nặng → toggle. Toggle: ô cầu trống → tạo sàn thường (type 1); có sàn → xóa. Frame sau `check` chạy lại → nếu cầu dưới khối vừa bị xóa thì khối rơi (thua).

**Số liệu kiểm chứng (BFS trên mô phỏng Python):** màn 1 tối ưu **10 bước** (161 trạng thái đạt được); màn 2 tối ưu **68 bước** (378 trạng thái). Hai lời giải `R U U R U R U R D L` (màn 1) và 68 bước (màn 2) được xác nhận thắng trong game thật (`./a.out <n> --play`).

**Khác biệt giữa game nền và đề → việc nhóm phải làm:**

| Yêu cầu của đề | Game chaitanya | Việc cần làm |
|---|---|---|
| Floor, void, goal; 3 tư thế; 4 hướng; rơi; thắng | Có đủ | Giữ; đã thêm kiểm tra biên mảng (`Floor::has_tile`) |
| Fragile | Có (type 2) | Giữ |
| Soft switch | Có — "normal switch" (type 3), kích hoạt khi bất kỳ ô nào đè | Giữ |
| Heavy switch | Có (type 4), chỉ khi đứng | Giữ |
| Một công tắc → **nhiều** ô cầu | Mỗi công tắc đúng **1** ô | Mở rộng định dạng `.data` (v2) — C++ và Python |
| Toggle **và permanent** | Chỉ toggle | Thêm chế độ permanent |
| **Split switch**, tách/gộp 2 cube | **Không có** | Thêm (phần khó nhất của 10đ Advanced tiles) |
| Nút New Game, Restart, Solve | Không có (chỉ phím) | Thêm UI (ví dụ Dear ImGui) |
| Solve → khối tự lăn | Không có | **Đã có** (`autoplay`, `--solve`, `--play`) |
| ≥ 10 màn | 2 màn | Thiết kế thêm (mục 7.1) |

---

## 3. Mô hình hóa thành bài toán tìm kiếm

Đây là phần cốt lõi về mặt học thuật (chiếm 10 điểm "Algorithm Description" trong report và là nền tảng cho toàn bộ 40 điểm solver).

### 3.1. Biểu diễn trạng thái

Trạng thái gồm **hai thành phần**:

```
State = (BlockConfig, BridgeConfig)
```

**BlockConfig** — một trong hai dạng:

- **Khối nguyên:** `("S"|"H"|"V", r, c)`.
- **Khối đã tách:** `("SPLIT", (r1, c1), (r2, c2), active)` với `active ∈ {0, 1}` là khối đang được điều khiển (nếu mô hình hóa thao tác "đổi khối" là một hành động — xem 3.2).

**BridgeConfig** — trạng thái mở/đóng của **mọi** cây cầu. Cách cài đặt gọn và nhanh nhất là **bitmask số nguyên** (bit `i` = 1 nếu cầu `i` mở) hoặc `frozenset` các cầu đang mở.

> ⚠️ Đề bài nhấn mạnh: hai trạng thái có **cùng vị trí khối** nhưng **khác cấu hình cầu** là **hai trạng thái khác nhau**. Nếu tập `visited` chỉ lưu vị trí khối, solver sẽ cắt bỏ nhầm những lời giải bắt buộc phải "đi ra bấm công tắc rồi quay lại chỗ cũ".

**Những thứ KHÔNG cần đưa vào trạng thái:**
- **Ô fragile:** nếu ô fragile vỡ thì người chơi đã thua → nước đi đó đơn giản là không hợp lệ, nên trạng thái "ô đã vỡ" không bao giờ xuất hiện trong một trạng thái hợp lệ.
- **Trạng thái công tắc permanent đã bấm hay chưa:** đã được phản ánh trong BridgeConfig (vì permanent chỉ đặt cầu về giá trị cố định, bấm lại không đổi gì).
- **Thông tin tĩnh của màn chơi** (loại ô, liên kết công tắc–cầu, vị trí đích, vị trí dịch chuyển của split switch): lưu trong đối tượng `Level` dùng chung, không nhân bản theo từng trạng thái.

**Ước lượng kích thước không gian trạng thái.** Với `T` ô sàn và `k` cây cầu (hoặc `k` nhóm cầu luôn thay đổi cùng nhau):

- Khối nguyên: tối đa `3T` vị trí.
- Khối tách: tối đa khoảng `T²` cặp vị trí (× 2 nếu lưu `active`).
- Cấu hình cầu: tối đa `2^k`.

→ `|S| ≲ (3T + 2T²) · 2^k`. Ví dụ `T = 60`, `k = 3`: `(180 + 7200) · 8 ≈ 59.000` trạng thái — vẫn nhỏ với BFS. **Hệ quả cho thực nghiệm:** trên màn nhỏ, các thuật toán chạy gần như tức thời và khác biệt không rõ; cần thiết kế thêm màn **lớn** và **nhiều công tắc** để thấy rõ khác biệt (xem mục 7).

### 3.2. Tập hành động

- **Khối nguyên:** `{Up, Down, Left, Right}` — tối đa 4 hành động.
- **Khối tách:** có hai cách mô hình hóa, nhóm cần **chọn một và ghi rõ trong report**:

| Phương án | Hành động | Ưu điểm | Nhược điểm |
|---|---|---|---|
| **A (khuyến nghị cho solver)** | 8 hành động: `(cube_i, dir)` với `i ∈ {0,1}` | Không cần thao tác "đổi khối"; độ dài lời giải = số lần lăn thực sự; có thể **chuẩn hóa** cặp cube (sắp xếp tọa độ) → giảm một nửa số trạng thái | Khác một chút so với cách chơi bằng bàn phím |
| **B (giống game gốc)** | 4 hướng cho khối đang active + hành động `SWITCH` | Khớp 1–1 với thao tác trong GUI | Thêm trạng thái `active`, thêm hành động "rỗng" cần định chi phí (0 hay 1?) — chi phí 0 có thể làm phức tạp UCS/A* |

Với phương án A, GUI vẫn có thể cho người chơi bấm Space để đổi khối; khi **phát lại lời giải** của solver, GUI tự chèn thao tác đổi khối khi cần.

### 3.3. Mô hình chuyển trạng thái (Transition Model)

Thứ tự xử lý đề xuất cho hàm `result(state, action)`:

```
1. Tính BlockConfig mới theo bảng chuyển trạng thái (mục 2.3)
   (hoặc di chuyển 1 cube 1 ô nếu đang ở chế độ tách).
2. Kiểm tra hợp lệ với BridgeConfig HIỆN TẠI:
   - mọi ô chiếm phải nằm trong bàn, không phải void, không phải cầu đóng;
   - khối đứng không được ở trên ô fragile.
   → nếu vi phạm: hành động không hợp lệ (không sinh trạng thái con).
3. Xác định các công tắc bị kích hoạt:
   - soft: có ô nào của khối/cube nằm trên nó;
   - heavy: khối nguyên ĐỨNG trên nó;
   - split: khối nguyên ĐỨNG trên nó.
4. Cập nhật BridgeConfig theo toggle/permanent (thứ tự xác định, ví dụ theo id công tắc).
5. Nếu kích hoạt split: thay BlockConfig bằng hai cube tại vị trí định sẵn.
6. Nếu đang tách và hai cube kề nhau (4-kề): gộp thành khối H hoặc V.
7. Kiểm tra lại tính hợp lệ với BridgeConfig MỚI
   (trường hợp hiếm: cầu dưới khối vừa bị đóng → khối rơi → không hợp lệ).
```

> Bước 7 xử lý tình huống khối nằm vắt một nửa trên soft switch, nửa còn lại trên chính cây cầu mà công tắc đó đóng lại. Cách xử lý này là một **quyết định thiết kế** — cần ghi rõ trong report.

### 3.4. Kiểm tra đích (Goal Test)

```
is_goal(state) ⇔ BlockConfig == ("S", goal_r, goal_c)
```

(Không phụ thuộc BridgeConfig; khối tách không bao giờ là đích.)

### 3.5. Hàm chi phí

- **Mặc định:** mỗi lần lăn có chi phí 1 → tối ưu số nước đi (dùng cho BFS/DFS và để so sánh).
- **Cho UCS (bắt buộc phải có ý nghĩa):** cần một hàm chi phí **không đồng nhất** và **có lý giải** (chi tiết ở mục 5.3).
- **Ràng buộc kỹ thuật:** mọi chi phí phải **dương** (`c ≥ ε > 0`). Chi phí 0 hoặc âm làm UCS/A* mất tính đúng đắn/tối ưu hoặc lặp vô hạn.

### 3.6. Tránh lặp trạng thái

- Trạng thái phải là **bất biến (immutable) và hashable** (tuple, namedtuple, `@dataclass(frozen=True)`), để làm khóa cho `set`/`dict`.
- BFS/DFS: tập `visited` (hoặc `explored ∪ frontier`).
- UCS/A*: `dict best_g[state]`; khi lấy một phần tử từ hàng đợi ưu tiên mà `g > best_g[state]` → bỏ qua (lazy deletion).

### 3.7. Mô hình đã cài trong repo (`ai/`)

- `State(x, y, orientation, bridge_mask)` — `@dataclass(frozen=True, slots=True)`, hashable. `bridge_mask`: bit `i` = số lần công tắc thứ `i` đã được kích hoạt (mod 2); một ô cầu mở khi (trạng thái ban đầu) XOR (số bit bật của các công tắc điều khiển nó là lẻ). Khi thêm split, `State` sẽ được mở rộng thêm vị trí hai cube.
- `BloxorzProblem.simulate(state, action) -> (next_state, Outcome)` chép đúng thứ tự của C++ (mục 2.6): lăn → `check` → (chưa thắng/thua) kích hoạt công tắc tuần tự → `check` lại. Các hàm cho solver: `initial_state()`, `actions()`, `result()`, `successors() -> [(action, next_state, cost)]`, `is_goal()`, `step_cost()` (mặc định 1; nhóm cắm hàm chi phí không đồng nhất qua tham số `cost_fn`).
- Thứ tự hành động cố định `U, D, L, R` → kết quả BFS/DFS tái lập được.
- So với đề xuất ở 3.3: game nền **không** có split và permanent, nên hiện tại không có bước 5–6; bước 7 (kiểm tra lại sau khi đổi cầu) **đã có** và khớp hành vi C++.

---

## 4. Yêu cầu của đồ án và thang điểm

### 4.1. Ràng buộc chung

- Mã nguồn **bắt buộc bằng Python**.
- **Được dùng thư viện hỗ trợ** (GUI, cấu trúc dữ liệu như `heapq`, `collections.deque`, vẽ biểu đồ...), nhưng **thuật toán tìm kiếm phải tự cài đặt**. Không dùng `networkx.shortest_path`, `aima-python`, `simpleai`... cho phần tìm kiếm.
- Tổ chức code tự do nhưng phải **chuyên nghiệp, rõ ràng**.

### 4.2. Bảng điểm tổng hợp (tổng 100 điểm)

| Hạng mục | Điểm | Nội dung chính |
|---|---:|---|
| **3.1 Cài lại game** | **20** | |
| ↳ Core mechanics | 10 | Floor, void, goal; khối 1×1×2 với 3 hướng; 4 hướng lăn đúng bảng chuyển; phát hiện nước đi không hợp lệ (rơi khỏi bàn/void); phát hiện thắng |
| ↳ Advanced tiles | 10 | Fragile, bridge, **đủ 3 loại** switch (soft, heavy, split với tách/gộp khối) và **cả hai** kiểu tác động (toggle, permanent). **Solver cũng phải xử lý đúng các ô này** |
| **3.2 BFS Solver** | **10** | Cài BFS; ghi nhận **Search Time, Memory Usage, Expanded Nodes, Solution Length** (áp dụng cho mọi solver) |
| **3.3 DFS Solver** | **10** | Cài DFS (hoặc IDS — phải nói rõ trong report); xử lý trạng thái lặp để tránh chu trình |
| **3.4 UCS Solver** | **10** | Cài UCS; **định nghĩa và lý giải hàm chi phí** trong report; chi phí đồng nhất làm UCS ≡ BFS → nên thiết kế chi phí **không đồng nhất** |
| **3.5 A\* Solver** | **10** | Cài A* với heuristic phù hợp; report phải giải thích cách thiết kế heuristic và chứng minh **admissible** và/hoặc **consistent** **theo đúng hàm chi phí đã chọn** |
| **3.6 Report (PDF)** | **30** | |
| ↳ Algorithm Description | 10 | Giải thích BFS, DFS/IDS, UCS, A*; hình thức hóa bài toán (trạng thái gồm hướng + cầu, hành động, transition, goal test, cost, heuristic, tránh lặp) |
| ↳ Experiments | 20 | Bộ test cố định, chia nhóm theo độ khó; đo 4 chỉ số; **bảng + biểu đồ + nhận xét** điểm mạnh/yếu |
| ↳ Planning & Task Distribution | (bắt buộc) | Phân công, **% hoàn thành của từng thành viên** — điểm cá nhân = điểm nhóm × % |
| ↳ References & Appendix | (bắt buộc) | Tài liệu tham khảo; phụ lục về việc dùng AI tạo sinh (LLM), các prompt đã dùng |
| **3.7 Video demo** | **10** | |
| ↳ Graphical Interface | 5 | Trình bày GUI: các loại ô, khối ở các hướng, các nút điều khiển |
| ↳ Feature Presentation | 5 | Chơi tay (lăn, bấm công tắc, thắng/thua); chạy solver và hiển thị/animate lời giải |
| **Tổng** | **100** | |

> **Nhận xét phân bổ điểm:** Report + video chiếm **40%**, riêng **Experiments chiếm 20%** — ngang với toàn bộ phần game. Nhóm **không được dồn hết thời gian vào code** rồi viết report vội vàng trong 1–2 ngày cuối.

### 4.3. Yêu cầu GUI

- **Bắt buộc có GUI** (desktop hoặc web). **Theo Q&A: GUI phải là 3D**, được viết bằng ngôn ngữ khác Python.
- **Theo Q&A: bấm Solve thì khối tự lăn tới đích** (phát lại lời giải trong GUI).
- Hiển thị các nút điều khiển cơ bản: **New Game**, **Restart**, và **nút gọi từng solver** (BFS / DFS / UCS / A*).
- Nên có thêm (để video đạt điểm tối đa): chọn màn, animate/replay lời giải, hiển thị số nước đi, hiển thị kết quả đo (thời gian, bộ nhớ, số node mở rộng, độ dài lời giải).

### 4.4. Định dạng màn chơi

- Phải có định dạng **text/JSON** đơn giản để nạp màn, **dùng chung cho mọi solver** (đảm bảo thực nghiệm công bằng). Nhóm dùng định dạng `.data` của game nền (mục 2.6), đọc được bởi cả C++ lẫn Python.
- Được tự thiết kế màn hoặc dựng lại màn kinh điển.

### 4.5. Các chỉ số đo bắt buộc (cho mỗi thuật toán, mỗi test case)

| Chỉ số | Ý nghĩa | Cách đo đề xuất |
|---|---|---|
| Search time | Thời gian chạy thuật toán | `time.perf_counter()` bao quanh **chỉ** phần tìm kiếm |
| Memory usage (peak) | Bộ nhớ đỉnh trong lúc tìm kiếm | `tracemalloc.start()` → `get_traced_memory()[1]` |
| Expanded nodes | Số node được lấy ra khỏi frontier và sinh con | Bộ đếm trong vòng lặp chính |
| Solution length | Số nước đi của lời giải | `len(path)`; với UCS/A* ghi thêm **tổng chi phí** |

### 4.6. Quy cách nộp bài

```
StudentID1_StudentID2_...(.zip / .rar)
├── Source/
│   ├── main.py
│   ├── README.md          (hướng dẫn chạy)
│   ├── requirements.txt   (thư viện cần cài)
│   └── ...
└── Report.pdf             (có link video demo)
```

- File nén > 20MB → upload Google Drive và chia sẻ link.
- **Tuyệt đối không chỉnh sửa sau deadline.**
- Video phải upload **YouTube** với link **công khai**, có **phụ đề hoặc thuyết minh** (tiếng Anh hoặc tiếng Việt).

### 4.7. Quy định học thuật

- Được dùng công cụ AI nhưng **phải dùng hợp lý**; giảng viên có quyền **phỏng vấn vấn đáp ngẫu nhiên** để kiểm tra mức độ hiểu bài.
- **Mọi hình thức đạo văn/gian lận → 0 điểm toàn môn.**

---

## 5. Phân tích từng thuật toán và hướng thiết kế

### 5.0. Khung chung cho mọi solver

Để thực nghiệm công bằng và code gọn, tất cả solver nên dùng chung một giao diện bài toán:

```python
class BloxorzProblem:
    def initial_state(self) -> State: ...
    def actions(self, s: State) -> list[Action]: ...          # chỉ trả về hành động hợp lệ
    def result(self, s: State, a: Action) -> State: ...
    def is_goal(self, s: State) -> bool: ...
    def step_cost(self, s: State, a: Action, s2: State) -> float: ...

@dataclass
class SearchResult:
    found: bool
    path: list[Action]
    cost: float
    expanded: int
    generated: int
    time_s: float
    peak_mem_bytes: int
    status: str          # "ok" | "no_solution" | "timeout" | "node_limit"
```

Thêm một hàm `successors(s)` trả về `(action, next_state, cost)` giúp tránh tính lại hai lần trong `actions()` và `result()`.

### 5.1. BFS — Breadth-First Search

- **Frontier:** hàng đợi FIFO (`collections.deque`).
- **Tính chất:** đầy đủ (complete) trên không gian hữu hạn; **tối ưu theo số nước đi** (mọi bước chi phí 1).
- **Độ phức tạp:** thời gian và bộ nhớ `O(b^d)` với `b ≤ 4` (hoặc 8 khi tách), `d` là độ dài lời giải tối ưu — thực tế bị chặn bởi `|S|`.
- **Chi tiết cài đặt:**
  - Đánh dấu `visited` **khi đưa vào hàng đợi** (không phải khi lấy ra) để tránh nhân bản trạng thái trong frontier.
  - Có thể kiểm tra đích **khi sinh** node con (early goal test) — vẫn tối ưu với BFS và tiết kiệm một lớp node. Nhưng phải **thống nhất cách đếm expanded nodes** giữa các thuật toán và ghi rõ trong report.
  - Truy vết đường đi bằng `parent: dict[State, (State, Action)]` thay vì lưu cả đường đi trong từng node (tiết kiệm bộ nhớ).
- **Kiểm chứng nhanh:** với màn mô phỏng Level 1 của game gốc (lưới 6×10, xuất phát `(1,1)`, đích `(4,7)`), BFS cài thử tìm được lời giải tối ưu **7 nước `R D D R R D R`**, mở rộng 65 node. Đây là test hồi quy tốt cho bảng chuyển trạng thái.

### 5.2. DFS — Depth-First Search (hoặc IDS)

- **Frontier:** ngăn xếp LIFO. **Nên cài bằng vòng lặp với stack tường minh**, tránh đệ quy (giới hạn đệ quy mặc định của Python ~1000 sẽ bị vượt trên màn lớn).
- **Tính chất:** đầy đủ trên không gian hữu hạn **khi có `visited`**; **không tối ưu** — lời giải thường dài hơn nhiều so với BFS.
- **Bộ nhớ:** về lý thuyết `O(b·m)` nhưng khi dùng `visited` toàn cục thì bộ nhớ thực tế vẫn `O(|S|)`. Đây là một điểm đáng thảo luận trong report.
- **Thứ tự hành động ảnh hưởng mạnh tới kết quả** (độ dài lời giải, số node mở rộng) → cố định thứ tự (ví dụ U, D, L, R) và ghi rõ.
- **Nếu chọn IDS:**
  - Ưu điểm: tối ưu theo số nước (như BFS) với bộ nhớ nhỏ.
  - Nhược điểm: mở rộng lại nhiều lần các tầng nông; việc chống lặp phức tạp hơn — chỉ kiểm tra lặp **trên đường đi hiện tại** (path checking), hoặc dùng `dict[state] = độ sâu nhỏ nhất đã gặp` trong từng vòng lặp sâu dần; dùng `visited` toàn cục kiểu DFS sẽ làm IDS **mất tính đầy đủ/tối ưu**.
  - **Phải ghi rõ trong report** là dùng IDS.
- **Khuyến nghị:** cài **DFS** (đúng tên đề) làm solver chính; nếu dư thời gian thì cài thêm IDS như phần mở rộng để thảo luận.

### 5.3. UCS — Uniform-Cost Search

- **Frontier:** hàng đợi ưu tiên `heapq` với phần tử `(g, counter, state)`; `counter` tăng dần để phá hòa và tránh so sánh trực tiếp hai `State`.
- **Kiểm tra đích khi LẤY RA khỏi heap**, không phải khi sinh — nếu không sẽ mất tính tối ưu.
- **Tính chất:** đầy đủ và **tối ưu theo tổng chi phí** khi mọi chi phí ≥ ε > 0.

**Thiết kế hàm chi phí — trái tim của phần này.** Đề nói rõ: nếu mọi bước có chi phí 1 thì UCS ≡ BFS; cần một hàm chi phí **không đồng nhất có lý giải**. Gợi ý hai hướng (có thể kết hợp):

**(a) Chi phí theo năng lượng vật lý (khuyến nghị — có cơ sở vật lý rõ ràng, dễ bảo vệ khi vấn đáp).**
Khi lăn khối quanh một cạnh đáy, trọng tâm phải được nâng lên tới điểm cao nhất (đứng thẳng trên cạnh quay) trước khi đổ sang. Công cần thực hiện tỉ lệ với độ nâng trọng tâm `Δh`. Với khối 1×1×2 (đơn vị cạnh = 1):

| Kiểu lăn | Trọng tâm ban đầu (độ cao) | Khoảng cách trọng tâm → cạnh quay | `Δh` |
|---|---|---|---|
| Đứng → nằm (đổ xuống) | 1 | √(1² + 0.5²) ≈ 1.118 | **≈ 0.118** |
| Nằm lăn ngang (vuông góc hướng lăn) | 0.5 | √(0.5² + 0.5²) ≈ 0.707 | **≈ 0.207** |
| Nằm → đứng (dựng lên) | 0.5 | √(0.5² + 1²) ≈ 1.118 | **≈ 0.618** |
| Cube 1×1×1 lăn (khi tách) | 0.5 | ≈ 0.707 | **≈ 0.207** (khối nhẹ bằng nửa) |

Đề xuất: `cost(a) = 1 + λ · Δh(a)` (ví dụ `λ = 1`), tức đổ xuống ≈ 1.118, lăn ngang ≈ 1.207, dựng đứng ≈ 1.618. Hằng số 1 đại diện cho "một thao tác của người chơi", phần `λ·Δh` đại diện cho "công vật lý". Để tránh lỗi so sánh số thực, có thể nhân lên thành số nguyên (ví dụ 1118 / 1207 / 1618) hoặc dùng `fractions.Fraction`.

→ Ý nghĩa: UCS sẽ ưu tiên lời giải **ít phải dựng khối đứng lên**, có thể khác lời giải ngắn nhất của BFS — đây chính là điểm thú vị để phân tích trong report (so sánh độ dài và tổng chi phí của BFS vs UCS).

**(b) Chi phí theo rủi ro / tình huống.**
- Lăn lên ô **fragile**: cộng thêm phạt (ví dụ +1) vì rủi ro thao tác sai làm vỡ ô.
- Nước đi **kích hoạt công tắc**: có thể đặt rẻ hơn nhưng **vẫn > 0** (ví dụ 0.5) — nhưng cần lý giải vì sao "rẻ" (khuyến khích khám phá cơ chế?). Lưu ý điều này cũng ảnh hưởng tới heuristic (chi phí nhỏ nhất `c_min` giảm).
- Di chuyển khi **đang tách**: điều khiển hai khối rời rạc phức tạp hơn → có thể đắt hơn.

**Bắt buộc ghi trong report:** công thức, bảng giá trị, lý do chọn, và chứng minh mọi chi phí dương.

### 5.4. A\* Search

- **Frontier:** heap theo `f = g + h`, phá hòa ưu tiên `h` nhỏ (hoặc `g` lớn) để tiến nhanh về đích.
- Với heuristic **consistent**, mỗi trạng thái chỉ cần mở rộng một lần (dùng closed set). Với heuristic chỉ **admissible**, phải cho phép mở lại node khi tìm thấy `g` tốt hơn (dùng `best_g`).
- **Heuristic phải admissible theo đúng hàm chi phí đã chọn ở UCS** (đề nhấn mạnh điều này).

**Cận dưới số nước đi — xây dựng từng bước.**

1. **Khoảng cách ô gần nhất.** Gọi `d(s) = min` khoảng cách Manhattan từ các ô mà khối chiếm tới ô đích. Từ bảng chuyển trạng thái: sau một lần lăn, mọi ô mới đều cách một ô cũ **không quá 2** (đổ xuống: ô xa nhất cách 2; dựng lên và lăn ngang: cách 1). Vậy mỗi nước đi `d` giảm tối đa 2, và đích có `d = 0` → **`h₁ = ⌈d/2⌉` là cận dưới của số nước đi** → admissible với chi phí đơn vị.
2. **Thay Manhattan bằng khoảng cách trên đồ thị ô (relaxed "point-object", đề gợi ý).** Chạy BFS **một lần** từ ô đích trên đồ thị 4-kề gồm mọi ô **có thể** là sàn (floor, goal, fragile, switch, **mọi cầu coi như mở**), bỏ qua void. Gọi `dist_G(x)`. Vì chuỗi các ô khối chiếm luôn tạo thành đường 4-kề qua các ô có sàn, lập luận ở bước 1 vẫn đúng → `h₂ = ⌈min dist_G / 2⌉` admissible và **chặt hơn** `h₁` (tính đến tường void).
3. **Thành phần hướng (orientation-aware).** Nếu khối chưa đứng trên đích thì cần ít nhất 1 nước → dùng **`max(h₂, 1)`** cho mọi trạng thái không phải đích.

> ⚠️ **Bẫy "cộng thêm phạt hướng":** công thức `⌈d/2⌉ + 1` khi khối đang nằm **KHÔNG admissible**. Phản ví dụ: khối nằm dọc chiếm `(r-2, c)` và `(r-1, c)`, đích ở `(r, c)`: `d = 1` → `⌈1/2⌉ + 1 = 2`, nhưng chỉ cần **1** nước (Down → đứng tại `(r, c)`). Phải dùng `max`, hoặc chứng minh cẩn thận nếu dùng tổng.

**Bẫy lớn: split switch phá vỡ tính admissible của heuristic khoảng cách.** Split switch **dịch chuyển tức thời** khối tới vị trí có thể rất gần đích → trong một nước đi, `d` có thể giảm rất nhiều → `⌈d/2⌉` có thể **đánh giá quá cao**. Cách sửa: khi tính `dist_G`, thêm **cạnh "cổng dịch chuyển" chi phí 0** từ ô split switch tới các ô đích dịch chuyển của nó (tính bằng Dijkstra/0-1 BFS ngược từ đích). Khi đó `dist_G(switch) ≤ dist_G(target)`, nên nước đi kích hoạt split vẫn chỉ làm giảm `min dist_G` tối đa 2 → **giữ nguyên tính admissible**. Ở chế độ tách, lấy `min` trên hai cube (mỗi nước cube chỉ làm giảm tối đa 1).

**Chuyển sang hàm chi phí không đồng nhất (5.3a).** Gọi `c_min` là chi phí nhỏ nhất của một hành động (≈ 1.118) và `c_up` là chi phí dựng đứng (≈ 1.618). Nhận xét: trạng thái đích (khối nguyên đứng) **chỉ** đạt được bằng nước **dựng đứng** (đổ xuống tạo ra khối nằm, lăn ngang giữ nguyên nằm, gộp cube tạo khối nằm). Vậy với trạng thái không phải đích:

```
n(s) = max(⌈dist_G(s) / 2⌉, 1)          # cận dưới số nước đi
h(s) = c_up + c_min · (n(s) − 1)         # nước cuối chắc chắn là dựng đứng
h(goal) = 0
```

- **Admissible:** mọi đường tới đích có ≥ `n(s)` nước, nước cuối tốn ≥ `c_up`, các nước còn lại mỗi nước ≥ `c_min`.
- **Consistent:** `n` giảm tối đa 1 sau mỗi nước nên `h(s) − h(s') ≤ c_min ≤ cost(s, a, s')`; với nước đi vào đích, `n(s) = 1` nên `h(s) = c_up ≤ cost`.
- **Lưu ý:** nếu thêm chi phí "rẻ" cho nước bấm công tắc (5.3b), `c_min` phải là **giá trị nhỏ nhất thực sự**, nếu không heuristic sẽ mất admissible.
- Nếu `dist_G = ∞` (ô của khối không nối được tới đích ngay cả khi mở mọi cầu) → trạng thái **chết**, có thể đặt `h = ∞` và cắt tỉa an toàn.

**Hướng nâng cao (nếu dư thời gian):** heuristic dạng **pattern database** — BFS/Dijkstra ngược từ trạng thái đích trên đồ thị **trạng thái khối** (có hướng) với mọi cầu mở, fragile coi như sàn thường, có thêm cạnh split. Đây là bài toán nới lỏng nên khoảng cách chính xác trên đó là heuristic **admissible và consistent**, chặt hơn nhiều so với Manhattan. Report có thể so sánh 2–3 heuristic (số node mở rộng của A* với `h = 0` ≡ UCS, `h₁`, `h₂`, PDB) — đây là phần thảo luận ăn điểm cao.

### 5.5. Bảng so sánh lý thuyết (dùng cho report)

| Thuật toán | Đầy đủ | Tối ưu | Thời gian | Bộ nhớ | Ghi chú |
|---|---|---|---|---|---|
| BFS | Có (hữu hạn) | Theo số nước | `O(b^d)` | `O(b^d)` | Baseline tối ưu số nước |
| DFS + visited | Có (hữu hạn) | Không | `O(\|S\|)` | `O(\|S\|)` (do visited) | Lời giải dài, phụ thuộc thứ tự hành động |
| IDS | Có | Theo số nước | `O(b^d)` (mở lại nhiều) | `O(b·d)` (nếu chỉ path-check) | Tiết kiệm bộ nhớ, chậm hơn |
| UCS | Có (c ≥ ε) | Theo tổng chi phí | `O(b^{1+⌊C*/ε⌋})` | như thời gian | Khác BFS khi chi phí không đồng nhất |
| A\* | Có | Có (h admissible) | Tùy chất lượng h | Lưu mọi node sinh ra | Ít node nhất nếu h tốt |

---

## 6. Kiến trúc phần mềm (đã chốt)

### 6.1. Cấu trúc thư mục (gốc repo = thư mục `Source/` khi nộp)

```
CSAI_Group9_Lab01/
├── main.py                  # CLI: python main.py code/data/1.data --algo astar [--out f] [--compare] [--memory]
├── ai/                      # Phần AI — Python >= 3.10, chỉ thư viện chuẩn
│   ├── level.py             # Level: đọc .data đúng định dạng Floor::initialize + validate
│   ├── state.py             # State(x, y, orientation, bridge_mask), Orientation, ACTIONS
│   ├── problem.py           # BloxorzProblem: luật game chép CHÍNH XÁC từ C++
│   ├── heuristics.py        # zero_heuristic (có sẵn), manhattan_heuristic (TODO)
│   └── solvers/
│       ├── base_solver.py   # BaseSolver (đo thời gian, bộ nhớ, reconstruct_path), SearchResult
│       ├── bfs_solver.py    # TODO
│       ├── dfs_solver.py    # TODO
│       ├── ucs_solver.py    # TODO
│       └── astar_solver.py  # TODO
├── tests/                   # pytest: test_problem.py (luật — PASS), test_solvers.py (SKIP tới khi cài)
├── code/                    # Game 3D C++/OpenGL: code gốc chaitanya + phần nhóm sửa
│   ├── main.cpp             # ./a.out [level] [--solve <algo> | --play <file>]
│   ├── autoplay.h/.cpp      # phát lại lời giải, gọi solver Python
│   ├── game.cpp, tile.cpp, superblock.cpp, ...   # luật + vẽ
│   ├── data/*.data          # màn chơi
│   ├── include/             # header GLAD (OpenGL 3.3 core) + KHR
│   └── makefile             # Linux/WSL, macOS (Homebrew), MSYS2/MinGW
├── Docs/                    # đề bài, report0, task, implementation_plan
├── README.md, THIRD_PARTY_NOTICES.md, requirements.txt, pyproject.toml
└── demo/                    # gif/mp4 của repo gốc — không đưa lên git
```

### 6.2. Luồng chạy C++ ↔ Python

```
./a.out 2 --solve astar                         (chạy trong thư mục code/)
  │ 1. system("python3 ../main.py data/2.data --algo astar --out solution.txt")
  ▼
main.py → Level.from_file → BloxorzProblem → AStarSolver.run()
  │ 2. kiểm tra lời giải bằng problem.is_valid_solution(), ghi "R U U R ..." ra solution.txt
  ▼
AutoPlayer.load("solution.txt") → mở cửa sổ → khối đứng yên đủ 0.25 s thì superblock.move(...)
  │ 3. Game::check mỗi frame → in "WON"/"GAME OVER", giữ màn hình 1.5 s rồi thoát (mã 0 / 1)
```

- Biến môi trường `BLOXORZ_PYTHON` đổi trình thông dịch (mặc định `python3` trên Linux/macOS, `python` trên Windows).
- Mã thoát của `main.py`: 0 có lời giải, 1 vô nghiệm, 2 solver chưa cài, 3 lời giải sai luật, 4 lỗi đọc màn.
- **Rủi ro lớn nhất: luật nằm ở hai nơi** (C++ để chơi và vẽ, Python để tìm kiếm). Quy tắc của nhóm: mọi thay đổi luật phải sửa **đồng thời** `code/*.cpp` và `ai/problem.py`, thêm test vào `tests/test_problem.py`, rồi kiểm chứng bằng `./a.out <n> --play <lời giải>`.

### 6.3. Định dạng màn chơi

Dùng định dạng `.data` của game nền (mục 2.6). Để đáp ứng đủ đề (một công tắc nhiều ô cầu, permanent, split), nhóm sẽ mở rộng thành **định dạng v2 tương thích ngược**, ví dụ thêm phần mở rộng ở cuối file:

```
# ... phần v1 giữ nguyên ...
ext 1                          # có phần mở rộng
switch normal 2 5 toggle 2     # loại, vị trí, chế độ (toggle | on | off), số ô cầu
6 4  6 5                       # danh sách ô cầu
split 8 3  4 9  10 9           # split switch tại (8,3) -> 2 cube tại (4,9) và (10,9)
```

> Cú pháp trên chỉ là phác thảo. Nhóm chốt cú pháp trong `Docs/implementation_plan.md` trước khi cài; parser C++ (`Floor::initialize`) và Python (`Level.from_text`) phải đọc giống hệt nhau.

### 6.4. GUI 3D

- **Đã có:** cảnh 3D (OpenGL 3.3), animation lăn, 6 chế độ camera (phím X/C/V/B/N/M), bản đồ nhỏ góc trái dưới, điều khiển bằng phím mũi tên, **tự phát lại lời giải** (`--solve`, `--play`), giữ màn hình 1.5 s khi thắng/thua, khóa phím khi đang phát lại.
- **Cần thêm:** nút **New Game / chọn màn**, **Restart**, **Solve BFS/DFS/UCS/A\*** và panel số liệu (gợi ý Dear ImGui — license MIT, có sẵn backend GLFW + OpenGL3); hiển thị hai cube khi split; chạy solver không làm treo cửa sổ (`std::thread`).

---

## 7. Thiết kế bộ test và thực nghiệm

### 7.1. Bộ test cố định theo nhóm độ khó

| Nhóm | Mô tả | Số màn gợi ý | Mục đích |
|---|---|---:|---|
| **G1 – Basic** | Chỉ floor/void/goal | 3–4 | Kiểm chứng core; có thể tái hiện Level 1 của Coolmath |
| **G2 – Fragile** | Thêm ô fragile | 2–3 | Ràng buộc "không được đứng" làm đổi lời giải |
| **G3 – Switches & Bridges** | Soft/heavy, toggle/permanent | 3–4 | Không gian trạng thái nhân `2^k`; cần quay lại vị trí cũ với cầu khác |
| **G4 – Split** | Split switch, tách/gộp | 2–3 | Không gian trạng thái `T²`, hệ số nhánh 8 |
| **G5 – Combined / Hard** | Kết hợp mọi cơ chế, màn lớn | 3–4 | Thấy rõ khác biệt BFS vs A*, DFS cho lời giải dài |
| **G6 – Special** | Màn **vô nghiệm**; màn lớn trống trải (open field 20×30) | 2 | Kiểm tra dừng đúng; stress-test DFS/bộ nhớ |

Mỗi màn nên ghi rõ trong report: kích thước, số ô sàn, số cầu/công tắc, độ dài lời giải tối ưu (từ BFS), và hình minh họa.

> **Theo Q&A: tối thiểu 10 test case (= 10 màn).** Hiện repo có 2 màn của game nền: `code/data/1.data` (13×13, 2 công tắc thường + 1 công tắc nặng, có fragile; tối ưu **10 bước**, 161 trạng thái đạt được) và `code/data/2.data` (19×14, 3 công tắc nặng, nhiều fragile; tối ưu **68 bước**, 378 trạng thái). Cả hai màn đều nhỏ, nên cần thêm màn lớn và nhiều công tắc ở G5/G6 để thấy rõ khác biệt giữa các thuật toán.

### 7.2. Quy trình đo

- Chạy mỗi cặp (solver, màn) **nhiều lần** (ví dụ 5 lần) và lấy **trung vị** thời gian.
- **Đo thời gian và bộ nhớ ở hai lượt chạy riêng:** `tracemalloc` làm chậm Python đáng kể nên nếu bật cùng lúc thì số liệu thời gian bị sai lệch.
- Tắt GUI khi đo; không in log trong vòng lặp tìm kiếm.
- Đặt **timeout** (ví dụ 60 giây) và/hoặc **giới hạn node**; ghi "TIMEOUT" thay vì để chương trình chạy mãi.
- Ghi nhận cấu hình máy (CPU, RAM, phiên bản Python) vào report.
- Xuất toàn bộ kết quả ra `results.csv` → bảng và biểu đồ được sinh tự động, tái lập được.

### 7.3. Trình bày kết quả

- **Bảng:** mỗi màn × mỗi thuật toán: time, peak memory, expanded, length (+ cost cho UCS/A*).
- **Biểu đồ:**
  - Grouped bar chart số node mở rộng theo màn (**trục log**).
  - Grouped bar chart thời gian và bộ nhớ.
  - Biểu đồ độ dài lời giải: DFS vs các thuật toán tối ưu.
  - (Mở rộng) A* với các heuristic khác nhau: `h = 0`, `h₁`, `h₂`, PDB.
- **Nhận xét định hướng (để kiểm chứng bằng số liệu thật, không viết trước):**
  - BFS và A* (chi phí đơn vị) luôn cho độ dài tối ưu như nhau; A* mở rộng ít node hơn.
  - DFS có thể tìm ra lời giải nhanh trên một số màn nhưng thường dài hơn rất nhiều.
  - UCS với chi phí năng lượng có thể chọn lời giải **dài hơn về số nước nhưng rẻ hơn về chi phí** so với BFS.
  - Trên màn có nhiều cầu, số trạng thái tăng theo `2^k` và khoảng cách giữa các thuật toán tăng rõ.
  - Phân tích **vì sao** heuristic yếu ở màn có split hoặc có cầu chắn (relaxation bỏ qua các ràng buộc này).

---

## 8. Kế hoạch thực hiện theo Sprint

Nhóm **5 thành viên**, thời gian **21 ngày**. Mỗi người phụ trách **một file solver**, người thứ năm phụ trách **heuristic và báo cáo so sánh**; ngoài ra mỗi người nhận thêm một mảng của game/báo cáo. Danh sách task chi tiết (ID, giờ, hạn, phụ thuộc) ở `Docs/task.md`.

### 8.1. Phân vai

| Vai trò | Solver | Mảng phụ trách thêm | Phần report |
|---|---|---|---|
| **TV1 – BFS & Luật game** | `bfs_solver.py` | Mở rộng luật ở **cả C++ và Python**: định dạng `.data` v2, nhiều ô cầu/công tắc, permanent, **split** (logic) + test | Luật chơi + Hình thức hóa bài toán |
| **TV2 – DFS & GUI** | `dfs_solver.py` (IDS tùy chọn) | GUI 3D: nút New Game/Restart/Solve, chọn màn, panel số liệu, vẽ cube khi split | Cài đặt & GUI; **quay video** |
| **TV3 – UCS & Chi phí** | `ucs_solver.py` | Hàm chi phí không đồng nhất (+ tùy chọn ML suy trọng số), quy trình đo time/memory | UCS + hàm chi phí |
| **TV4 – A\* & Màn chơi** | `astar_solver.py` | Thiết kế **≥ 10 màn** G1–G6, ảnh minh họa | A\* + mô tả bộ test; kịch bản video |
| **TV5 – Heuristic, Thực nghiệm & PM** | `heuristics.py` | Heuristic + chứng minh admissible/consistent; `experiments/` (chạy, CSV, biểu đồ); Notion; ráp report | Heuristic, Experiments, Planning, References |

> **Mọi thành viên đều phải hiểu toàn bộ hệ thống** (mô hình trạng thái, luật, 4 thuật toán, heuristic, hàm chi phí), vì giảng viên có thể vấn đáp bất kỳ ai.

### 8.2. Tổng quan timeline

```
Ngày:        1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21
Sprint 0   [███]                                          (khung repo — phần lớn đã xong)
Sprint 1       [███████████████]                          4 solver + heuristic cơ bản
Sprint 2                       [███████████████]          luật nâng cao (v2, permanent, split)
Sprint 3                                       [███████████████]   GUI nút bấm + đủ 10 màn
Sprint 4                                                       [███████████████]   thực nghiệm
Sprint 5                                                                       [███████████]
Buffer                                                                                 [██]
```

### Sprint 0 — Khởi động & khung repo (Ngày 1–2)

- [x] Tạo repo, import code gốc chaitanya @ `c0ccb55` thành commit mốc, cite đầy đủ.
- [x] Khung `ai/` (Level, State, BloxorzProblem chép đúng luật C++), CLI `main.py`, test luật (PASS).
- [x] Game: `autoplay`, `--solve`, `--play`, GLAD 3.3, makefile đa nền tảng; build và chạy được trên WSL.
- [ ] **Cả 5 người** build và chạy được `./a.out 1 --play` trên máy mình; chạy được `pytest`.
- [ ] Cả nhóm đọc `README.md`, mục 2.6 và `Docs/implementation_plan.md`; chốt cú pháp `.data` v2.

**Definition of Done:** ai cũng giải thích lại được mô hình trạng thái (`bridge_mask`) và bảng chuyển theo tọa độ code.

### Sprint 1 — Bốn solver + heuristic cơ bản (Ngày 2–6)

- [ ] **TV1:** BFS · **TV2:** DFS · **TV3:** UCS (chi phí 1 trước) · **TV4:** A\* (thử với `zero_heuristic`) · **TV5:** `manhattan_heuristic` + bản nháp chứng minh.
- [ ] Mỗi solver cập nhật đủ bộ đếm (`nodes_expanded`, `nodes_generated`, `max_frontier`).
- [ ] **TV4:** thiết kế 3–4 màn G1/G2 bản đầu.

**Definition of Done:** `pytest` không còn SKIP ở `test_solvers.py`; BFS/UCS/A\* cho đúng 10 và 68 bước; `./a.out 2 --solve <algo>` chạy được với cả 4 thuật toán.

### Sprint 2 — Luật nâng cao (Ngày 6–10)

- [ ] **TV1:** định dạng `.data` v2 (nhiều ô cầu, permanent on/off) ở cả C++ và Python + test.
- [ ] **TV1 + TV2:** split switch — TV1 làm logic (C++ `Game`, Python `State` mở rộng hai cube, 8 hành động, tự gộp), TV2 làm phần vẽ và phím Space.
- [ ] **TV3:** chốt hàm chi phí không đồng nhất (mục 5.3) + viết lý giải.
- [ ] **TV4:** màn G3 (công tắc/cầu) và G4 (split) bản nháp.
- [ ] **TV5:** heuristic cho màn có split (cổng dịch chuyển, mục 5.4); khung `experiments/run_all.py`.

**Definition of Done:** chơi tay được màn có split/permanent; solver giải được các màn đó; test luật mới PASS ở cả hai phía.

### Sprint 3 — GUI nút bấm + đủ 10 màn (Ngày 10–14)

- [ ] **TV2:** nút New Game/chọn màn, Restart, Solve BFS/DFS/UCS/A\*, panel số liệu; solver chạy không làm treo cửa sổ.
- [ ] **TV4:** hoàn thiện **≥ 10 màn** (G1–G6), mỗi màn có lời giải tối ưu ghi trong `levels/README` hoặc `code/data/README.md`.
- [ ] **TV3:** đo bộ nhớ đỉnh tách lượt, kiểm tra chi phí A\* = UCS trên mọi màn.
- [ ] **TV5:** `run_all.py` xuất CSV (lặp nhiều lần, lấy trung vị).
- [ ] **TV1:** test so khớp luật C++ ↔ Python trên toàn bộ màn (phát lại lời giải của solver bằng `--play`).

**Definition of Done:** từ GUI bấm được cả 4 solver trên mọi màn và xem khối tự lăn tới đích.

### Sprint 4 — Thực nghiệm, đánh bóng & bug bash (Ngày 14–18)

- [ ] **TV5:** chạy thực nghiệm chính thức, vẽ biểu đồ (trục log), tổng hợp bảng.
- [ ] **Cả nhóm:** bug bash — chơi chéo màn của nhau; sửa bug.
- [ ] **TV2:** đánh bóng GUI (chú thích loại ô, màu, bố cục) để quay video.
- [ ] **TV3 + TV4:** phân tích số liệu UCS/A\*, viết nhận xét điểm mạnh/yếu.

**Definition of Done:** `results.csv` + biểu đồ cuối cùng; code freeze.

### Sprint 5 — Report, video, đóng gói (Ngày 18–21)

- [ ] **Cả nhóm:** viết report theo dàn ý 8.3 (mỗi người phần của mình, xem 8.1); đọc chéo.
- [ ] **TV2:** quay video theo 8.4, có phụ đề/thuyết minh, upload YouTube (**Public**).
- [ ] **TV5:** ráp `Report.pdf`, đóng gói `StudentID1_..._StudentID5` (gốc repo → `Source/`, bỏ `demo/`, file build).
- [ ] **Cả nhóm:** thống nhất **% đóng góp**; **luyện vấn đáp**.
- [ ] **Buffer (Ngày 20–21):** dành cho sự cố; nộp sớm.

### 8.3. Dàn ý Report.pdf đề xuất

1. **Thông tin nhóm** + link video YouTube (ngay trang đầu).
2. **Project Planning & Task Distribution:** bảng phân công, timeline, **% hoàn thành mỗi người**.
3. **Giới thiệu bài toán Bloxorz** và luật chơi (kèm ảnh chụp GUI).
4. **Hình thức hóa bài toán tìm kiếm:** trạng thái (hướng + cầu + tách), hành động, mô hình chuyển trạng thái (bảng 12 ô + các cơ chế), goal test, hàm chi phí, cách tránh lặp, ước lượng kích thước không gian trạng thái, **các quyết định thiết kế** cho chỗ đề chưa nói rõ.
5. **Mô tả thuật toán:** BFS, DFS (hoặc IDS — ghi rõ), UCS (định nghĩa + lý giải hàm chi phí), A\* (thiết kế heuristic + chứng minh admissible/consistent theo đúng hàm chi phí); pseudocode; tính chất lý thuyết.
6. **Cài đặt:** kiến trúc code, định dạng màn, GUI.
7. **Thực nghiệm:** môi trường máy, mô tả các nhóm test, phương pháp đo, bảng, biểu đồ, thảo luận điểm mạnh/yếu.
8. **Kết luận & hướng phát triển.**
9. **Tài liệu tham khảo:** sách (Russell & Norvig — *AIMA*), slide môn học, trang game Coolmath, tài liệu thư viện, các repo GitHub đã tham khảo (nếu có).
10. **Phụ lục:** cách nhóm dùng AI tạo sinh (công cụ nào, cho việc gì, các prompt tiêu biểu, phần nào tự viết/kiểm chứng).

### 8.4. Kịch bản video gợi ý (5–8 phút)

| Thời điểm | Nội dung | Hạng mục điểm |
|---|---|---|
| 0:00–0:30 | Giới thiệu nhóm, đề tài | — |
| 0:30–1:30 | Tổng quan GUI 3D: bàn chơi, **chú thích từng loại ô** (floor, void, goal, fragile, các switch, cầu mở/đóng), các chế độ camera, các nút | GUI (5đ) |
| 1:30–3:00 | Chơi tay: lăn qua 3 hướng, đứng trên fragile → thua, bấm soft/heavy switch (toggle/permanent) → cầu đổi, split → điều khiển 2 cube → gộp, thắng màn | Feature (5đ) |
| 3:00–5:30 | Chạy BFS / DFS / UCS / A\* trên cùng một màn, hiển thị số liệu, animate lời giải; chỉ ra DFS dài hơn, A\* mở ít node hơn, UCS khác BFS | Feature (5đ) |
| 5:30–6:30 | Tóm tắt nhanh kết quả thực nghiệm (biểu đồ) | — |

Có **phụ đề hoặc thuyết minh** xuyên suốt.

---

## 9. Các lưu ý quan trọng

### 9.1. Những điểm đề bài chưa nói rõ — cần chốt và ghi vào report

| # | Câu hỏi | Đề xuất của nhóm |
|---:|---|---|
| 1 | Công tắc kích hoạt khi nào? | Khi **kết thúc một nước đi** mà khối/cube đè lên nó (mọi nước đi đều rời khỏi toàn bộ ô cũ nên không có trường hợp "đứng yên trên công tắc") |
| 2 | Khối đè lên 2 công tắc cùng lúc? | Kích hoạt **cả hai**, theo thứ tự id cố định |
| 3 | Cầu đóng lại ngay dưới khối? | Khối **rơi** → nước đi không hợp lệ (bước 7 của transition) |
| 4 | Permanent nghĩa là gì? | Hai biến thể `open` (luôn mở) và `close` (luôn đóng); bấm lại không có tác dụng |
| 5 | Cube 1×1×1 đứng trên fragile? | **An toàn**: áp lực của cube (1 đơn vị/ô) bằng áp lực của khối nằm (2 đơn vị trên 2 ô) — lập luận vật lý nhất quán. (Hoặc chọn ngược lại, miễn là ghi rõ) |
| 6 | Cube đi vào ô goal? | Goal hoạt động như sàn thường với cube (không thắng, không rơi) |
| 7 | Cube trên heavy / split switch? | Không kích hoạt (theo đề); cube trên soft → kích hoạt |
| 8 | Khi nào gộp cube? | Ngay sau nước đi làm hai cube **4-kề** nhau → khối nằm H/V tương ứng |
| 9 | Thao tác đổi cube có tính là một nước? | Theo phương án A (mục 3.2): không — solver dùng 8 hành động; GUI tự chèn khi replay |
| 10 | Split switch có liên kết cầu không? | Cho phép tùy chọn trong JSON, mặc định không |
| 11 | Vị trí dịch chuyển của split không hợp lệ? | Bắt lỗi khi **validate màn**, không xử lý lúc chạy |
| 12 | Khối nằm vắt qua goal? | Hợp lệ (goal chịu lực như sàn), nhưng không thắng |

> **Game nền đã chốt sẵn một số điểm** (mục 2.6), nhóm giữ nguyên để C++ và Python khớp nhau: công tắc kích hoạt ở cuối nước đi và **không** kích hoạt ở nước thắng/thua (#1); khối đè hai công tắc thì kích hoạt cả hai theo thứ tự ô đầu → ô sau → công tắc nặng (#2); cầu biến mất dưới khối → rơi (#3); nằm vắt qua lỗ đích không thắng, không thua (#12). Các điểm #4–#11 sẽ chốt khi cài permanent và split.

### 9.2. Bẫy kỹ thuật thường gặp

1. **`visited` thiếu BridgeConfig** → bỏ sót lời giải ở màn có công tắc toggle.
2. **Trạng thái mutable** (list, dict) làm khóa `set` → lỗi hoặc bug khó tìm. Dùng tuple/frozen dataclass; **không bao giờ sửa `Level` trong lúc tìm kiếm**.
3. **Biểu diễn khối không chuẩn hóa** (cùng một vị trí có 2 cách viết) → trùng trạng thái, số node bị thổi phồng.
4. **UCS/A\* kiểm tra đích lúc sinh node** → mất tính tối ưu.
5. **`heapq` so sánh `State`** khi `f` bằng nhau → `TypeError`. Luôn thêm `counter`.
6. **DFS đệ quy** → `RecursionError` trên màn lớn.
7. **Heuristic cộng thêm phạt hướng** (`⌈d/2⌉ + 1`) → không admissible (phản ví dụ ở 5.4).
8. **Quên split teleport khi chứng minh admissible** → heuristic đánh giá quá cao trên màn có split.
9. **Đổi hàm chi phí nhưng không sửa heuristic** → A\* không còn tối ưu. Luôn chạy **test chéo chi phí A\* == UCS**.
10. **Chi phí 0 hoặc âm** (ví dụ "bấm công tắc miễn phí") → UCS/A\* sai.
11. **Đo bộ nhớ và thời gian cùng lúc với `tracemalloc`** → thời gian bị đội lên.
12. **Đo cả thời gian nạp màn/vẽ GUI** vào search time → số liệu không công bằng.
13. **Đếm expanded nodes không thống nhất** giữa các thuật toán (có/không tính node đích, node "stale" trong heap) → so sánh sai. Định nghĩa một lần, ghi trong report.
14. **Solver chạy trên luồng GUI** → cửa sổ "Not Responding" khi quay video.
15. **Logic luật bị nhân đôi** (GUI một bản, solver một bản) → hai bên lệch nhau. Với kiến trúc C++ + Python đã chốt, việc nhân đôi là không tránh được → bắt buộc theo quy trình ở mục 9.6.1 (sửa cả hai + test + phát lại bằng `--play`).
16. **Số thực trong chi phí** → so sánh `==` sai; dùng số nguyên đã nhân hệ số hoặc `Fraction`, hoặc so sánh có sai số.

### 9.3. Lưu ý về report & thực nghiệm

- Experiments chiếm **20 điểm** — đầu tư đúng mức: bộ test cố định, có mô tả nhóm, có bảng, có biểu đồ, có **phân tích** (không chỉ liệt kê số).
- Màn quá nhỏ → mọi thuật toán đều ~0 ms, không có gì để phân tích. Cần có màn đủ lớn/khó.
- **Phần UCS:** phải định nghĩa và **lý giải** hàm chi phí; nếu chỉ dùng chi phí 1 sẽ mất ý nghĩa (đề nói rõ).
- **Phần A\*:** phải có lập luận admissible/consistent **chặt chẽ** — nên viết dưới dạng mệnh đề + chứng minh ngắn.
- Ghi rõ nếu dùng **IDS** thay DFS.
- Ảnh chụp GUI, hình minh họa các màn chơi giúp report dễ đọc hơn nhiều.
- Report phải là **PDF**, trình bày đẹp, có mục lục, đánh số hình/bảng.

### 9.4. Lưu ý về làm việc nhóm & liêm chính học thuật

- **Điểm cá nhân = điểm nhóm × % hoàn thành** → ghi nhận đóng góp **công bằng, chính xác** ngay từ đầu (dùng lịch sử commit, bảng task) để tránh tranh cãi cuối kỳ.
- Dùng AI được phép nhưng **phải hiểu từng dòng code mình nộp**; giảng viên có thể **vấn đáp ngẫu nhiên**. Ghi nhận việc dùng AI trung thực trong phụ lục.
- Nếu tham khảo repo/tài liệu bên ngoài → **ghi nguồn đầy đủ**. Đạo văn/gian lận = **0 điểm toàn môn**.
- Dùng Git ngay từ ngày đầu; mỗi người làm trên branch riêng, merge qua review ngắn.

### 9.5. Lưu ý về nộp bài

- Tên file nén đúng định dạng `StudentID1_StudentID2_...`.
- Thư mục `Source/` có `main.py`, `README.md`, `requirements.txt`.
- **Không** đưa `venv/`, `__pycache__/`, `.git/`, file video vào bản nén (giữ dưới 20MB).
- Link video phải mở được khi **không đăng nhập** (kiểm tra bằng cửa sổ ẩn danh).
- Nộp **sớm**; sau deadline không được sửa gì.
- Gốc repo chính là thư mục `Source/`; khi nén bỏ `demo/` (~30MB), `.git/`, `*.o`, `a.out`.

### 9.6. Lưu ý kỹ thuật với game nền C++

1. **Luật ở hai nơi** (`code/*.cpp` và `ai/problem.py`): sửa luật thì sửa cả hai + thêm test + chạy `./a.out <n> --play <lời giải>` để so khớp.
2. **Chạy game trong thư mục `code/`**: game đọc `data/<n>.data` và shader `Sample_GL.*` theo đường dẫn tương đối; solver được gọi bằng `../main.py`.
3. **Tọa độ dễ nhầm:** `R` tăng `x` = đi **xuống một dòng** trong file `.data`; `U` tăng `y` = sang **phải một cột**.
4. **WSLg** có thể in cảnh báo `MESA`/`libEGL` khi mở cửa sổ — vô hại (driver chuyển sang render bằng phần mềm).
5. **Xuống dòng LF:** repo có `.gitattributes` ép LF để `make` và file `.data` chạy được trong WSL; đừng để editor đổi sang CRLF.
6. **Ô cầu ban đầu phải là 0** trong lưới (C++ bật cầu luôn tạo ô thường type 1); `Level.validate()` sẽ báo lỗi nếu vi phạm.
7. **Thân các solver/heuristic là phần nhóm tự làm** (yêu cầu học thuật + vấn đáp): không dùng AI sinh nguyên thân thuật toán; nếu có tham khảo thì ghi vào phụ lục.

---

## 10. Checklist trước khi nộp

**Game (20đ)**
- [ ] Floor / void / goal, khối 3 hướng, 4 hướng lăn đúng bảng chuyển
- [ ] Phát hiện rơi khỏi bàn / rơi vào void; phát hiện thắng (chỉ khi đứng trên goal)
- [ ] Fragile; bridge mở/đóng; soft, heavy, split switch; toggle **và** permanent
- [ ] Tách thành 2 cube, đổi cube bằng Space, tự gộp khi kề nhau
- [ ] GUI **3D** có New Game, Restart, nút Solve BFS/DFS/UCS/A\*
- [x] Bấm Solve → khối tự lăn tới đích (`--solve` / `--play`; còn thiếu nút bấm trong GUI)
- [x] Màn chơi nạp từ file `.data`, dùng chung cho C++ và mọi solver Python
- [ ] **≥ 10 màn** (test case) chia nhóm độ khó
- [ ] Ghi nguồn repo `chaitanya100100/Bloxorz` @ `c0ccb55` trong README, THIRD_PARTY_NOTICES và References

**Solvers (40đ)**
- [ ] BFS, DFS (hoặc IDS — đã ghi rõ), UCS, A\* — tự cài đặt
- [ ] Trạng thái bao gồm hướng khối **và** cấu hình cầu (và trạng thái tách)
- [ ] Tránh lặp trạng thái ở mọi solver
- [ ] Mọi solver xử lý đúng các ô nâng cao
- [ ] Ghi nhận Search Time, Peak Memory, Expanded Nodes, Solution Length cho mọi solver
- [ ] Test chéo: A\*(đơn vị) = BFS về độ dài; A\* = UCS về chi phí

**Report (30đ)**
- [ ] Phân công + % hoàn thành từng thành viên
- [ ] Hình thức hóa bài toán đầy đủ (trạng thái, hành động, transition, goal, cost, heuristic, chống lặp)
- [ ] Hàm chi phí UCS được định nghĩa và lý giải
- [ ] Heuristic A\* được giải thích + chứng minh admissible/consistent theo hàm chi phí đã chọn
- [ ] Bộ test cố định chia nhóm độ khó, có mô tả
- [ ] Bảng + biểu đồ + nhận xét điểm mạnh/yếu
- [ ] Tài liệu tham khảo + phụ lục dùng AI
- [ ] Link video YouTube công khai

**Video (10đ)**
- [ ] Trình bày GUI và mọi loại ô, khối ở các hướng, các nút
- [ ] Chơi tay: lăn, bấm công tắc, thắng, thua
- [ ] Chạy solver và animate lời giải
- [ ] Có phụ đề hoặc thuyết minh

**Nộp bài**
- [ ] Cấu trúc thư mục đúng, `README.md`, `requirements.txt`
- [ ] Chạy thử trên máy sạch thành công
- [ ] File nén đúng tên, < 20MB (hoặc link Drive)
- [ ] Nộp trước deadline

---

*Tài liệu này là bản phân tích định hướng (report 0). Các con số thực nghiệm, hàm chi phí và heuristic cuối cùng sẽ được chốt và kiểm chứng trong quá trình thực hiện, và trình bày chính thức trong `Report.pdf`.*
