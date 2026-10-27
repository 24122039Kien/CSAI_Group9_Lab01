# Giải thích và hướng dẫn thuật toán Breadth-First Search (BFS)

Tài liệu đi cùng [ai/solvers/bfs_solver.py](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/ai/solvers/bfs_solver.py), dùng để đọc hiểu code,
chuẩn bị review, viết mục "3.2 Breadth-First Search" và phần thực nghiệm trong `Report.pdf`.
Thuật toán được xây dựng và kiểm thử bởi TV1 (Đạt), phụ trách module BFS và mở rộng cơ chế game.

---

## 1. BFS đang giải bài toán gì trong Bloxorz?

Trong Bloxorz, mỗi nút trong không gian trạng thái là một đối tượng:
```python
State(x, y, orientation, bridge_mask, cubes)
```
- **Tọa độ & Tư thế**: `(x, y)` cùng `Orientation` (`Z` - đứng thẳng, `X` - nằm dọc, `Y` - nằm ngang).
- **Cấu hình cầu (`bridge_mask`)**: Biểu diễn trạng thái đóng/mở của các ô cầu trên bàn chơi. Cùng vị trí và tư thế nhưng cấu hình cầu khác nhau đại diện cho hai trạng thái không gian hoàn toàn khác nhau.
- **Trạng thái tách (`cubes`)**: Khi đi vào ô split switch, khối phân rã thành hai cube 1×1×1 độc lập.

**Mục tiêu của BFS**: Tìm chuỗi hành động hợp lệ ngắn nhất (tối ưu tuyệt đối về số nước đi) đưa khối từ trạng thái khởi đầu `problem.initial_state()` đến khi đứng thẳng tại ô đích `problem.is_goal(state)`.

---

## 2. Nguyên lý hoạt động & Tính tối ưu của BFS

### 2.1. Duyệt theo tầng (Layer-by-layer)
BFS duyệt toàn bộ các trạng thái cách gốc khoảng cách $k$ bước trước khi xét tới bất kỳ trạng thái nào ở khoảng cách $k + 1$.
Nhờ sử dụng hàng đợi **FIFO (First-In, First-Out)** thông qua `collections.deque`:
- Trạng thái sinh trước được đưa vào đuôi (`append`).
- Trạng thái xét trước được lấy ra từ đầu (`popleft`).

