# Danh sách task — Project 1: Bloxorz Solver (nhóm 5 thành viên)

> Tài liệu này tách kế hoạch 6 sprint trong `report0.md` thành **các task cụ thể** để đưa vào Notion và phân công.
> Thời gian tính theo **Ngày 1 → Ngày 21** (Ngày 1 = ngày bắt đầu làm). Khi đưa vào Notion, đổi "Ngày N" thành ngày thật trên lịch.

---

## 1. Hướng dẫn đưa vào Notion

**Cách nhanh nhất:**
1. Copy **bảng tổng hợp ở mục 4** → dán vào một trang Notion → Notion tạo một *simple table*.
2. Bấm menu `⋯` của bảng → **Turn into database** để có thể lọc, nhóm, xem dạng Board/Timeline.
3. Với từng task, mở trang con và dán phần **mô tả + tiêu chí hoàn thành** ở mục 5 (các dòng `- [ ]` sẽ thành to-do).

**Các thuộc tính (property) nên tạo trong database:**

| Property | Kiểu | Giá trị |
|---|---|---|
| `ID` | Text | `S0-01`, `S1-02`, ... |
| `Task` | Title | Tên task |
| `Sprint` | Select | Sprint 0 … Sprint 5 |
| `Người phụ trách` | Person | TV1 … TV5 (hoặc "Tất cả") |
| `Ưu tiên` | Select | **P0** (bắt buộc, ảnh hưởng điểm) · **P1** (cần có) · **P2** (mở rộng, làm nếu dư thời gian) |
| `Ước lượng (h)` | Number | Số giờ ước tính |
| `Hạn` | Date | Ngày N → đổi sang ngày thật |
| `Phụ thuộc` | Relation (tới chính database) | Các task phải xong trước |
| `Hạng mục điểm` | Multi-select | Game-Core · Game-Adv · BFS · DFS · UCS · A* · Report-Algo · Report-Exp · Report-Plan · Video · Nộp bài · Quản lý |
| `Trạng thái` | Status | Not started · In progress · In review · Done |

**View gợi ý:** Board theo `Trạng thái` (dùng hằng ngày) · Board theo `Người phụ trách` (xem tải công việc) · Timeline theo `Hạn` (kiểm tra tiến độ) · Filter `Ưu tiên = P0` (những gì không được trễ).

---

## 2. Phân vai 5 thành viên

| Mã | Vai trò | Phụ trách chính | Phần report | Phần video |
|---|---|---|---|---|
| **TV1** | **Engine Lead** | `core/`: State, bảng chuyển trạng thái, luật chơi, mọi ô nâng cao (fragile, cầu, 3 loại switch, tách/gộp) | Giới thiệu luật chơi + Hình thức hóa bài toán | — |
| **TV2** | **Uninformed Search Lead** | Khung solver + đo đạc, BFS, DFS (IDS nếu dư thời gian), test luật nâng cao, chạy thực nghiệm, README | BFS, DFS/IDS + phân tích thực nghiệm | — |
| **TV3** | **Informed Search Lead** | Hàm chi phí, `problem.py`, heuristic, UCS, A*, chứng minh admissible/consistent | UCS (hàm chi phí) + A* (heuristic, chứng minh) | — |
| **TV4** | **GUI Lead** | GUI: vẽ, điều khiển, nút, chạy solver nền, phát lại lời giải | Cài đặt & GUI (ảnh chụp) | **Quay, dựng, upload video** |
| **TV5** | **Levels, Experiments & PM** | Định dạng/loader màn chơi, thiết kế bộ màn G1–G6, script thực nghiệm, biểu đồ, quản lý Notion, ráp report, đóng gói nộp bài | Experiments (bảng, biểu đồ) + Planning + Tham khảo + Phụ lục AI + ráp PDF | — |

> **Quy tắc chung:** mọi thành viên phải hiểu **toàn bộ** hệ thống (mô hình trạng thái, bảng chuyển, 4 thuật toán, heuristic, hàm chi phí), vì giảng viên có thể vấn đáp bất kỳ ai. Task `S5-13` dành riêng cho việc này.

---

## 3. Tải công việc dự kiến

| Thành viên | Tổng (h) | Trong đó P2 – mở rộng (h) | Task riêng | Task chung "Tất cả" |
|---|---:|---:|---:|---:|
| TV1 – Engine | 55 | 0 | 43 | 12 |
| TV2 – Uninformed | 55 | 6 | 43 | 12 |
| TV3 – Informed | 56 | 6 | 44 | 12 |
| TV4 – GUI | 59 | 0 | 47 | 12 |
| TV5 – Levels/Exp/PM | 58 | 0 | 46 | 12 |
| **Tổng** | **283** | **12** | 223 | 60 |

Task chung "Tất cả" (12h/người): đọc đề & chơi thử (2h), 4 buổi sprint review (4 × 1h), bug bash (2h), đọc chéo report (1h), họp chốt % đóng góp (1h), luyện vấn đáp (2h).

---

## 4. Bảng tổng hợp task (copy vào Notion)

