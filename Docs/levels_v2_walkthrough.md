# Hướng dẫn và giải thích bộ màn chơi v2 (Level 15 & 16)

Hai màn dùng phần mở rộng `ext 1` của định dạng `.data` (Đạt, nhánh `feat/dat/v2-mechanics`).
Khi merge vào `main` (report4), hai màn được đổi số từ 12/13 thành **15/16** vì màn 12–14 đã là
màn khó của Sang. Số liệu dưới đây lấy từ file dữ liệu và lời giải hiện tại.

---

## 1. Tổng quan

| Màn | Nhóm | Tên | Kích thước | Ô sàn | Công tắc | Ô cầu | Tối ưu |
|:---:|:---:|---|:---:|---:|---|:---:|:---:|
| **15** | G3 | Dual bridges crossing | 14×17 | 57 | 2 thường (1 `toggle` hai ô, 1 `permanent_on`) | 3 | **37 bước** |
| **16** | G4 | The great split | 14×18 | 48 | 2 thường `toggle` (2 ô và 4 ô) + 1 split | 6 | **49 bước** |

---

## 2. Chi tiết

### 2.1. Màn 15 — Dual Bridges Crossing (G3)

- Xuất phát đứng tại `(8, 4)`, lỗ đích `(6, 6)`.
- Công tắc thường `(4, 8)` (`toggle`) bật/tắt **hai** ô cầu `(4, 14)` và `(5, 14)` cùng lúc.
- Công tắc thường `(2, 14)` (`permanent_on`) mở ô cầu `(6, 8)`; bấm lại không đóng.
- Diễn biến của lời giải tối ưu:
  - nước 14 nằm đè lên `(4, 8)` → mở hai ô cầu phía đông;
  - nước 24 đứng lên `(2, 14)` → mở vĩnh viễn ô `(6, 8)`;
  - nước 34 lại đè lên `(4, 8)` → hai ô cầu phía đông đóng lại (không còn cần), ô `(6, 8)` vẫn mở.
- File: [code/data/15.data](../code/data/15.data), [code/data/solutions/15.txt](../code/data/solutions/15.txt).

### 2.2. Màn 16 — The Great Split (G4)

- Xuất phát đứng tại `(3, 11)`, lỗ đích `(3, 3)`.
- Split switch `(3, 14)`: đứng lên thì khối tách thành hai khối 1×1×1 tại `(3, 11)` và `(3, 14)`.
- Công tắc thường `(11, 7)` mở hai ô cầu `(3, 5)`, `(3, 6)`; công tắc thường `(11, 13)` mở bốn ô cầu
  `(3, 8)`, `(3, 9)`, `(4, 14)`, `(5, 14)`. Cả hai nằm ở dải phía nam, chỉ khối nhỏ tới được.
- Diễn biến của lời giải tối ưu:
  - nước 2: đứng lên split switch → tách;
  - khối nhỏ số 1 đi xuống phía nam: đè `(11, 13)` ở nước 15 (mở 4 ô), nước 17 đi ngang qua đè lại
    (4 ô đóng, vì công tắc nằm trên đường đi), đè `(11, 7)` ở nước 25 (mở 2 ô) rồi quay về đè
    `(11, 13)` ở nước 33 (mở lại 4 ô) để cả 6 ô cầu cùng mở;
  - nước 42: khối nhỏ số 1 quay lên `(4, 14)` cạnh khối số 0 ở `(3, 13)` → **tự gộp** thành khối nằm;
  - 7 nước cuối lăn khối qua các cầu vào lỗ đích.
- Trong file lời giải, khi đang tách mỗi nước có dạng `<khối><hướng>` (`0U`, `1R`…); khối `0` là khối có
  tọa độ nhỏ hơn. Trong game bấm **Space** để đổi khối đang điều khiển.
- File: [code/data/16.data](../code/data/16.data), [code/data/solutions/16.txt](../code/data/solutions/16.txt).

---

## 3. Kiểm chứng

- `tests/test_bfs_solver.py`: BFS giải màn 15 đúng 37 bước, màn 16 đúng 49 bước (có nước điều khiển khối nhỏ).
- `tests/test_levels.py`: catalog khớp file màn, lời giải thắng đúng ở nước cuối, tối ưu theo BFS độc lập.
- Game C++: `./a.exe 15 --play data/solutions/15.txt` → `WON, Steps : 37`;
  `./a.exe 16 --play data/solutions/16.txt` → `WON, Steps : 49` (report4).
