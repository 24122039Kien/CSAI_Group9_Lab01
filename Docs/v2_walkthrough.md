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

## 3. Biểu diễn trạng thái cầu (`bridge_mask`)

> **Cập nhật khi merge vào `main` (report4):** bản đầu dùng 1 bit cho mỗi **công tắc** (chẵn lẻ số lần
> kích hoạt) và cho `permanent_off` cũng `mask | bit` như `permanent_on`. Cách đó **mở** cầu khi bấm
> công tắc "off", lệch với C++ (đã đối chiếu bằng `./a.exe --play`: Python thắng, C++ rơi). Nay đổi
> sang 1 bit cho mỗi **ô cầu**, đúng như đề yêu cầu "state must include the open/closed status of
> every bridge" và giống cách C++ lưu ô thật.

### 3.1. Mỗi ô cầu một bit
- `Level.bridge_bits[(a, b)]` = bit riêng của ô cầu $(a, b)$, đánh số theo thứ tự xuất hiện lần đầu.
- Bit bật = ô cầu đang **mở**. Ô cầu lúc đầu luôn là 0 trên lưới (đóng), nên `initial_state()` có `bridge_mask = 0`.
- `Level.has_tile(x, y, mask)` với ô cầu: có sàn khi `mask & bridge_bits[(x, y)]` khác 0.
- Mỗi công tắc có `switch.mask` = OR các bit ô cầu nó điều khiển (nhiều ô cầu → nhiều bit).

### 3.2. Ba chế độ công tắc (`BloxorzProblem._activate_switch`)
| Chế độ | Phép toán | Ý nghĩa, giống `Floor::*_bridge_toggle` |
|---|---|---|
| `toggle` | `mask ^ switch.mask` | đảo từng ô cầu |
| `permanent_on` | `mask \| switch.mask` | tạo ô đang trống, ô đang có giữ nguyên |
| `permanent_off` | `mask & ~switch.mask` | xóa ô đang có, ô đang trống giữ nguyên |

Nhiều công tắc cùng điều khiển một ô cầu tương tác đúng: chúng cùng tác động lên **một** bit của ô đó.

---

## 4. Đồng bộ Python và C++

| Chức năng | Phía Python (`ai/`) | Phía C++ (`code/`) |
|---|---|---|
| Cấu trúc dữ liệu | `Switch(kind, pos, targets, mode, bit)` trong [ai/level.py](../ai/level.py) | `normal_bridges`, `heavy_bridges`, `switch_modes` trong [code/tile.h](../code/tile.h) |
| Đọc file `.data` | `Level.from_text()` phân tích token `ext 1` | `Floor::initialize()` phân tích token `ext 1` trong [code/tile.cpp](../code/tile.cpp) |
| Kích hoạt công tắc | `BloxorzProblem._activate_switch()` trong [ai/problem.py](../ai/problem.py) | `Floor::normal_bridge_toggle()` và `heavy_bridge_toggle()` |
| Kiểm tra sàn | `Level.has_tile()` đọc bit của ô cầu | `Floor::has_tile()` và `Floor::toggle()` cập nhật con trỏ `Tile*` |

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
- `tests/test_v2_switches.py`: **5 passed** (kiểm thử đầy đủ multi-bridge toggle, permanent on, permanent off, và nhiều công tắc cùng một ô cầu).
- Toàn bộ repo: **93 passed, 14 skipped** (0 hồi quy, 100% tương thích ngược).