| ID | Task | Sprint | Người phụ trách | Ưu tiên | Ước lượng (h) | Hạn | Phụ thuộc | Hạng mục điểm |
|---|---|---|---|---|---:|---|---|---|
| S0-01 | Đọc kỹ đề + chơi thử game gốc trên Coolmath | Sprint 0 | Tất cả | P0 | 2 | Ngày 1 | — | Quản lý |
| S0-02 | Tạo repo Git, skeleton thư mục, `.gitignore`, `requirements.txt`, quy ước branch/commit | Sprint 0 | TV5 | P0 | 2 | Ngày 1 | — | Quản lý |
| S0-03 | Viết `DESIGN.md`: chốt 12 quyết định thiết kế | Sprint 0 | TV1 | P0 | 3 | Ngày 2 | S0-01 | Game-Adv, Report-Algo |
| S0-04 | Chốt interface `State`, `BloxorzProblem`, `SearchResult` | Sprint 0 | TV2 | P0 | 2 | Ngày 2 | S0-03 | Quản lý |
| S0-05 | Chốt định dạng JSON màn chơi + bảng ký hiệu | Sprint 0 | TV5 | P0 | 2 | Ngày 2 | S0-03 | Game-Core |
| S0-06 | Chọn thư viện GUI + prototype cửa sổ vẽ lưới tĩnh | Sprint 0 | TV4 | P0 | 3 | Ngày 2 | — | Game-Core |
| S0-07 | Dựng Notion board, lịch họp, kênh liên lạc | Sprint 0 | TV5 | P1 | 1 | Ngày 1 | — | Quản lý |
| S0-08 | Ôn lý thuyết UCS/A*/admissible/consistent, phác thảo hàm chi phí | Sprint 0 | TV3 | P0 | 3 | Ngày 2 | — | UCS, A* |
| S1-01 | Cài `State`/`BlockConfig` bất biến, hashable, chuẩn hóa ô neo | Sprint 1 | TV1 | P0 | 3 | Ngày 3 | S0-04 | Game-Core |
| S1-02 | Cài bảng chuyển trạng thái 12 ô (`roll`) | Sprint 1 | TV1 | P0 | 3 | Ngày 3 | S1-01 | Game-Core |
| S1-03 | Kiểm tra hợp lệ (ngoài bàn/void), goal test, hàm `step()` duy nhất | Sprint 1 | TV1 | P0 | 3 | Ngày 4 | S1-02 | Game-Core |
| S1-04 | Unit test bảng chuyển, rơi khỏi bàn, thắng/không thắng | Sprint 1 | TV1 | P0 | 3 | Ngày 4 | S1-02, S1-03 | Game-Core |
| S1-05 | Level loader JSON + validator | Sprint 1 | TV5 | P0 | 4 | Ngày 4 | S0-05 | Game-Core |
| S1-06 | Thiết kế bộ màn G1 – Basic (3–4 màn, có Level 1) | Sprint 1 | TV5 | P0 | 3 | Ngày 5 | S0-05 | Report-Exp |
| S1-07 | `solvers/base.py`: đo thời gian, bộ nhớ, đếm node, timeout, truy vết | Sprint 1 | TV2 | P0 | 4 | Ngày 4 | S0-04 | BFS |
| S1-08 | Cài BFS | Sprint 1 | TV2 | P0 | 3 | Ngày 5 | S1-03, S1-07 | BFS |
| S1-09 | Test solver: Level 1 = 7 nước, màn vô nghiệm dừng đúng | Sprint 1 | TV2 | P0 | 2 | Ngày 6 | S1-06, S1-08 | BFS |
| S1-10 | GUI: vẽ lưới + khối ở 3 hướng | Sprint 1 | TV4 | P0 | 4 | Ngày 4 | S0-06, S1-01 | Game-Core, Video |
| S1-11 | GUI: phím mũi tên, Restart, thông báo thắng/thua, đếm nước | Sprint 1 | TV4 | P0 | 4 | Ngày 6 | S1-03, S1-05, S1-10 | Game-Core |
| S1-12 | `problem.py`: `successors()`, hàm chi phí thay được (unit/energy) | Sprint 1 | TV3 | P0 | 3 | Ngày 5 | S1-03 | UCS |
| S1-13 | Bản nháp hàm chi phí năng lượng (tính Δh, bảng, lý giải) | Sprint 1 | TV3 | P0 | 3 | Ngày 6 | S0-08 | UCS, Report-Algo |
| S1-14 | Sprint 1 review: demo nội bộ, merge, cập nhật Notion | Sprint 1 | Tất cả | P0 | 1 | Ngày 6 | S1-01…S1-13 | Quản lý |
| S2-01 | Engine: ô fragile | Sprint 2 | TV1 | P0 | 2 | Ngày 7 | S1-03 | Game-Adv |
| S2-02 | Engine: `BridgeConfig` bitmask + cầu mở/đóng | Sprint 2 | TV1 | P0 | 3 | Ngày 7 | S1-03 | Game-Adv |
| S2-03 | Engine: soft/heavy switch, toggle/permanent, kiểm tra lại sau khi đổi cầu | Sprint 2 | TV1 | P0 | 4 | Ngày 8 | S2-02 | Game-Adv |
| S2-04 | Engine: split switch, 2 cube, 8 hành động, tự gộp, luật cho cube | Sprint 2 | TV1 | P0 | 6 | Ngày 10 | S2-03 | Game-Adv |
| S2-05 | Unit test các ô nâng cao (viết từ `DESIGN.md`) | Sprint 2 | TV2 | P0 | 4 | Ngày 10 | S0-03, S2-04 | Game-Adv |
| S2-06 | Cài DFS (stack tường minh, visited, thứ tự hành động cố định) | Sprint 2 | TV2 | P0 | 3 | Ngày 7 | S1-07 | DFS |
| S2-07 | Kiểm chứng BFS/DFS trên màn có cầu (màn phải quay lại chỗ cũ) | Sprint 2 | TV2 | P0 | 2 | Ngày 9 | S2-03, S2-06 | BFS, DFS |
| S2-08 | (Mở rộng) Cài IDS để so sánh với DFS | Sprint 2 | TV2 | P2 | 3 | Ngày 10 | S2-06 | DFS |
| S2-09 | GUI: vẽ fragile, 3 loại switch, cầu mở/đóng, cube, tô sáng cube đang điều khiển | Sprint 2 | TV4 | P0 | 5 | Ngày 9 | S1-10, S2-03 | Game-Adv, Video |
| S2-10 | GUI: phím Space đổi cube + màn hình chọn màn (New Game) | Sprint 2 | TV4 | P0 | 4 | Ngày 10 | S2-04, S2-09 | Game-Adv |
| S2-11 | Thiết kế bộ màn G2 (Fragile), G3 (Switch & Bridge), G4 (Split) | Sprint 2 | TV5 | P0 | 6 | Ngày 10 | S1-05 | Report-Exp |
| S2-12 | Heuristic `h₁` (Manhattan) và `h₂` (khoảng cách trên đồ thị ô + cổng split) | Sprint 2 | TV3 | P0 | 5 | Ngày 10 | S1-05 | A* |
| S2-13 | Dựng khung report (Overleaf/Word) theo dàn ý | Sprint 2 | TV5 | P1 | 2 | Ngày 9 | — | Report-Plan |
| S2-14 | Sprint 2 review: demo nội bộ, merge, cập nhật Notion | Sprint 2 | Tất cả | P0 | 1 | Ngày 10 | S2-01…S2-13 | Quản lý |
| S3-01 | Chốt hàm chi phí cuối cùng (năng lượng ± phạt fragile), quy về số nguyên | Sprint 3 | TV3 | P0 | 2 | Ngày 11 | S1-13 | UCS |
| S3-02 | Cài UCS (heap + counter, lazy deletion, goal test khi pop) | Sprint 3 | TV3 | P0 | 4 | Ngày 12 | S1-12, S3-01 | UCS |
| S3-03 | Cài A* (f = g + h, phá hòa, `best_g`) | Sprint 3 | TV3 | P0 | 4 | Ngày 13 | S2-12, S3-02 | A* |
| S3-04 | Heuristic cho chi phí không đồng nhất: `c_up + c_min·(n−1)` | Sprint 3 | TV3 | P0 | 2 | Ngày 13 | S2-12, S3-01 | A* |
| S3-05 | Test chéo: chi phí A* = UCS; độ dài A*(unit) = BFS trên mọi màn | Sprint 3 | TV3 | P0 | 3 | Ngày 14 | S3-03, S3-04 | A*, UCS |
| S3-06 | Bản nháp chứng minh admissible & consistent | Sprint 3 | TV3 | P0 | 4 | Ngày 14 | S3-04 | A*, Report-Algo |
| S3-07 | Solver registry + hàm gọi chung `run_solver(name, level)` | Sprint 3 | TV2 | P0 | 2 | Ngày 11 | S2-06, S3-02 | BFS, DFS, UCS, A* |
| S3-08 | GUI: nút Solve BFS/DFS/UCS/A*, chạy luồng nền, báo "Đang giải…" | Sprint 3 | TV4 | P0 | 5 | Ngày 12 | S3-07 | Game-Core, Video |
| S3-09 | GUI: phát lại lời giải (Play/Pause/Step, tốc độ), tự chèn đổi cube | Sprint 3 | TV4 | P0 | 5 | Ngày 14 | S3-08 | Video |
| S3-10 | GUI: panel số liệu (time, memory, expanded, length, cost) | Sprint 3 | TV4 | P1 | 2 | Ngày 13 | S3-08 | Video |
| S3-11 | Thiết kế bộ màn G5 (Combined/Hard) và G6 (Vô nghiệm, Open field) | Sprint 3 | TV5 | P0 | 5 | Ngày 13 | S2-11 | Report-Exp |
| S3-12 | `experiments/run_all.py`: chạy lặp, median, đo time/memory tách lượt, xuất CSV | Sprint 3 | TV5 | P0 | 5 | Ngày 14 | S3-07 | Report-Exp |
| S3-13 | Hỗ trợ tích hợp, sửa bug engine phát sinh, bổ sung docstring | Sprint 3 | TV1 | P1 | 4 | Ngày 14 | — | Game-Core, Game-Adv |
| S3-14 | (Mở rộng) Profiling & tối ưu tốc độ solver | Sprint 3 | TV2 | P2 | 3 | Ngày 14 | S3-05 | BFS, DFS |
| S3-15 | Sprint 3 review: demo cả 4 solver trên GUI | Sprint 3 | Tất cả | P0 | 1 | Ngày 14 | S3-01…S3-14 | Quản lý |
| S4-06 | Bug bash: chơi chéo màn của nhau, ghi bug lên Notion | Sprint 4 | Tất cả | P0 | 2 | Ngày 15 | S3-15 | Game-Core, Game-Adv |
| S4-08 | Sửa các bug engine từ bug bash | Sprint 4 | TV1 | P0 | 4 | Ngày 16 | S4-06 | Game-Core, Game-Adv |
| S4-01 | Chạy thực nghiệm chính thức trên toàn bộ màn, ghi cấu hình máy | Sprint 4 | TV2 | P0 | 3 | Ngày 17 | S3-11, S3-12, S4-08 | Report-Exp |
| S4-02 | `plot.py`: vẽ toàn bộ biểu đồ (trục log) + bảng tổng hợp | Sprint 4 | TV5 | P0 | 4 | Ngày 18 | S4-01 | Report-Exp |
| S4-03 | (Mở rộng) Heuristic pattern database + so sánh h=0 / h₁ / h₂ / PDB | Sprint 4 | TV3 | P2 | 6 | Ngày 17 | S3-03 | A*, Report-Exp |
| S4-04 | Phân tích số liệu: điểm mạnh/yếu từng thuật toán (bản nháp) | Sprint 4 | TV2 | P0 | 5 | Ngày 18 | S4-01 | Report-Exp |
| S4-05 | Đánh bóng GUI: chú thích loại ô, màu sắc, bố cục | Sprint 4 | TV4 | P1 | 4 | Ngày 17 | S3-09 | Video |
| S4-07 | Viết `README.md` (cài đặt, chạy GUI, chạy thực nghiệm, định dạng màn) | Sprint 4 | TV2 | P0 | 2 | Ngày 17 | S3-12 | Nộp bài |
| S4-09 | Viết kịch bản video + chuẩn bị màn demo | Sprint 4 | TV4 | P0 | 2 | Ngày 18 | S4-05 | Video |
| S4-10 | Sprint 4 review: chốt số liệu & bản build cuối | Sprint 4 | Tất cả | P0 | 1 | Ngày 18 | S4-01…S4-09 | Quản lý |
| S5-12 | Họp chốt % đóng góp của từng thành viên | Sprint 5 | Tất cả | P0 | 1 | Ngày 19 | S4-10 | Report-Plan |
| S5-01 | Report: Giới thiệu luật chơi + Hình thức hóa bài toán + quyết định thiết kế | Sprint 5 | TV1 | P0 | 5 | Ngày 19 | S0-03 | Report-Algo |
| S5-02 | Report: Mô tả BFS, DFS (/IDS) + pseudocode | Sprint 5 | TV2 | P0 | 4 | Ngày 19 | S2-06 | Report-Algo |
| S5-03 | Report: UCS (hàm chi phí) + A* (heuristic + chứng minh) | Sprint 5 | TV3 | P0 | 5 | Ngày 19 | S3-06 | Report-Algo |
| S5-04 | Report: chương Experiments (nhóm test, phương pháp, bảng, biểu đồ, thảo luận) | Sprint 5 | TV5 | P0 | 4 | Ngày 19 | S4-02, S4-04 | Report-Exp |
| S5-05 | Report: Cài đặt & GUI (kiến trúc, định dạng màn, ảnh chụp) | Sprint 5 | TV4 | P0 | 3 | Ngày 19 | S4-05 | Report-Algo |
| S5-07 | Report: Planning & Task Distribution, Tham khảo, Phụ lục dùng AI | Sprint 5 | TV5 | P0 | 3 | Ngày 19 | S5-12 | Report-Plan |
| S5-06 | Quay + dựng video, phụ đề/thuyết minh, upload YouTube (Public) | Sprint 5 | TV4 | P0 | 6 | Ngày 20 | S4-09 | Video |
| S5-08 | Ráp report, định dạng, xuất `Report.pdf` (chèn link video) | Sprint 5 | TV5 | P0 | 4 | Ngày 20 | S5-01…S5-07 | Report-Plan |
| S5-09 | Đọc chéo report, sửa lỗi nội dung & trình bày | Sprint 5 | Tất cả | P0 | 1 | Ngày 20 | S5-08 | Report-Plan |
| S5-10 | Kiểm tra cài đặt sạch trên máy khác (venv mới) | Sprint 5 | TV2 | P0 | 1 | Ngày 20 | S4-07 | Nộp bài |
| S5-13 | Luyện vấn đáp: mỗi người trình bày 1 phần, hỏi chéo | Sprint 5 | Tất cả | P0 | 2 | Ngày 21 | S5-08 | Quản lý |
| S5-11 | Đóng gói zip đúng tên, kiểm tra dung lượng & link video ẩn danh, nộp bài | Sprint 5 | TV5 | P0 | 1 | Ngày 21 | S5-06, S5-09, S5-10 | Nộp bài |

