# Report 6: Rà soát 4 thuật toán tìm kiếm, sửa bộ đếm BFS và ba lưu ý khi so sánh số liệu (nhóm 9)

> **Ngày:** 30/10/2026 · **Người thực hiện:** Kiên (TV5)
> **Phạm vi:** sau khi merge DFS (Phát) và UCS (Hải Đăng) vào `main`, rà soát lại **toàn bộ** phần tìm kiếm: BFS (Đạt), DFS (Phát), UCS (Hải Đăng), A\* (Sang), heuristic Manhattan (TV5) và phần nối trong `main.py`. Sửa lỗi tìm thấy, thêm test cố định, và đo số liệu để giải thích **ba điểm dễ hiểu sai khi so sánh các thuật toán** trong `Report.pdf`.
> Không đổi luật chơi, không đổi lời giải: mọi solver vẫn cho đúng lời giải như trước.

---

## 1. Tóm tắt

| Việc | Kết quả |
|---|---|
| Rà soát 4 solver + heuristic | ✅ **Cả 4 đều đúng**: tối ưu đúng chỗ cần tối ưu, chống lặp đúng, không trạng thái nào bị mở rộng hai lần. Heuristic **admissible + consistent** trên **mọi trạng thái, mọi cạnh** của 16 màn |
| Lỗi tìm thấy | 1 lỗi nhỏ: BFS **đếm thiếu `nodes_generated`** 1–3 node ở 9/16 màn (mục 3). Không ảnh hưởng lời giải. **Đã sửa** |
| Test mới | `tests/test_search_audit.py` (136 test kiểm chứng chéo với đáp án tính độc lập) + 1 test BFS tái hiện đúng lỗi |
| Ba lưu ý khi so sánh số liệu | Đã đo và giải thích (mục 6): (1) BFS kiểm tra đích sớm nên **ít node hơn UCS**; (2) "frontier max" của **A\*** tính cả bản ghi cũ; (3) **A\* gần như không cắt tỉa được ở màn 13** vì đích gần như là trạng thái xa nhất của đồ thị |
| Kiểm chứng | pytest **357 passed, 1 skipped** (trước: 220 passed, 1 skipped) |

---

## 2. Cách rà soát

### 2.1. Đọc code

Đọc từng dòng `ai/solvers/{bfs,dfs,ucs,astar}_solver.py`, `ai/heuristics.py`, `ai/tracing.py` và `main.py`, đối chiếu với mã giả ở `Docs/system.md` mục 8 và quy ước bộ đếm trong `ai/solvers/base_solver.py`:

| Thuật toán | Cấu trúc frontier | Chống lặp | Kiểm tra đích | Nhận xét |
|---|---|---|---|---|
| BFS | `deque` (FIFO) | `parent` đánh dấu khi **sinh** | khi **sinh** (early goal test) | Đúng; tối ưu theo số bước vì chi phí mỗi bước bằng 1 |
| DFS | `list` (LIFO), không đệ quy | `parent` đánh dấu khi **đẩy** | khi **lấy ra** | Đúng; là DFS trên đồ thị theo kiểu AIMA (con đã có trong stack thì không đẩy lại). Đẩy ngược để lấy ra theo U, D, L, R |
| UCS | `heapq` `(g, counter, state)` | `best_g` + bỏ qua bản ghi cũ | khi **lấy ra** | Đúng; tối ưu theo tổng chi phí với mọi chi phí dương |
| A\* | `heapq` `(f, h, counter, g, state)` | `best_g` + bỏ qua bản ghi cũ, cho mở rộng lại | khi **lấy ra** | Đúng; tối ưu kể cả khi heuristic chỉ admissible (không consistent) |
| Manhattan | – | – | – | Chứng minh trong docstring chặt chẽ, kể cả trường hợp split (mục 2.2 kiểm lại bằng máy) |

`main.py` nối đúng: chỉ A\* nhận `--heuristic`, file vết dùng `relax=True` cho UCS/A\* (đổi cha khi tìm được g nhỏ hơn), bộ nhớ đo ở lượt chạy riêng.

### 2.2. Kiểm chứng độc lập

