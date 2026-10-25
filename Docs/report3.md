# Report 3: Review và merge bộ màn v1 + A\* (nhóm 9)

> **Ngày:** 25/10/2026 · **Người thực hiện:** Kiên (review + merge) · **Code của:** Sang (TV4)
> **Phạm vi:** hai nhánh `feat/sang/level-suite` và `feat/sang/astar-solver`, merge vào `main` theo thứ tự đó, kèm một commit dọn dẹp tài liệu.

---

## 1. Tóm tắt

- Đã merge **bộ màn v1** (11 màn) và **A\* solver** vào `main`. Hai nhánh không đụng file của nhau, merge không xung đột.
- Test sau merge: **88 passed, 14 skipped** (14 test SKIP chờ BFS, DFS, UCS và `manhattan_heuristic`).
- **Lời giải của cả 10 màn có lời giải đều thắng trong game C++ thật** (`Bloxorz.exe --play`), đúng số bước. Vậy C++ và Python đọc màn và áp luật giống nhau.
- A\* (chạy với `--heuristic zero`) giải đúng độ dài tối ưu ở cả 10 màn và báo vô nghiệm đúng ở màn 11.
- **Lưu ý cho cả nhóm:** nút **A\*** trong game vẫn báo "chưa cài" cho tới khi TV5 cài `manhattan_heuristic`, vì đó là heuristic mặc định.

## 2. Lịch sử commit sau merge

```
* fbe4054 docs: update status after merging level suite and A* solver
*   4e5ad80 feat(astar): merge feat/sang/astar-solver into main
|\
| * 22f0554 docs(astar): explain search and heuristic integration
| * a6a5264 test(astar): verify heuristics against exhaustive BFS distances
| * fbc340f Merge remote-tracking branch 'origin/main' into feat/sang/astar-solver
| * ad60111 test(astar): cover edge cases and improved paths
| * e677316 feat(astar): implement A* search and path reconstruction
* | 8e79549 feat(levels): merge feat/sang/level-suite into main
|\ \
| * c6833d5 docs(levels): document catalog, replays and verification
| * bd16d85 test(levels): certify optimal paths and essential mechanics
| * 334980b feat(levels): add reproducible board previews
| * 214c36c feat(levels): add nine v1 levels and certified solutions
|/
* 6d54dad feat(gui): merge feat/kien/gui-upgrade into main   ← origin/main trước khi merge
```

Merge dùng `--no-ff` (giữ commit merge), giống cách đã merge `feat/kien/gui-upgrade`, để lịch sử thấy rõ từng nhánh.

## 3. Nhánh `feat/sang/level-suite`: bộ màn v1

### 3.1. Nội dung

| Thành phần | Mô tả |
|---|---|
| `code/data/3.data` … `11.data` | 9 màn mới (màn 1, 2 là màn gốc) |
| `code/data/catalog.json` | Thông tin từng màn: nhóm độ khó, kích thước, số ô, ô cam, công tắc, ô cầu, độ dài tối ưu, file lời giải, ảnh |
| `code/data/solutions/<n>.txt` | Lời giải tối ưu (chi phí 1) của 10 màn có lời giải |
| `code/data/previews/*.png` | Sơ đồ từng màn + ảnh tổng hợp `overview.png`, dùng được cho báo cáo |
| `code/data/README.md` | Danh mục màn, cách phát lại, cách kiểm chứng |
| `scripts/render_level_previews.py` | Sinh lại ảnh từ file `.data` (cần Pillow, chỉ dùng khi vẽ ảnh; solver và test không cần) |
| `tests/test_levels.py`, `tests/helpers.py` | Kiểm tra catalog khớp file màn, độ dài tối ưu bằng BFS độc lập, màn vô nghiệm, viền trống, công tắc bắt buộc, ô cam thật sự làm lời giải dài ra, ảnh hợp lệ |

### 3.2. Danh sách màn

| Màn | Nhóm | Tên | Kích thước | Ô cam | Công tắc (thường / nặng) | Tối ưu |
|---:|---|---|---|---:|---|---:|
| 1 | G3 | Upstream level 1 | 13×13 | 1 | 2 / 1 | 10 |
| 2 | G3 | Upstream level 2 | 19×14 | 39 | 0 / 3 | 68 |
| 3 | G1 | First rolls | 7×13 | 0 | 0 / 0 | 4 |
| 4 | G1 | Turning courtyard | 11×15 | 0 | 0 / 0 | 16 |
| 5 | G2 | Fragile detour | 9×13 | 1 | 0 / 0 | 6 |
| 6 | G2 | Fragile maze | 12×16 | 1 | 0 / 0 | 20 |
| 7 | G3 | Soft switch crossing | 11×19 | 0 | 1 / 0 | 12 |
| 8 | G3 | Heavy switch crossing | 11×19 | 0 | 0 / 1 | 15 |
| 9 | G5 | Three chambers | 19×25 | 4 | 1 / 1 | 36 |
| 10 | G6 | Open field | 20×30 | 0 | 0 / 0 | 27 |
| 11 | G6 | Disconnected islands | 11×17 | 0 | 0 / 0 | vô nghiệm |

Đã đạt yêu cầu Q&A "tối thiểu 10 test case". Chưa có nhóm G4 (split switch) vì cơ chế split chưa được cài.