### 2.2. Chứng minh tính tối ưu (Optimality)
Trong Bloxorz, mỗi bước lăn khối (hoặc di chuyển một cube) đều có chi phí đơn vị bằng nhau:
$$c(s, a, s') = 1 \quad \forall s, a$$
Do mọi cạnh trên cây tìm kiếm có trọng số dương bằng nhau, hàm chi phí đường đi $g(s)$ tỉ lệ thuận với độ sâu $d(s)$ trên cây tìm kiếm:
$$g(s) = d(s)$$
Vì BFS duyệt tuần tự theo thứ tự tăng dần của độ sâu $d$, trạng thái đích đầu tiên được phát hiện chắc chắn có độ sâu $d^*$ nhỏ nhất. Do đó, lời giải mà BFS tìm được luôn đảm bảo **tối ưu tuyệt đối về số bước đi**.

---

## 3. Cấu trúc dữ liệu & Thuật toán chi tiết

| Cấu trúc dữ liệu | Kiểu dữ liệu | Vai trò kỹ thuật |
|---|---|---|
| `frontier` | `collections.deque[State]` | Hàng đợi FIFO chứa các trạng thái biên đang chờ duyệt |
| `parent` | `dict[State, tuple[State, str] | None]` | Vừa làm tập đã thăm (`visited`), vừa lưu `(state_cha, action)` để truy vết đường đi |
| `self.nodes_expanded` | `int` | Đếm số lượng node đã lấy ra khỏi frontier và sinh con |
| `self.nodes_generated` | `int` | Đếm tổng số lượng node con hợp lệ được sinh ra |
| `self.max_frontier` | `int` | Kích thước lớn nhất của hàng đợi `frontier` trong suốt quá trình chạy |

### Cơ chế Early Goal Test (Kiểm tra đích sớm)
Trong bài toán có chi phí bước đồng nhất $c = 1$, việc kiểm tra đích ngay khi sinh node con (`Early Goal Test`) bảo đảm:
1. **Bảo toàn tính tối ưu**: Trạng thái con được sinh ra ở tầng $d$ sẽ được phát hiện ngay lập tức mà không cần đợi nó được lấy ra khỏi `frontier` ở tầng $d$.
2. **Tiết kiệm tài nguyên**: Giảm đáng kể số node mở rộng (`nodes_expanded`) và số node sinh ra ở tầng cuối cùng, giúp thuật toán chạy nhanh hơn rõ rệt.

---

## 4. Các bước thực thi trong `BFSSolver.solve()`

1. **Khởi tạo & Xét biên**:
   - Lấy `start = self.problem.initial_state()`.
   - Nếu `is_goal(start)`: Trả về ngay `make_result([])` (0 bước đi, `nodes_expanded = 0`, `max_frontier = 1`).
2. **Nạp gốc vào hàng đợi**:
   - `frontier = deque([start])`
   - `parent = {start: None}`
   - `self.max_frontier = 1`
3. **Vòng lặp mở rộng**:
   - Trong khi `frontier` không rỗng:
     - Lấy `state = frontier.popleft()`.
     - Tăng `self.nodes_expanded += 1`.
     - Gọi `self.problem.successors(state)` để sinh các nhánh `(action, nxt, cost)`.
     - Với mỗi nhánh:
       - Tăng `self.nodes_generated += 1`.
       - Nếu `nxt not in parent`:
         - Đánh dấu đã thăm: `parent[nxt] = (state, action)`.
         - **Kiểm tra đích**: Nếu `self.problem.is_goal(nxt)`:
           - Dùng `self.reconstruct_path(parent, nxt)` để đảo ngược đường đi từ đích về gốc.
           - Trả về `self.make_result(path)`.
         - Nếu chưa tới đích: đẩy vào `frontier.append(nxt)` và cập nhật `self.max_frontier`.
4. **Không có lời giải**:
   - Nếu duyệt cạn `frontier` mà không tìm thấy đích: Trả về `self.make_result(None)` (`status = "no_solution"`).

---

## 5. Bảng đánh giá thực nghiệm (Benchmark Results)

Kết quả đo đạc thực tế từ script [scripts/benchmark_bfs.py](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/scripts/benchmark_bfs.py) trên toàn bộ bộ dữ liệu chuẩn (Levels 1 - 13):

| Màn | Trạng thái | Số bước tối ưu | Nodes Expanded | Nodes Generated | Max Queue | Thời gian (ms) | Peak RAM (KB) | Đặc điểm màn chơi |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|---|
| **1** | THẮNG (OK) | **10** | 125 | 343 | 36 | 10.94 ms | 26.3 KB | Màn cơ bản làm quen |
| **2** | THẮNG (OK) | **68** | 371 | 749 | 14 | 24.01 ms | 62.3 KB | Công tắc cầu đôi đảo chiều |
| **3** | THẮNG (OK) | **4** | 4 | 6 | 1 | 0.34 ms | 2.1 KB | Map mini đường thẳng |
| **4** | THẮNG (OK) | **16** | 126 | 275 | 23 | 8.97 ms | 15.8 KB | Ô cam dễ vỡ (fragile) |
| **5** | THẮNG (OK) | **6** | 34 | 100 | 19 | 1.94 ms | 7.6 KB | Hành lang ô cam ngắn |
| **6** | THẮNG (OK) | **20** | 214 | 598 | 45 | 11.80 ms | 27.1 KB | Cầu và công tắc mềm |
| **7** | THẮNG (OK) | **12** | 209 | 633 | 41 | 10.27 ms | 28.2 KB | Công tắc nặng (heavy bridge) |
| **8** | THẮNG (OK) | **15** | 206 | 621 | 31 | 13.72 ms | 27.6 KB | Kết hợp ô cam & công tắc |
| **9** | THẮNG (OK) | **36** | 1,319 | 4,467 | 83 | 108.00 ms | 260.0 KB | Không gian rộng, nhiều ngõ cụt |
| **10** | THẮNG (OK) | **27** | 1,190 | 4,449 | 87 | 71.55 ms | 123.5 KB | Mê cung nhiều cầu đan xen |
| **11** | VÔ NGHIỆM | — | 113 | 352 | 39 | 6.03 ms | 14.4 KB | Map cô lập (Test No-Solution) |
| **12** | THẮNG (OK) | **37** | 330 | 752 | 22 | 22.16 ms | 52.7 KB | **Multi-Bridge & Permanent Switch** |
| **13** | THẮNG (OK) | **49** | 4,503 | 22,199 | 199 | 670.59 ms | 1,109.0 KB | **Split Switch & Recombination** |

### Nhận xét chuyên sâu:
1. **Tính hoàn hảo trên cơ chế mới (v2 Mechanics)**:
   - Level 12 (Multi-bridge & Permanent switch): BFS giải quyết mượt mà trong 37 bước chỉ với 22.16 ms.
   - Level 13 (Stage 10 split switch): Không gian trạng thái mở rộng lên 8 hành động khi tách (`0U..1R`), BFS tự động điều phối 2 cube dẫm switch mở cầu, né ô cam và gộp khối thành công chỉ trong 0.67 giây và 1.1 MB RAM.
2. **Khả năng nhận diện vô nghiệm (Level 11)**:
   - BFS duyệt cạn toàn bộ thành phần liên thông gồm 113 node chỉ trong 6 ms và trả về `status = "no_solution"` chính xác mà không gặp lỗi lặp vô tận.

---

## 6. Đánh giá độ phức tạp & So sánh thuật toán

- **Độ phức tạp thời gian**: $O(b^d)$, trong đó $b$ là hệ số nhánh trung bình ($b \approx 2.5 - 4$ với khối nguyên; $b \approx 6 - 8$ với khối tách), $d$ là độ sâu lời giải ngắn nhất.
- **Độ phức tạp không gian**: $O(b^d)$, BFS phải lưu toàn bộ frontier và parent trong bộ nhớ.
- **So sánh**:
  - **So với DFS**: BFS bảo đảm tìm được lời giải ngắn nhất, không bao giờ bị lạc vào nhánh vô tận; tuy nhiên tốn bộ nhớ RAM hơn DFS.
  - **So với A\***: BFS không cần hàm Heuristic hướng dẫn. Với các bài toán có không gian trạng thái vừa và nhỏ, BFS rất nhanh và đơn giản; trên các màn cực lớn, A* với Heuristic admissible sẽ thu hẹp không gian tìm kiếm tốt hơn.