---

## 5. Chi tiết từng task

> Mỗi task có **Mô tả** (làm gì) và **Tiêu chí hoàn thành** (khi nào được chuyển sang Done). Một task code chỉ được tính Done khi: đã merge qua Pull Request, **có ít nhất 1 người khác review**, toàn bộ test vẫn pass.

### Sprint 0 — Khởi động & thiết kế (Ngày 1–2)

#### S0-01 · Đọc kỹ đề + chơi thử game gốc — *Tất cả · P0 · 2h · Ngày 1*
**Mô tả:** Mỗi người đọc `AI - Project 1.pdf` và `report0.md`, chơi thử Bloxorz trên Coolmath, cố gắng gặp đủ fragile, soft/heavy switch, split.
- [ ] Đã đọc đề và report0
- [ ] Đã chơi qua ít nhất 1 màn có fragile, 1 màn có switch, 1 màn có split
- [ ] Ghi các câu hỏi/chỗ chưa rõ vào trang "Câu hỏi" trên Notion

#### S0-02 · Tạo repo Git và skeleton — *TV5 · P0 · 2h · Ngày 1*
**Mô tả:** Tạo repo private, thêm cả nhóm; dựng cấu trúc thư mục theo mục 6.1 report0 (`bloxorz/core`, `solvers`, `gui`, `levels`, `experiments`, `tests`).
- [ ] Repo private, cả 5 người có quyền push
- [ ] `.gitignore` loại `venv/`, `__pycache__/`, `*.pyc`, `results/`, file video
- [ ] `requirements.txt` ban đầu, `main.py` chạy được (in "Hello")
- [ ] Quy ước: branch `feature/<id-task>-<tên>`, commit message có ID task, PR cần 1 reviewer