### 3.3. Kiểm chứng trong game C++

Lệnh (chạy trong `code/`): `Bloxorz.exe <n> --play data/solutions/<n>.txt`

| Màn | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| Kết quả | WON | WON | WON | WON | WON | WON | WON | WON | WON | WON |
| Số bước | 10 | 68 | 4 | 16 | 6 | 20 | 12 | 15 | 36 | 27 |

Mọi màn đều thắng đúng ở nước cuối (mã thoát 0) với số bước bằng `optimal_length` trong catalog.

## 4. Nhánh `feat/sang/astar-solver`: A\*

### 4.1. Nội dung

| File | Mô tả |
|---|---|
| `ai/solvers/astar_solver.py` | Cài `AStarSolver.solve()`: heap `(f, h, counter, g, state)`, `best_g` + lazy deletion, kiểm tra đích khi lấy ra, cho phép mở rộng lại khi tìm được `g` tốt hơn |
| `tests/test_astar_solver.py` | Trường hợp biên trên đồ thị nhỏ: vô nghiệm, xuất phát tại đích, phải chờ đích rẻ nhất được lấy ra, heuristic admissible nhưng không consistent (buộc mở rộng lại), bỏ qua bản ghi cũ |
| `tests/test_astar_integration.py` | Dựng toàn bộ đồ thị trạng thái của màn 1 và 2, tính khoảng cách thật tới đích bằng BFS ngược, rồi kiểm tra heuristic admissible và consistent trên **mọi** trạng thái và cạnh, và A\* trả đúng chi phí tối ưu. Phần `manhattan` SKIP tới khi TV5 cài xong |
| `Docs/astar_walkthrough.md` | Giải thích code A\* theo từng bước, chuẩn bị cho báo cáo và vấn đáp |

### 4.2. Nhận xét review

- **Đúng thuật toán:** khớp với mô tả trong `Docs/system.md` mục 7.4. Kiểm tra đích khi lấy ra nên tối ưu. Cho phép mở rộng lại nên vẫn tối ưu khi heuristic chỉ admissible mà không consistent.
- **Cách đếm:** `nodes_expanded` không tính bản ghi cũ và trạng thái đích. `nodes_generated` tính mọi con hợp lệ. `max_frontier` tính **cả bản ghi cũ** còn trong heap. Khi cài BFS/DFS/UCS, các thành viên nên thống nhất định nghĩa này để bảng thực nghiệm công bằng.

### 4.3. A\* (`h = 0`, chi phí 1) trên toàn bộ màn

| Màn | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Số bước | 10 | 68 | 4 | 16 | 6 | 20 | 12 | 15 | 36 | 27 | vô nghiệm |
| Node mở rộng | 160 | 377 | 4 | 127 | 52 | 215 | 238 | 232 | 1393 | 1201 | 113 |

Thời gian mọi màn đều dưới 30 ms. Đây là số đo nhanh một lần trên máy Kiên, **không phải số liệu thực nghiệm chính thức**.

## 5. Commit dọn dẹp `fbe4054`

- `Docs/astar_walkthrough.md`: bỏ đường dẫn Python riêng trên máy Sang (`..\.runtime\astar-python312\python.exe`).
- `README.md`: đánh dấu `AStarSolver` đã cài; ghi chú nút A\* chờ `manhattan_heuristic`; đổi "chờ nghiệm thu phát lại C++" thành đã kiểm chứng; cập nhật số test (88 passed, 14 skipped); thêm mô tả hai file test A\*.

## 6. Việc tiếp theo

| Ai | Việc | Vì sao |
|---|---|---|
| TV5 | Cài `manhattan_heuristic` | Nút A\* trong game và test tích hợp A\* đang chờ phần này. Test `test_astar_integration.py` sẽ tự kiểm tra admissible/consistent khi bỏ `NotImplementedError` |
| TV1, TV2, TV3 | Cài BFS, DFS, UCS | Thống nhất cách đếm `nodes_expanded` / `max_frontier` với A\* (mục 4.2) |
| TV3 | Hàm chi phí không đồng nhất + tham số chọn trong `main.py` | `main.py` luôn dùng chi phí 1 (xem `Docs/system.md` mục 7.3.6) |
| TV4 | Thêm 1–2 màn có **4–6 công tắc** và màn có **nhiều ô cam** | Màn nặng nhất mới khoảng 1400 node, chạy dưới 30 ms, nên khác biệt giữa thuật toán chưa rõ cho phần Experiments. Hai màn G2 mới có 1 ô cam mỗi màn. Màn nhiều ô cam cũng giúp minh họa hàm chi phí phạt ô cam |
| Cả nhóm | Biết trước: thắng màn 10 rồi bấm Next sẽ vào màn 11 (vô nghiệm) | Tránh bất ngờ khi quay video |
| Cả nhóm | Xóa nhánh đã merge (tùy chọn) | `feat/kien/gui-upgrade`, `feat/sang/level-suite`, `feat/sang/astar-solver` đã nằm trong `main` |

## 7. Công cụ

Đã cài **GitHub CLI** (`gh` 2.103.2) tại `D:\DevTools\GitHubCLI` để xem và merge pull request từ terminal. Cần đăng nhập một lần bằng `gh auth login` trước khi dùng.