Đọc code thôi thì dễ bỏ sót, nên mình viết thêm một bộ kiểm chứng **không dùng solver nào của nhóm làm chuẩn**:

1. Liệt kê **toàn bộ đồ thị trạng thái** của từng màn bằng `successors()` (màn 13: 47.680 trạng thái, 173.056 cạnh).
2. Tính **khoảng cách thật tới đích** h\*(s) của mọi trạng thái bằng BFS ngược từ các trạng thái đích, và chi phí tối ưu với chi phí không đồng nhất bằng một bản Dijkstra viết riêng.
3. Bọc bài toán bằng một lớp đếm số lần gọi `successors()` (mỗi lần gọi là một lần mở rộng thật) để đối chiếu với bộ đếm mà solver báo cáo.

| Kiểm tra (cả 16 màn) | Kết quả |
|---|---|
| BFS, UCS, A\* (Manhattan), A\* (h = 0) cho đúng số bước tối ưu h\*(start) | ✅ |
| DFS cho lời giải hợp lệ, không ngắn hơn tối ưu; **thứ tự mở rộng trùng từng node** với một bản DFS đồ thị chuẩn viết riêng | ✅ |
| Màn 11 vô nghiệm: cả 4 báo `no_solution`; BFS/DFS/UCS duyệt đúng 113 trạng thái tới được | ✅ |
| Không trạng thái nào bị mở rộng hai lần (ở cả 5 cấu hình) | ✅ |
| `nodes_expanded` bằng số lần mở rộng thật | ✅ |
| `nodes_generated` bằng tổng số con thật | ❌ **BFS đếm thiếu** ở 9 màn → mục 3 |
| Heuristic: h(đích) = 0, **h(s) ≤ h\*(s)** với mọi s (admissible), **h(s) ≤ 1 + h(s')** với mọi cạnh (consistent) | ✅ |
| 3 bộ chi phí ngẫu nhiên 1..7 mỗi nước, 15 màn có lời giải: chi phí của UCS = A\* (h = 0) = A\* (Manhattan) = Dijkstra riêng | ✅ |
| UCS và A\* (h = 0) cho **cùng số node, cùng lời giải** (đúng lý thuyết: A\* với h = 0 chính là UCS) | ✅ |
| Chạy lại cho cùng kết quả (tất định) | ✅ |

---

## 3. Lỗi tìm thấy: BFS đếm thiếu `nodes_generated`

**Nguyên nhân.** BFS kiểm tra đích **khi sinh con** và `return` ngay giữa vòng lặp. Các con đứng **sau** đích trong danh sách `successors()` đã được sinh ra (tốn thời gian và bộ nhớ thật) nhưng chưa được cộng vào `nodes_generated`. DFS, UCS và A\* duyệt hết danh sách con trước khi kiểm tra đích nên không bị.

**Ảnh hưởng.** Chỉ con số "node sinh ra" của BFS lệch quy ước so với ba thuật toán kia. Lời giải, độ dài, số node mở rộng, frontier và thời gian vẫn đúng.

| Màn | 1 | 3 | 4 | 5 | 7 | 12 | 13 | 15 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Trước khi sửa | 343 | 6 | 275 | 100 | 633 | 23.276 | 173.044 | 752 |
| Sau khi sửa (= số con thật) | 345 | 7 | 276 | 102 | 636 | 23.277 | 173.045 | 753 |

Các màn còn lại không lệch, vì ở đó đích tình cờ là con cuối cùng trong danh sách.

**Cách sửa** (`ai/solvers/bfs_solver.py`): cộng đủ số con ngay sau khi gọi `successors()`, trước khi kiểm tra đích:

```python
# Đếm đủ mọi con đã sinh, kể cả các con đứng sau đích khi
# Early Goal Test trả về giữa vòng lặp (cùng quy ước với DFS/UCS/A*).
children = self.problem.successors(state)
self.nodes_generated += len(children)

for action, nxt, _cost in children:
    ...
```

**Test.** `test_bfs_counts_siblings_generated_after_early_goal` (`tests/test_bfs_solver.py`) dựng một đồ thị có đích là **con đầu tiên** trong 3 con: test **fail** trên code cũ (đếm 1) và **pass** sau khi sửa (đếm 3).

---

## 4. Test mới

| File | Nội dung |
|---|---|
| `tests/test_bfs_solver.py` (+1 test) | Tái hiện lỗi ở mục 3 |
| `tests/test_search_audit.py` (mới, 136 test) | Đưa bộ kiểm chứng ở mục 2.2 vào repo: bộ đếm khớp công việc thật và không mở rộng hai lần (5 cấu hình solver × 15 màn); độ dài tối ưu so với h\* tính bằng BFS ngược; UCS ≡ A\* (h = 0); thứ tự mở rộng của DFS khớp bản chuẩn; heuristic admissible + consistent trên toàn bộ đồ thị; UCS/A\* khớp Dijkstra với chi phí ngẫu nhiên |

Để bộ test chạy nhanh (thêm khoảng 17 giây), `test_search_audit.py` bỏ qua màn 13 và chỉ thử chi phí ngẫu nhiên trên 8 màn đại diện. Script kiểm chứng đầy đủ đã chạy trên cả 16 màn (mục 2.2).

```bash
python -m pytest -q                              # 357 passed, 1 skipped
python -m pytest -q tests/test_search_audit.py   # riêng phần kiểm chứng chéo: 136 passed
```

Test bị skip là test dành cho khung ban đầu khi BFS chưa cài (`tests/test_solvers.py`), skip như vậy là đúng.

---

## 5. Bảng số liệu đầy đủ (sau khi sửa)

**Cách đo:** Python 3.12.6, Intel Core i5-12450HX, Windows 11; chi phí mỗi bước bằng 1 (đúng `main.py`). Thời gian là **trung vị của 5 lần chạy**, mỗi lần xóa cache của heuristic trước khi chạy. Bộ nhớ đỉnh đo bằng `tracemalloc` ở lượt chạy riêng. Thời gian dưới khoảng 5 ms dao động mạnh giữa các lần đo, chỉ nên xem là cùng bậc.

**Số node** (frontier = `max_frontier`):

| Màn | Tối ưu | BFS: mở rộng / sinh / frontier | DFS: số bước / mở rộng / frontier | UCS: mở rộng / frontier | A\*: mở rộng / frontier |
|---:|---:|---|---|---|---|
| 1 | 10 | 125 / 345 / 36 | 41 / 43 / 34 | 160 / 47 | 81 / 60 |
| 2 | 68 | 371 / 749 / 14 | 76 / 148 / 20 | 377 / 14 | 357 / 14 |
| 3 | 4 | 4 / 7 / 1 | 4 / 4 / 1 | 4 / 1 | 4 / 1 |
| 4 | 16 | 126 / 276 / 23 | 42 / 66 / 18 | 127 / 23 | 71 / 24 |
| 5 | 6 | 34 / 102 / 19 | 49 / 57 / 36 | 52 / 32 | 14 / 15 |
| 6 | 20 | 214 / 598 / 45 | 78 / 121 / 56 | 215 / 45 | 214 / 37 |
| 7 | 12 | 209 / 636 / 41 | 50 / 143 / 82 | 238 / 41 | 118 / 97 |
| 8 | 15 | 206 / 621 / 31 | 56 / 191 / 80 | 232 / 31 | 143 / 57 |
| 9 | 36 | 1.319 / 4.467 / 83 | 347 / 1.303 / 426 | 1.393 / 83 | 925 / 143 |
| 10 | 27 | 1.190 / 4.449 / 87 | 377 / 582 / 513 | 1.201 / 87 | 1.126 / 480 |
| 11 | – | 113 / 352 / 39 | – / 113 / 43 | 113 / 39 | 113 / 40 |
| 12 | 44 | 6.661 / 23.277 / 478 | 1.293 / 5.019 / 1.858 | 6.663 / 478 | 6.342 / 651 |
| 13 | 66 | 47.673 / 173.045 / 3.252 | 4.295 / 41.456 / 16.531 | 47.675 / 3.252 | 47.657 / 3.744 |
| 14 | 36 | 679 / 1.600 / 52 | 89 / 607 / 117 | 687 / 52 | 555 / 64 |
| 15 | 37 | 330 / 753 / 22 | 81 / 254 / 61 | 351 / 27 | 309 / 27 |
| 16 | 49 | 4.503 / 22.199 / 200 | 655 / 1.660 / 1.194 | 4.600 / 200 | 4.557 / 292 |

**Thời gian và bộ nhớ đỉnh:**

| Màn | BFS ms | DFS ms | UCS ms | A\* ms | BFS KB | DFS KB | UCS KB | A\* KB |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2,2 | 1,3 | 3,5 | 2,0 | 18,3 | 8,9 | 36,9 | 24,1 |
| 2 | 6,4 | 3,7 | 9,5 | 6,0 | 52,6 | 18,1 | 75,3 | 75,8 |
| 3 | 0,1 | 0,1 | 0,1 | 0,1 | 2,0 | 1,1 | 1,4 | 1,4 |
| 4 | 2,8 | 1,9 | 3,2 | 1,2 | 15,6 | 9,2 | 19,6 | 18,5 |
| 5 | 0,9 | 2,1 | 1,5 | 0,4 | 7,6 | 13,2 | 11,1 | 5,0 |
| 6 | 5,6 | 2,0 | 6,1 | 3,6 | 27,1 | 26,2 | 37,0 | 38,9 |
| 7 | 4,2 | 2,3 | 6,1 | 2,4 | 28,1 | 26,5 | 40,8 | 42,1 |
| 8 | 5,9 | 4,2 | 4,8 | 3,1 | 27,5 | 28,3 | 40,6 | 39,9 |
| 9 | 23,9 | 19,9 | 21,6 | 24,0 | 206,2 | 206,7 | 308,7 | 183,9 |
| 10 | 41,1 | 22,8 | 45,7 | 64,4 | 123,4 | 122,3 | 184,1 | 237,9 |
| 11 | 4,6 | 5,4 | 3,5 | 5,4 | 14,4 | 13,3 | 18,2 | 20,8 |
| 12 | 246,9 | 194,0 | 276,9 | 367,2 | 1017,9 | 1031,8 | 1455,4 | 1509,7 |
| 13 | 1765,8 | 1581,8 | 2138,5 | 2808,7 | 9213,4 | 9325,9 | 12905,1 | 13553,7 |
| 14 | 21,4 | 18,7 | 22,4 | 24,6 | 103,5 | 66,5 | 152,4 | 95,8 |
| 15 | 9,3 | 8,9 | 12,8 | 16,0 | 52,7 | 32,0 | 76,0 | 51,0 |
| 16 | 334,9 | 123,3 | 401,0 | 435,2 | 1076,7 | 740,5 | 1357,4 | 1363,8 |

Các nhận xét "đúng sách" rút ra được ngay từ bảng: DFS luôn mở rộng ít nhất và nhanh nhất nhưng lời giải dài gấp nhiều lần (màn 13: 4.295 bước so với 66); BFS, UCS, A\* luôn cho cùng độ dài tối ưu; A\* mở rộng ít nhất trong ba thuật toán tối ưu. Ba điểm **dễ hiểu sai** được giải thích ở mục 6.

---

## 6. Ba lưu ý khi so sánh số liệu

### 6.1. BFS mở rộng ít node hơn UCS chỉ vì **thời điểm kiểm tra đích**, không phải vì "BFS tốt hơn"

**Hiện tượng.** Chi phí bằng 1 thì BFS và UCS cho cùng lời giải tối ưu, nhưng UCS luôn mở rộng nhiều node hơn (màn 1: 160 so với 125; màn 16: 4.600 so với 4.503). Đọc bảng mà không biết lý do thì dễ kết luận sai rằng UCS kém hơn.

**Nguyên nhân.** BFS kiểm tra đích **khi sinh con** (early goal test): vừa sinh ra đích là dừng, không cần lấy hết tầng cuối. UCS (và A\*) phải kiểm tra **khi lấy ra**. Chi phí không đồng nhất thì một đường được sinh ra trước chưa chắc là đường rẻ nhất (`tests/test_ucs_solver.py::test_ucs_tests_goal_on_pop_not_on_generate`). Nếu kiểm tra sớm như BFS, UCS sẽ mất tính tối ưu.

**Bằng chứng.** Mình chạy thêm một bản BFS chỉ để đo, kiểm tra đích **khi lấy ra**. Số node mở rộng của nó **trùng khớp với UCS trên cả 16 màn**:

| Màn | BFS (đích khi sinh) | BFS (đích khi lấy ra) | UCS | Chênh BFS → UCS |
|---:|---:|---:|---:|---:|
| 1 | 125 | 160 | 160 | +35 |
| 2 | 371 | 377 | 377 | +6 |
| 3 | 4 | 4 | 4 | +0 |
| 4 | 126 | 127 | 127 | +1 |
| 5 | 34 | 52 | 52 | +18 |
| 6 | 214 | 215 | 215 | +1 |
| 7 | 209 | 238 | 238 | +29 |
| 8 | 206 | 232 | 232 | +26 |
| 9 | 1.319 | 1.393 | 1.393 | +74 |
| 10 | 1.190 | 1.201 | 1.201 | +11 |
| 11 | 113 | 113 | 113 | +0 |
| 12 | 6.661 | 6.663 | 6.663 | +2 |
| 13 | 47.673 | 47.675 | 47.675 | +2 |
| 14 | 679 | 687 | 687 | +8 |
| 15 | 330 | 351 | 351 | +21 |
| 16 | 4.503 | 4.600 | 4.600 | +97 |

Như vậy toàn bộ phần chênh là các node ở tầng cuối mà UCS phải lấy ra trước khi tới lượt đích. Màn 11 vô nghiệm nên cả hai duyệt hết, không chênh.

Thời gian của UCS cũng nhỉnh hơn BFS (màn 13: 2.139 so với 1.766 ms), vì `heapq` tốn O(log n) mỗi thao tác còn `deque` chỉ tốn O(1).

**Cách viết trong báo cáo.** Khi so BFS với UCS, ghi rõ hai thuật toán kiểm tra đích ở thời điểm khác nhau. Với chi phí bằng 1 thì chúng **duyệt cùng một tập trạng thái theo cùng thứ tự tầng**; phần chênh chỉ là tầng cuối. Lợi thế thật của UCS chỉ lộ ra khi dùng **chi phí không đồng nhất** (phần TV3 còn làm): khi đó BFS vẫn chọn đường ít bước, còn UCS chọn đường rẻ nhất (`test_ucs_prefers_cheaper_path_over_shorter_one`).

### 6.2. "Frontier max" của **A\*** tính cả bản ghi cũ; của UCS thì chính xác (khi chi phí bằng 1)

**Hiện tượng.** Ở màn 10, frontier lớn nhất của A\* là **480**, gấp 5,5 lần UCS (87), dù A\* mở rộng ít node hơn. Đọc như vậy dễ hiểu nhầm rằng A\* phải giữ 480 trạng thái khác nhau cùng lúc.

**Nguyên nhân.** UCS và A\* dùng `heapq`, mà `heapq` không có thao tác "giảm khóa". Khi tìm được đường rẻ hơn tới một trạng thái đang chờ, code **đẩy thêm một bản ghi mới** và để bản ghi cũ nằm lại; khi bản ghi cũ được lấy ra thì bỏ qua (lazy deletion). `max_frontier` đo **số phần tử trong heap**, nên tính cả các bản ghi cũ này. BFS và DFS không bao giờ đẩy một trạng thái hai lần, nên frontier của chúng chính là số trạng thái khác nhau.

**Bằng chứng.** Mình đo song song kích thước heap và số **trạng thái thật** đang chờ (chưa mở rộng), cùng tổng số bản ghi cũ đã đẩy vào. Đo bằng cách bọc `heappush`/`heappop`, không sửa code solver.

| Màn | UCS chi phí 1: heap / trạng thái thật / bản ghi cũ | A\* chi phí 1: heap / trạng thái thật / bản ghi cũ | UCS chi phí 1..7 | A\* chi phí 1..7 |
|---:|---|---|---|---|
| 1 | 47 / 47 / 0 | 60 / 57 / 6 | 45 / 40 / 17 | 54 / 47 / 14 |
| 2 | 14 / 14 / 0 | 14 / 14 / 0 | 11 / 11 / 3 | 12 / 12 / 2 |
| 3 | 1 / 1 / 0 | 1 / 1 / 0 | 1 / 1 / 0 | 1 / 1 / 0 |
| 4 | 23 / 23 / 0 | 24 / 23 / 2 | 24 / 24 / 1 | 22 / 22 / 1 |
| 5 | 32 / 32 / 0 | 15 / 15 / 0 | 36 / 34 / 2 | 30 / 30 / 0 |
| 6 | 45 / 45 / 0 | 37 / 35 / 10 | 61 / 56 / 19 | 58 / 50 / 22 |
| 7 | 41 / 41 / 0 | 97 / 87 / 14 | 48 / 42 / 32 | 51 / 45 / 30 |
| 8 | 31 / 31 / 0 | 57 / 53 / 18 | 35 / 30 / 27 | 40 / 36 / 27 |
| 9 | 83 / 83 / 0 | 143 / 101 / 201 | 146 / 123 / 240 | 122 / 106 / 223 |
| 10 | 87 / 87 / 0 | **480 / 235 / 475** | 148 / 122 / 239 | 219 / 166 / 260 |
| 11 | 39 / 39 / 0 | 40 / 33 / 19 | – | – |
| 12 | 478 / 478 / 0 | 651 / 521 / 1.185 | 603 / 507 / 1.139 | 609 / 507 / 1.301 |
| 13 | 3.252 / 3.252 / 0 | 3.744 / 3.121 / 9.591 | 3.621 / 2.982 / 8.851 | 3.774 / 3.016 / 10.099 |
| 14 | 52 / 52 / 0 | 64 / 59 / 43 | 48 / 48 / 23 | 54 / 53 / 26 |
| 15 | 27 / 27 / 0 | 27 / 27 / 14 | 34 / 33 / 17 | 33 / 32 / 16 |
| 16 | 200 / 200 / 0 | 292 / 292 / 12 | 316 / 232 / 1.521 | 368 / 255 / 1.579 |

(Cột chi phí 1..7 dùng bộ chi phí ngẫu nhiên số 0 của mục 2.2. Màn 11 vô nghiệm nên không đo.)

Đọc bảng:

- **UCS với chi phí bằng 1 không sinh bản ghi cũ nào** trên cả 16 màn. UCS lấy node theo đúng thứ tự g, nên lần đầu một trạng thái được sinh ra đã là đường rẻ nhất tới nó. Frontier của UCS vì vậy **chính xác**, so trực tiếp được với BFS.
- **A\*** có bản ghi cũ ngay cả khi heuristic consistent. A\* lấy node theo f chứ không theo g, nên một trạng thái **đang chờ** có thể được tìm thấy lại qua đường rẻ hơn trước khi đến lượt nó. Ở màn 10, gần **một nửa** heap là bản ghi cũ (480 so với 235); màn 13 có 9.591 lần đẩy thừa. (Trạng thái **đã mở rộng** thì không bao giờ bị mở lại, vì heuristic consistent; mục 2.2 đã kiểm.)
- Với **chi phí không đồng nhất**, cả UCS và A\* đều có bản ghi cũ. Ví dụ màn 16, UCS: heap 316 nhưng chỉ có 232 trạng thái thật.

> **Đính chính:** khi rà soát lần đầu, mình nói chung là "frontier max của UCS/A\* tính cả bản ghi cũ". Số đo cho thấy điều đó chỉ đúng với **A\*** khi chi phí bằng 1, và đúng với **cả hai** khi chi phí không đồng nhất.

**Cách viết trong báo cáo.** Định nghĩa rõ: "frontier max = số phần tử lớn nhất trong cấu trúc frontier, với UCS/A\* là heap, **kể cả bản ghi cũ**". Định nghĩa này phản ánh đúng **bộ nhớ** thực dùng: cột KB của A\* cũng cao tương ứng, ví dụ màn 10 là 237,9 KB so với 184,1 KB của UCS. Khi bàn về "số trạng thái phải giữ cùng lúc", dùng cột "trạng thái thật" ở bảng trên. Có thể nêu thêm như một điểm yếu của cách cài lazy deletion: đổi bộ nhớ lấy sự đơn giản, thay vì dùng cấu trúc heap có thao tác giảm khóa.

### 6.3. Ở màn 13, A\* gần như không cắt tỉa được và còn **chậm hơn** BFS

**Hiện tượng.** Màn 13: A\* mở rộng 47.657 node, chỉ ít hơn UCS 18 node (0,04%), nhưng chạy 2.809 ms so với 2.139 ms của UCS và 1.766 ms của BFS. Ngược lại ở màn 1, A\* bớt được gần một nửa (81 so với 160), và ở màn 5 bớt được 73% (14 so với 52).

**Lời giải thích dễ nghĩ tới đầu tiên chưa đủ.** Có thể nghĩ "heuristic yếu ở màn 13". Nhưng tỉ lệ h/h\* trung bình trên mọi trạng thái của màn 13 là **0,40**, gần bằng màn 1 (**0,46**), mà màn 1 lại cắt tỉa tốt. (Lần rà soát đầu mình cũng giải thích theo hướng "Manhattan bỏ qua cầu"; đó chỉ là một phần, xem bên dưới.)

**Nguyên nhân thật: đích gần như là trạng thái xa nhất của đồ thị.** Theo lý thuyết, với C\* là chi phí tối ưu:

- UCS **buộc phải** mở rộng mọi trạng thái có g(s) < C\*.
- A\* **buộc phải** mở rộng mọi trạng thái có f(s) = g(s) + h(s) < C\*.

Với g(s) là khoảng cách thật từ điểm xuất phát, đếm hai tập này trên toàn bộ đồ thị:

| Màn | Trạng thái tới được | C\* | Có g < C\* (UCS buộc mở rộng) | Có g + h < C\* (A\* buộc mở rộng) | A\* cắt bớt | Số cấu hình cầu trung bình trên mỗi vị trí khối |
|---:|---:|---:|---:|---:|---:|---:|
| 5 | 120 | 6 | 49 | 12 | 75,5% | 1,0 |
| 1 | 1.028 | 10 | 153 | 80 | 47,7% | 7,7 |
| 4 | 128 | 16 | 127 | 69 | 45,7% | 1,0 |
| 9 | 1.917 | 36 | 1.392 | 923 | 33,7% | 2,4 |
| 14 | 703 | 36 | 681 | 550 | 19,2% | 1,0 |
| 15 | 494 | 37 | 345 | 302 | 12,5% | 3,5 |
| 10 | 1.206 | 27 | 1.201 | 1.125 | 6,3% | 1,0 |
| 2 | 676 | 68 | 377 | 356 | 5,6% | 5,0 |
| 12 | 6.668 | 44 | 6.661 | 6.340 | 4,8% | 15,0 |
| 16 | 5.324 | 49 | 4.600 | 4.454 | 3,2% | 3,5 |
| 6 | 216 | 20 | 215 | 213 | 0,9% | 1,0 |
| **13** | **47.680** | **66** | **47.673** | **47.655** | **0,0%** | **60,1** |

(Số node mở rộng thật ở mục 5 khớp với hai cột "buộc mở rộng", chỉ cộng thêm vài node có f = C\* được lấy ra trước đích.)

Ở màn 13, **47.673/47.680 trạng thái (99,99%) gần điểm xuất phát hơn cả đích**. Bất kỳ thuật toán nào **bảo đảm tối ưu** cũng phải đi qua gần hết đồ thị, vì chưa xét hết các trạng thái đó thì chưa thể chắc không có đường ngắn hơn. Heuristic chỉ cắt được những trạng thái có g + h ≥ 66, mà gần như không có trạng thái nào như vậy. Màn 6 và màn 10 **không có cầu** nhưng cũng cắt được rất ít (0,9% và 6,3%), cùng một lý do: đích nằm ở rìa xa nhất của đồ thị.

**Phần cầu làm tình hình tệ thêm.** Màn 13 có 6 công tắc, trung bình mỗi vị trí và tư thế của khối đi kèm **60 cấu hình cầu**. Manhattan chỉ nhìn vị trí khối nên cho **cùng một giá trị h** cho cả 60 cấu hình. Vì vậy nó không thể ưu tiên "đã mở đúng cầu" hơn "chưa mở cầu nào".

**Vì sao A\* còn chậm hơn.** Khi gần như không cắt được node nào, A\* làm đúng khối lượng của UCS nhưng mỗi node tốn hơn: tính h cho mỗi con sinh ra, phần tử heap 5 thành phần thay vì 3, và thêm 9.591 lần đẩy bản ghi cũ (mục 6.2). Trên các màn lớn, thời gian mỗi lần mở rộng của A\* khoảng **59 µs**, so với khoảng 45 µs của UCS và 37 µs của BFS (màn 13).

**Cách viết trong báo cáo.** Đây là **điểm yếu của A\* + Manhattan**, không phải lỗi cài đặt, và là một ý hay cho phần nhận xét điểm mạnh/yếu:

- A\* mạnh khi đích nằm "ở giữa" đồ thị và hướng đi về đích khá thẳng (màn 1, 4, 5, 9).
- A\* gần như vô dụng khi lời giải buộc phải đi vòng qua gần hết không gian trạng thái (màn 6, 10, 12, 13, 16), vì heuristic chỉ dựa trên khoảng cách hình học.
- Hướng cải thiện (chỉ là ý tưởng, **chưa cài**): heuristic có tính đến công tắc. Ví dụ, nếu cầu bắt buộc phải đi qua đang đóng thì cộng thêm quãng đường tới công tắc tương ứng. Cách này phải chứng minh lại tính admissible.

---

## 7. Gợi ý dùng trong `Report.pdf`

1. Mục mô tả BFS/UCS: nêu rõ thời điểm kiểm tra đích của từng thuật toán và lý do (mục 6.1).
2. Mục định nghĩa chỉ số: định nghĩa "node mở rộng", "node sinh ra", "frontier max" theo đúng `base_solver.py`; ghi chú frontier của UCS/A\* là kích thước heap kể cả bản ghi cũ (mục 6.2).
3. Bảng thực nghiệm: dùng hai bảng ở mục 5 (đã là số liệu sau khi sửa BFS). Nếu chạy lại trên máy khác, số node phải trùng khớp, chỉ thời gian và bộ nhớ thay đổi.
4. Phần nhận xét điểm mạnh/yếu: dùng bảng ở mục 6.3 để giải thích vì sao A\* không phải lúc nào cũng nhanh hơn.
5. Khi TV3 làm xong hàm chi phí không đồng nhất: chạy lại các bảng ở mục 5 và 6.2. Đó là lúc UCS khác BFS thật sự, và nên có một ví dụ màn mà UCS chọn đường dài hơn nhưng rẻ hơn.

---

## 8. File đã đổi

| File | Thay đổi |
|---|---|
| `ai/solvers/bfs_solver.py` | Sửa bộ đếm `nodes_generated` (mục 3) |
| `tests/test_bfs_solver.py` | +1 test tái hiện lỗi |
| `tests/test_search_audit.py` | Mới: 136 test kiểm chứng chéo (mục 4) |
| `Docs/report6.md` | Báo cáo này |
| `README.md`, `Docs/play.md` | Cập nhật số test, thêm liên kết tới report6 |

## 9. Việc còn lại

- **Hàm chi phí không đồng nhất (TV3)** vẫn chưa có. UCS/A\* đã sẵn sàng và đã được test với chi phí ngẫu nhiên; khi có hàm chi phí thật, nhớ nhân heuristic với chi phí nhỏ nhất của một nước để giữ admissible (docstring `manhattan_heuristic`).
- Phần thực nghiệm tự động (`experiments/run_all.py`, biểu đồ) chưa có. Các bảng ở mục 5 và 6 có thể dùng làm số liệu đối chiếu.