#### S0-03 · Viết `DESIGN.md` — *TV1 · P0 · 3h · Ngày 2*
**Mô tả:** Chốt 12 quyết định thiết kế ở mục 9.1 report0 (thời điểm kích hoạt switch, cầu đóng dưới khối, permanent, cube trên fragile/goal/heavy, gộp cube, đổi cube có tính nước không, …) và mô hình hóa khối tách (phương án A, 8 hành động).
- [ ] Đủ 12 quyết định, mỗi cái có lý do 1–2 câu
- [ ] Có thứ tự 7 bước của hàm chuyển trạng thái
- [ ] Cả 4 thành viên còn lại đã đọc và comment đồng ý trên Notion

#### S0-04 · Chốt interface `State`, `BloxorzProblem`, `SearchResult` — *TV2 · P0 · 2h · Ngày 2*
**Mô tả:** Viết file stub (chỉ chữ ký hàm + docstring) để mọi người code song song.
- [ ] Stub `state.py`, `problem.py`, `solvers/base.py` đã merge
- [ ] `SearchResult` có đủ: `found, path, cost, expanded, generated, time_s, peak_mem_bytes, status`
- [ ] Định nghĩa **"expanded node"** ghi rõ trong docstring (node được pop ra và sinh con; không tính node stale)

#### S0-05 · Chốt định dạng JSON màn chơi — *TV5 · P0 · 2h · Ngày 2*
**Mô tả:** Theo mẫu ở mục 6.2 report0: lưới ký tự + `bridges` + `switches` (type, mode, bridges) + `splits` (pos, targets).
- [ ] File `levels/FORMAT.md` mô tả mọi trường và ký hiệu
- [ ] 1 màn ví dụ hợp lệ có đủ mọi loại ô

#### S0-06 · Chọn thư viện GUI + prototype — *TV4 · P0 · 3h · Ngày 2*
**Mô tả:** So sánh nhanh Pygame / Tkinter / Web, chọn một (khuyến nghị Pygame); prototype mở cửa sổ và vẽ một lưới tĩnh từ mảng ký tự.
- [ ] Ghi lựa chọn + lý do vào `DESIGN.md`
- [ ] Prototype chạy được trên máy của cả 5 người

#### S0-07 · Dựng Notion board — *TV5 · P1 · 1h · Ngày 1*
- [ ] Database task theo mục 1, có đủ property và 4 view
- [ ] Lịch họp: sprint review cuối mỗi sprint + 2 buổi check-in 15 phút/tuần
- [ ] Kênh liên lạc chung (Messenger/Discord/Zalo)

#### S0-08 · Ôn lý thuyết + phác thảo hàm chi phí — *TV3 · P0 · 3h · Ngày 2*
**Mô tả:** Ôn UCS, A*, admissible, consistent (AIMA chương 3); đọc mục 5.3–5.4 report0; phác thảo hàm chi phí.
- [ ] Ghi chú ngắn (1 trang) trên Notion về điều kiện tối ưu của UCS/A*
- [ ] Đề xuất sơ bộ hàm chi phí để nhóm góp ý

### Sprint 1 — Core mechanics + GUI tối thiểu + BFS (Ngày 2–6)

#### S1-01 · `State`/`BlockConfig` — *TV1 · P0 · 3h · Ngày 3*
**Mô tả:** Khối nguyên `("S"|"H"|"V", r, c)` với ô neo trên cùng/trái nhất; khối tách `("SPLIT", cube_a, cube_b)` với cặp cube đã **sắp xếp** (chuẩn hóa); `BridgeConfig` là `int` bitmask.
- [ ] Dùng `tuple` hoặc `@dataclass(frozen=True)` — dùng làm khóa `set`/`dict` được
- [ ] Hàm `cells(block)` trả về các ô bị chiếm

