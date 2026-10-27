# Giải thích và hướng dẫn cơ chế .data v2 (Multi-Bridge & Permanent Switches)

Tài liệu đi cùng nhánh `feat/dat/v2-mechanics`, dùng để giải thích cơ chế định dạng màn chơi v2,
cách hoạt động của công tắc điều khiển nhiều ô cầu, công tắc một chiều (`permanent`), và cách đồng bộ giữa Python và C++.
Tài liệu này phục vụ các thành viên trong nhóm hiểu rõ kiến trúc và dùng làm tư liệu cho mục "Luật chơi & Hình thức hóa" trong `Report.pdf`.

---

## 1. Vấn đề giải quyết

Ở định dạng v1 nguyên bản của game gốc:
- Mỗi dòng công tắc chỉ ánh xạ đúng **1 ô công tắc $\rightarrow$ 1 ô cầu duy nhất**.
- Mọi công tắc chỉ hoạt động theo kiểu **toggle** (dẫm lần 1 thì mở, dẫm lần 2 thì đóng).
- Chưa hỗ trợ cơ chế công tắc **permanent** (kích hoạt 1 lần là giữ nguyên vĩnh viễn) mà đề bài yêu cầu và có trong game Bloxorz gốc.

Giai đoạn 1 nâng cấp toàn bộ hệ thống lên **v2** mà vẫn giữ **tương thích ngược 100%** với toàn bộ màn v1 cũ.

---

## 2. Đặc tả định dạng `.data` v2

Phần v1 được giữ nguyên ở đầu file. Phần mở rộng v2 được đặt ở cuối file, bắt đầu bằng từ khóa `ext 1`:

```text
<n> <m>
<lưới n x m>
<sx> <sy> <along: X|Y|Z>
<tx> <ty>
<k>
<k dòng công tắc thường v1: x y a b>
<h>
<h dòng công tắc nặng v1: x y a b>

ext 1
# switch <kind: normal|heavy> <x> <y> <mode: toggle|permanent_on|permanent_off> <số_ô_cầu> <a1> <b1> <a2> <b2> ...
switch normal 1 1 toggle 2 1 2 1 3
switch heavy 4 5 permanent_on 3 6 1 6 2 6 3
```

- **Quy tắc ghi đè:** Nếu một công tắc tại tọa độ `(x, y)` đã khai báo ở phần v1, khai báo tại `ext 1` sẽ **ghi đè và nâng cấp** công tắc đó thành nhiều ô cầu hoặc chế độ permanent.
- **Tương thích ngược:** Các file v1 không có khối `ext` vẫn được parser Python và C++ đọc bình thường như trước.

---

## 3. Kiến trúc toán học Bitmask Parity

### 3.1. Đại số trạng thái cầu (`bridge_mask`)
Trong lớp `State(x, y, orientation, bridge_mask)`:
- Mỗi công tắc $S_i$ được gán một chỉ số bit duy nhất: $\text{bit}_i \in \{0, 1, 2, \dots\}$.
- Giá trị `bridge_mask` là một số nguyên đại diện cho vector nhị phân trạng thái kích hoạt của tất cả công tắc.

### 3.2. Nhiều ô cầu cho một công tắc
- Trong `Level.bridge_bits`:
  Với mỗi ô cầu $(a, b)$, mask các công tắc điều khiển nó là:
  $$\text{bits}[(a, b)] = \bigvee_{S_i \text{ điều khiển } (a, b)} (1 \ll \text{bit}_i)$$
- Khi kiểm tra ô $(x, y)$ có sàn hay không (`Level.has_tile(x, y, mask)`):
  $$\text{parity} = \text{popcount}(\text{mask} \ \& \ \text{bits}[(x, y)]) \pmod 2$$
  Nếu $\text{parity} = 1$, trạng thái mặt sàn của ô $(x, y)$ bị đảo ngược so với lưới ban đầu $\text{grid}[x][y]$.
  Nhờ tính chất tuyến tính của XOR parity, **nhiều công tắc khác nhau cùng điều khiển chung một ô cầu** sẽ tự động tương tác chính xác (XOR lẫn nhau) mà không cần code thêm trường hợp đặc biệt nào!

### 3.3. Cơ chế Permanent Switch
- **`toggle`**: 
  $$\text{mask}_{\text{mới}} = \text{mask} \oplus (1 \ll \text{bit}_i)$$
  Mỗi lần bước lên công tắc, bit trạng thái bị lật giữa 0 và 1.
- **`permanent_on` / `permanent_off`**:
  $$\text{mask}_{\text{mới}} = \text{mask} \lor (1 \ll \text{bit}_i)$$
  Toán tử OR bit bảo đảm rằng: sau lần dẫm đầu tiên, bit luôn bằng 1. Những lần dẫm tiếp theo $\text{mask} \lor (1 \ll \text{bit}_i)$ không làm thay đổi mask, do đó cầu giữ nguyên trạng thái vĩnh viễn!

---

## 4. Đồng bộ Python và C++

| Chức năng | Phía Python (`ai/`) | Phía C++ (`code/`) |
|---|---|---|
| Cấu trúc dữ liệu | `Switch(kind, pos, targets, mode, bit)` trong [ai/level.py](../ai/level.py) | `normal_bridges`, `heavy_bridges`, `switch_modes` trong [code/tile.h](../code/tile.h) |
| Đọc file `.data` | `Level.from_text()` phân tích token `ext 1` | `Floor::initialize()` phân tích token `ext 1` trong [code/tile.cpp](../code/tile.cpp) |
| Kích hoạt công tắc | `BloxorzProblem._activate_switch()` trong [ai/problem.py](../ai/problem.py) | `Floor::normal_bridge_toggle()` và `heavy_bridge_toggle()` |
| Kiểm tra sàn | `Level.has_tile()` dùng parity bitmask | `Floor::has_tile()` và `Floor::toggle()` cập nhật con trỏ `Tile*` |

---

## 5. Hướng dẫn cho các thành viên trong nhóm

1. **Dành cho TV4 (Thiết kế màn chơi):**
   - Bạn có thể bắt đầu tạo các màn chơi thuộc nhóm **G3 (Switch)** và **G5 (Combined)** có công tắc mở cùng lúc 2–4 ô cầu, hoặc công tắc một chiều `permanent_on`.
   - Cú pháp rất đơn giản: chỉ cần thêm dòng `ext 1` và `switch ...` vào cuối file `.data`.
2. **Dành cho TV2 (Giao diện 3D C++):**
   - C++ đã đọc và hỗ trợ đầy đủ `normal_bridges` và `heavy_bridges`. Hiệu ứng trồi sụt ô cầu (`appear_time`, `GhostTile`) tự động áp dụng cho tất cả các ô trong danh sách `targets`.
3. **Dành cho TV5 (Báo cáo & Thực nghiệm):**
   - Không gian trạng thái $S$ nay có thêm các trạng thái cầu phong phú hơn. Đây là luận điểm rất mạnh để trình bày trong mục "Formalization: State Space Representation" của báo cáo `Report.pdf`.

---

## 6. Kết quả kiểm thử

Chạy toàn bộ test suite từ thư mục gốc:
```bash
python -m pytest tests/test_v2_switches.py -v
python -m pytest -q
```
**Kết quả:**
- `tests/test_v2_switches.py`: **5 passed** (kiểm thử đầy đủ multi-bridge toggle, permanent on, permanent off, và tương tác parity nhiều công tắc).
- Toàn bộ repo: **93 passed, 14 skipped** (0 hồi quy, 100% tương thích ngược).