#### S1-02 · Bảng chuyển trạng thái 12 ô — *TV1 · P0 · 3h · Ngày 3*
**Mô tả:** Cài `roll(block, direction)` đúng bảng ở mục 2.3 report0 (chưa xét hợp lệ).
- [ ] Đủ 3 hướng khối × 4 hướng lăn
- [ ] Không chứa logic GUI

#### S1-03 · Kiểm tra hợp lệ, goal test, hàm `step()` — *TV1 · P0 · 3h · Ngày 4*
**Mô tả:** `step(level, state, action) -> State | None` (None = nước đi không hợp lệ/thua) là **hàm duy nhất** mà cả GUI và solver gọi.
- [ ] Ngoài bàn / void → `None`
- [ ] `is_goal` chỉ đúng khi khối nguyên đứng trên goal
- [ ] Khối nằm vắt qua goal là hợp lệ nhưng không thắng

#### S1-04 · Unit test core — *TV1 · P0 · 3h · Ngày 4*
- [ ] 12 test cho 12 ô của bảng chuyển
- [ ] Test "tập ô sau khi lăn rời hoàn toàn tập ô cũ"
- [ ] Test rơi khỏi mép, rơi vào void (từng nửa khối)
- [ ] Test thắng / nằm trên goal không thắng
- [ ] `pytest` pass

#### S1-05 · Level loader + validator — *TV5 · P0 · 4h · Ngày 4*
**Mô tả:** `Level.from_json(path)` trả về đối tượng tĩnh, bất biến trong lúc tìm kiếm.
- [ ] Đọc được mọi trường của `FORMAT.md`
- [ ] Validator báo lỗi rõ ràng: thiếu/nhiều goal, thiếu start, tọa độ ngoài lưới, switch trỏ tới cầu không tồn tại, ô dịch chuyển của split không phải sàn, start trên fragile
- [ ] Test cho ít nhất 3 file JSON lỗi

#### S1-06 · Bộ màn G1 – Basic — *TV5 · P0 · 3h · Ngày 5*
- [ ] 3–4 màn, trong đó có 1 màn tái hiện Level 1 (lời giải tối ưu 7 nước)
- [ ] Ghi kích thước, số ô sàn, lời giải tối ưu (sau khi có BFS) vào `levels/README.md`

#### S1-07 · Khung solver `base.py` — *TV2 · P0 · 4h · Ngày 4*
**Mô tả:** Hàm bọc `measure(solver, problem, measure_memory: bool)`; timeout và giới hạn node; truy vết đường đi qua `parent` dict.
- [ ] Đo thời gian bằng `time.perf_counter()`, **chỉ** phần tìm kiếm
- [ ] Đo bộ nhớ đỉnh bằng `tracemalloc` ở **lượt chạy riêng**
- [ ] Trạng thái `ok / no_solution / timeout / node_limit`

#### S1-08 · BFS — *TV2 · P0 · 3h · Ngày 5*
- [ ] `deque`, đánh dấu visited khi push
- [ ] Trả về `SearchResult` đầy đủ số liệu
- [ ] Ghi rõ có/không dùng early goal test (thống nhất với cách đếm node)

#### S1-09 · Test solver — *TV2 · P0 · 2h · Ngày 6*
- [ ] BFS trên màn Level 1 cho đúng 7 nước
- [ ] Màn vô nghiệm → `no_solution`, không treo
- [ ] Mọi lời giải trả về đều được **phát lại qua `step()`** và kết thúc ở trạng thái đích (test này dùng lại cho mọi solver)

#### S1-10 · GUI: vẽ lưới + khối — *TV4 · P0 · 4h · Ngày 4*
- [ ] Vẽ sàn, void, goal phân biệt rõ
- [ ] Khối hiển thị đúng ở cả 3 hướng

#### S1-11 · GUI: điều khiển cơ bản — *TV4 · P0 · 4h · Ngày 6*
- [ ] Phím mũi tên gọi `step()`; phím R / nút Restart
- [ ] Thông báo thắng / thua (rơi), cho phép chơi lại
- [ ] Hiển thị số nước đã đi

#### S1-12 · `problem.py` — *TV3 · P0 · 3h · Ngày 5*
**Mô tả:** `BloxorzProblem(level, cost_model)` với `successors(s) -> [(action, s2, cost)]`; `cost_model` thay được (`UnitCost`, `EnergyCost`).
- [ ] BFS/DFS dùng được qua interface này
- [ ] Test: số successor đúng tại vài trạng thái tay tính

#### S1-13 · Bản nháp hàm chi phí năng lượng — *TV3 · P0 · 3h · Ngày 6*
- [ ] Bảng Δh cho 4 kiểu lăn (đổ xuống 0.118, lăn ngang 0.207, dựng đứng 0.618, cube 0.207) có hình vẽ minh họa
- [ ] Công thức `cost = 1 + λ·Δh`, lý do chọn λ
- [ ] Chứng minh mọi chi phí > 0

#### S1-14 · Sprint 1 review — *Tất cả · P0 · 1h · Ngày 6*
- [ ] Demo: chơi tay màn G1 trên GUI, BFS giải Level 1
- [ ] Cập nhật trạng thái task, chuyển task trễ sang sprint sau

### Sprint 2 — Advanced tiles + DFS (Ngày 6–10)

#### S2-01 · Fragile — *TV1 · P0 · 2h · Ngày 7*
- [ ] Khối **đứng** trên fragile → `None`; khối nằm (1 hoặc 2 nửa trên fragile) → hợp lệ
- [ ] Cube trên fragile xử lý theo `DESIGN.md`

#### S2-02 · Cầu và `BridgeConfig` — *TV1 · P0 · 3h · Ngày 7*
- [ ] Cầu đóng xử lý như void, cầu mở như sàn
- [ ] Trạng thái ban đầu của cầu đọc từ JSON vào bitmask

#### S2-03 · Soft/heavy switch, toggle/permanent — *TV1 · P0 · 4h · Ngày 8*
- [ ] Soft: bất kỳ ô nào của khối/cube đè lên; Heavy: chỉ khối nguyên đứng
- [ ] Toggle đảo bit; permanent `open`/`close` đặt cố định
- [ ] Đè 2 switch cùng lúc → kích hoạt cả hai theo thứ tự id
- [ ] Kiểm tra lại hợp lệ sau khi đổi cầu (cầu đóng dưới khối → rơi)

#### S2-04 · Split, cube, gộp — *TV1 · P0 · 6h · Ngày 10*
- [ ] Khối đứng trên split → 2 cube tại vị trí định sẵn
- [ ] 8 hành động `(cube_i, dir)` khi đang tách
- [ ] Cube kích hoạt soft, không kích hoạt heavy/split, không thắng trên goal
- [ ] Hai cube 4-kề → tự gộp thành khối H/V đúng ô neo

#### S2-05 · Unit test ô nâng cao — *TV2 · P0 · 4h · Ngày 10*
**Mô tả:** TV2 viết test **dựa trên `DESIGN.md`** (không đọc code của TV1 trước) → phát hiện chỗ hiểu lệch đặc tả.
- [ ] ≥ 2 test cho mỗi: fragile, cầu, soft, heavy, toggle, permanent, split, gộp
- [ ] Test "nằm trên heavy không kích hoạt", "cube trên heavy không kích hoạt"
- [ ] Mọi test pass sau khi TV1 sửa (nếu có)

#### S2-06 · DFS — *TV2 · P0 · 3h · Ngày 7*
- [ ] Stack tường minh (không đệ quy), visited, thứ tự hành động cố định U-D-L-R
- [ ] Chạy được trên màn open field 20×30 không lỗi `RecursionError`

#### S2-07 · Kiểm chứng BFS/DFS với trạng thái cầu — *TV2 · P0 · 2h · Ngày 9*
- [ ] Có 1 màn bắt buộc đi bấm switch rồi quay lại đúng vị trí cũ; BFS/DFS giải được
- [ ] Chứng minh bằng test: nếu bỏ `BridgeConfig` khỏi visited thì màn này không giải được

#### S2-08 · (Mở rộng) IDS — *TV2 · P2 · 3h · Ngày 10*
- [ ] Chống lặp bằng path-checking hoặc `dict[state] = độ sâu nhỏ nhất`
- [ ] Độ dài lời giải IDS = BFS trên mọi màn G1–G3

#### S2-09 · GUI: ô nâng cao — *TV4 · P0 · 5h · Ngày 9*
- [ ] Fragile màu cam; soft hình tròn; heavy hình chữ X; split hình "( )"
- [ ] Cầu mở/đóng nhìn khác nhau rõ ràng
- [ ] Vẽ 2 cube, tô sáng cube đang điều khiển

#### S2-10 · GUI: đổi cube + chọn màn — *TV4 · P0 · 4h · Ngày 10*
- [ ] Space đổi cube đang điều khiển
- [ ] New Game mở danh sách màn trong `levels/`, chọn để chơi

#### S2-11 · Bộ màn G2, G3, G4 — *TV5 · P0 · 6h · Ngày 10*
- [ ] G2: 2–3 màn fragile (có màn mà fragile làm lời giải dài hơn)
- [ ] G3: 3–4 màn có soft, heavy, toggle **và** permanent
- [ ] G4: 2–3 màn split (có màn cần cube bấm soft switch)
- [ ] Tất cả qua validator và **chơi tay thắng được**

#### S2-12 · Heuristic `h₁`, `h₂` — *TV3 · P0 · 5h · Ngày 10*
**Mô tả:** Module `heuristics.py`: tiền xử lý một lần cho mỗi màn (BFS ngược từ goal trên đồ thị ô có sàn, coi mọi cầu mở, thêm cạnh 0 từ ô split tới ô đích dịch chuyển).
- [ ] `h₁ = ⌈Manhattan_min / 2⌉`, `h₂ = ⌈dist_G_min / 2⌉`, dùng `max(·, 1)` cho trạng thái không phải đích
- [ ] Khối tách: lấy min trên 2 cube
- [ ] `dist_G = ∞` → trạng thái chết

#### S2-13 · Khung report — *TV5 · P1 · 2h · Ngày 9*
- [ ] Template (Overleaf/Word) với đủ mục theo dàn ý 8.3 report0, chia quyền cho cả nhóm

#### S2-14 · Sprint 2 review — *Tất cả · P0 · 1h · Ngày 10*
- [ ] Demo: chơi tay màn G2–G4; BFS/DFS giải được mọi màn này

### Sprint 3 — UCS, A*, tích hợp GUI (Ngày 10–14)

#### S3-01 · Chốt hàm chi phí — *TV3 · P0 · 2h · Ngày 11*
- [ ] Quyết định có cộng phạt fragile hay không; ghi `c_min`, `c_up`
- [ ] Chi phí quy về số nguyên (ví dụ ×1000) để tránh lỗi so sánh số thực

#### S3-02 · UCS — *TV3 · P0 · 4h · Ngày 12*
- [ ] `heapq` với `(g, counter, state)`; lazy deletion qua `best_g`
- [ ] Goal test khi **pop**
- [ ] Trả về cả `cost` và độ dài lời giải

#### S3-03 · A* — *TV3 · P0 · 4h · Ngày 13*
- [ ] `(f, h, counter, state)` — phá hòa theo `h` nhỏ
- [ ] Hỗ trợ chọn heuristic qua tham số (`h0`, `h1`, `h2`)

#### S3-04 · Heuristic cho chi phí không đồng nhất — *TV3 · P0 · 2h · Ngày 13*
- [ ] `h = c_up + c_min·(n − 1)`, `h(goal) = 0`, với `n = max(⌈dist_G/2⌉, 1)`

#### S3-05 · Test chéo — *TV3 · P0 · 3h · Ngày 14*
- [ ] Mọi màn: chi phí A* == chi phí UCS (chi phí năng lượng)
- [ ] Mọi màn: độ dài A* (unit) == độ dài BFS
- [ ] Nếu sai → coi là bug nghiêm trọng, báo ngay trong nhóm

#### S3-06 · Chứng minh admissible & consistent — *TV3 · P0 · 4h · Ngày 14*
- [ ] Mệnh đề 1: mỗi nước đi giảm `min dist_G` tối đa 2 (kể cả nước kích hoạt split)
- [ ] Mệnh đề 2: trạng thái đích chỉ đạt được bằng nước dựng đứng
- [ ] Chứng minh admissible và consistent cho cả chi phí unit và chi phí năng lượng
- [ ] Phản ví dụ cho công thức `⌈d/2⌉ + 1`

#### S3-07 · Solver registry — *TV2 · P0 · 2h · Ngày 11*
- [ ] `run_solver("bfs"|"dfs"|"ucs"|"astar", level, **options) -> SearchResult`
- [ ] GUI và `run_all.py` đều gọi qua hàm này

#### S3-08 · GUI: nút solver + luồng nền — *TV4 · P0 · 5h · Ngày 12*
- [ ] 4 nút Solve BFS / DFS / UCS / A*
- [ ] Solver chạy trong thread riêng, cửa sổ không bị "Not Responding"
- [ ] Báo "Đang giải…", báo timeout / vô nghiệm

#### S3-09 · GUI: phát lại lời giải — *TV4 · P0 · 5h · Ngày 14*
- [ ] Play / Pause / Step / chỉnh tốc độ
- [ ] Tự hiển thị thao tác đổi cube khi lời giải chuyển sang điều khiển cube khác
- [ ] Phát lại xong đúng trạng thái thắng

#### S3-10 · GUI: panel số liệu — *TV4 · P1 · 2h · Ngày 13*
- [ ] Hiện time, peak memory, expanded, length, cost của lần giải gần nhất

#### S3-11 · Bộ màn G5, G6 — *TV5 · P0 · 5h · Ngày 13*
- [ ] G5: 3–4 màn kết hợp mọi cơ chế, có màn lớn (≥ 15×20)
- [ ] G6: 1 màn vô nghiệm + 1 màn open field 20×30
- [ ] Mỗi màn có ảnh chụp để đưa vào report

#### S3-12 · Script thực nghiệm — *TV5 · P0 · 5h · Ngày 14*
- [ ] Chạy mọi (solver × màn), mỗi cặp 5 lần, lấy median thời gian
- [ ] Đo bộ nhớ ở lượt riêng; tắt GUI và log
- [ ] Xuất `results.csv`: `level, group, solver, heuristic, time_ms, peak_kb, expanded, generated, length, cost, status`

#### S3-13 · Hỗ trợ tích hợp — *TV1 · P1 · 4h · Ngày 14*
- [ ] Sửa bug engine phát sinh khi tích hợp solver/GUI
- [ ] Docstring cho toàn bộ `core/`

#### S3-14 · (Mở rộng) Tối ưu tốc độ — *TV2 · P2 · 3h · Ngày 14*
- [ ] Profile bằng `cProfile` trên màn lớn nhất; ghi lại 2–3 điểm nghẽn
- [ ] Tối ưu (tiền xử lý loại ô, tránh tạo object thừa) mà không đổi kết quả test

#### S3-15 · Sprint 3 review — *Tất cả · P0 · 1h · Ngày 14*
- [ ] Demo: bấm cả 4 solver trên GUI cho 1 màn G5 và xem phát lại lời giải

### Sprint 4 — Thực nghiệm, đánh bóng & bug bash (Ngày 14–18)

#### S4-06 · Bug bash — *Tất cả · P0 · 2h · Ngày 15*
- [ ] Mỗi người chơi tay ít nhất 4 màn **do người khác tạo**
- [ ] Mọi bug ghi lên Notion (bước tái hiện, kết quả mong đợi/thực tế)

#### S4-08 · Sửa bug engine — *TV1 · P0 · 4h · Ngày 16*
- [ ] Mọi bug engine mức nghiêm trọng đã sửa và có test hồi quy
- [ ] (Bug GUI do TV4 sửa trong S4-05)

#### S4-01 · Chạy thực nghiệm chính thức — *TV2 · P0 · 3h · Ngày 17*
- [ ] Chạy trên bản code **sau** bug bash
- [ ] Ghi CPU, RAM, hệ điều hành, phiên bản Python
- [ ] Lưu `results.csv` cuối cùng vào repo

#### S4-02 · Biểu đồ + bảng — *TV5 · P0 · 4h · Ngày 18*
- [ ] Grouped bar: expanded nodes (trục log), time, peak memory theo màn
- [ ] Biểu đồ độ dài lời giải (DFS vs tối ưu); BFS vs UCS (độ dài & chi phí)
- [ ] Bảng tổng hợp xuất sẵn dạng LaTeX/Markdown để dán vào report

#### S4-03 · (Mở rộng) Pattern database — *TV3 · P2 · 6h · Ngày 17*
- [ ] BFS/Dijkstra ngược trên đồ thị trạng thái khối nới lỏng (mọi cầu mở, fragile = sàn, có cạnh split)
- [ ] So sánh số node mở rộng của A* với h=0 / h₁ / h₂ / PDB
- [ ] Test chéo A* (PDB) == UCS vẫn đúng

#### S4-04 · Phân tích số liệu — *TV2 · P0 · 5h · Ngày 18*
- [ ] Nhận xét từng thuật toán: điểm mạnh/yếu **dựa trên số liệu**
- [ ] Giải thích các trường hợp bất thường (ví dụ DFS nhanh hơn BFS ở màn nào, vì sao)
- [ ] Phân tích ảnh hưởng của số cầu (`2^k`) và split tới không gian trạng thái

#### S4-05 · Đánh bóng GUI — *TV4 · P1 · 4h · Ngày 17*
- [ ] Bảng chú thích (legend) các loại ô ngay trên màn hình
- [ ] Sửa các bug GUI từ bug bash
- [ ] Bố cục gọn gàng, đẹp khi quay video

#### S4-07 · `README.md` — *TV2 · P0 · 2h · Ngày 17*
- [ ] Cài đặt (`pip install -r requirements.txt`), chạy GUI (`python main.py`)
- [ ] Chạy thực nghiệm và vẽ biểu đồ
- [ ] Phím tắt, định dạng màn, cách thêm màn mới

#### S4-09 · Kịch bản video — *TV4 · P0 · 2h · Ngày 18*
- [ ] Kịch bản theo mục 8.4 report0, phủ đủ tiêu chí 5đ GUI + 5đ Feature
- [ ] Chọn sẵn màn demo: 1 màn thua (fragile), 1 màn switch, 1 màn split, 1 màn chạy 4 solver

#### S4-10 · Sprint 4 review — *Tất cả · P0 · 1h · Ngày 18*
- [ ] Chốt `results.csv` và biểu đồ cuối cùng
- [ ] **Code freeze**: sau buổi này chỉ sửa bug, không thêm tính năng

### Sprint 5 — Report, video, đóng gói (Ngày 18–21)

#### S5-12 · Họp chốt % đóng góp — *Tất cả · P0 · 1h · Ngày 19*
- [ ] Dựa trên Notion (task Done, giờ thực tế) và lịch sử commit
- [ ] Cả 5 người đồng ý, ghi biên bản ngắn

#### S5-01 · Report: Luật chơi + Hình thức hóa — *TV1 · P0 · 5h · Ngày 19*
- [ ] Trạng thái (hướng + cầu + tách), hành động, transition (bảng 12 ô + 7 bước), goal test, chống lặp, ước lượng không gian trạng thái
- [ ] Bảng 12 quyết định thiết kế

#### S5-02 · Report: BFS, DFS — *TV2 · P0 · 4h · Ngày 19*
- [ ] Ý tưởng, pseudocode, tính chất (đầy đủ, tối ưu, độ phức tạp)
- [ ] Ghi rõ DFS hay IDS; nếu có IDS thì so sánh

#### S5-03 · Report: UCS, A* — *TV3 · P0 · 5h · Ngày 19*
- [ ] Hàm chi phí: công thức, bảng, hình minh họa, lý giải
- [ ] Heuristic: thiết kế, chứng minh admissible/consistent theo hàm chi phí đã chọn

#### S5-04 · Report: Experiments — *TV5 · P0 · 4h · Ngày 19*
- [ ] Mô tả 6 nhóm test, phương pháp đo, môi trường máy
- [ ] Bảng + biểu đồ + phần thảo luận (lấy từ S4-04)

#### S5-05 · Report: Cài đặt & GUI — *TV4 · P0 · 3h · Ngày 19*
- [ ] Sơ đồ kiến trúc thư mục, định dạng màn
- [ ] Ảnh chụp GUI: các loại ô, khối 3 hướng, cube, panel solver

#### S5-07 · Report: Planning, Tham khảo, Phụ lục AI — *TV5 · P0 · 3h · Ngày 19*
- [ ] Bảng phân công + % hoàn thành (từ S5-12) + timeline
- [ ] Tài liệu tham khảo đầy đủ (AIMA, slide môn, Coolmath, thư viện, repo đã tham khảo)
- [ ] Phụ lục: mỗi người ghi trung thực mình dùng AI vào việc gì, prompt tiêu biểu

#### S5-06 · Quay + dựng + upload video — *TV4 · P0 · 6h · Ngày 20*
- [ ] Phủ đủ kịch bản S4-09; có phụ đề hoặc thuyết minh
- [ ] Upload YouTube chế độ **Public**; gửi link cho TV5

#### S5-08 · Ráp report, xuất PDF — *TV5 · P0 · 4h · Ngày 20*
- [ ] Trang đầu có thông tin nhóm + **link video**
- [ ] Mục lục, đánh số hình/bảng, định dạng thống nhất
- [ ] Xuất `Report.pdf`

#### S5-09 · Đọc chéo report — *Tất cả · P0 · 1h · Ngày 20*
- [ ] Mỗi người đọc ít nhất 1 chương **không phải** của mình, sửa lỗi/góp ý trực tiếp

#### S5-10 · Kiểm tra cài đặt sạch — *TV2 · P0 · 1h · Ngày 20*
- [ ] Clone repo trên máy khác → venv mới → `pip install -r requirements.txt` → `python main.py` chạy được
- [ ] Làm theo README từ đầu tới cuối không vướng

#### S5-13 · Luyện vấn đáp — *Tất cả · P0 · 2h · Ngày 21*
- [ ] Mỗi người trình bày 5 phút một phần **không phải** của mình
- [ ] Bộ câu hỏi luyện: vì sao trạng thái phải có cầu? vì sao BFS tối ưu? vì sao DFS không tối ưu? vì sao UCS ≡ BFS khi chi phí 1? chứng minh heuristic admissible? split ảnh hưởng heuristic thế nào?

#### S5-11 · Đóng gói và nộp — *TV5 · P0 · 1h · Ngày 21*
- [ ] Cấu trúc `StudentID1_..._StudentID5/Source/...` + `Report.pdf`
- [ ] Không có `venv/`, `__pycache__/`, `.git/`, video trong file nén; dung lượng < 20MB (nếu không thì dùng link Drive)
- [ ] Mở link video bằng cửa sổ ẩn danh xem được
- [ ] Nộp **trước deadline ít nhất vài giờ**

---

## 6. Mốc quan trọng & đường găng

| Mốc | Ngày | Điều kiện đạt |
|---|---|---|
| **M1 – Thiết kế chốt** | Ngày 2 | `DESIGN.md`, interface, định dạng JSON đã merge |
| **M2 – Chơi được bản cơ bản** | Ngày 6 | GUI chơi tay màn G1, BFS đúng 7 nước ở Level 1 |
| **M3 – Đủ luật chơi** | Ngày 10 | Mọi ô nâng cao chạy trên GUI; BFS/DFS giải G1–G4 |
| **M4 – Đủ 4 solver trên GUI** | Ngày 14 | Test chéo A* = UCS pass; nút solver + phát lại lời giải |
| **M5 – Code freeze** | Ngày 18 | Số liệu và biểu đồ cuối cùng |
| **M6 – Nộp bài** | Ngày 21 | PDF + video + zip đã nộp |

**Đường găng (trễ một task trên chuỗi này sẽ làm trễ cả dự án):**

```
S0-03 → S0-04 → S1-01 → S1-02 → S1-03 → S2-02 → S2-03 → S2-04 → (S2-10, S2-05)
      → S3-07 → S3-08 → S3-09 → S4-05 → S4-09 → S5-06 → S5-11
```

Chuỗi này đi qua **TV1** (engine, Sprint 1–2) rồi **TV4** (GUI, Sprint 3–5). Nhóm cần theo dõi sát hai người này, sẵn sàng hỗ trợ ngay khi có dấu hiệu trễ (ví dụ TV2 nhận một phần S2-04 nếu tới Ngày 8 chưa xong S2-03).

---

## 7. Quy tắc làm việc nhóm

1. **Mỗi task một branch, một PR**, có ít nhất 1 reviewer khác người viết.
2. **Cập nhật trạng thái trên Notion** ngay khi bắt đầu / gửi review / hoàn thành.
3. Task bị kẹt quá **1 ngày** → báo ngay trong nhóm, không giữ im lặng.
4. Mọi thay đổi luật chơi phải cập nhật **`DESIGN.md` trước**, code sau.
5. Ghi lại **giờ làm thực tế** trên Notion để chốt % đóng góp công bằng ở S5-12.
6. Dùng AI thì ghi lại công cụ và prompt chính vào một trang Notion chung, để làm phụ lục report (S5-07).
