# Giải thích và hướng dẫn cơ chế Split Switch (Dual Cubes Mechanics)

Tài liệu đi cùng nhánh `feat/dat/v2-mechanics`, giải thích chi tiết cơ chế tách khối thành hai cube 1×1×1 độc lập,
quy tắc di chuyển, tương tác vật lý của từng cube và thuật toán tự động gộp khối khi hai cube kề nhau.
Tài liệu này dùng để chuẩn bị review, làm việc với TV2 (dựng hình 3D trong C++) và viết mục "Formalization" trong `Report.pdf`.

---

## 1. Cơ chế Split Switch là gì?

Trong Bloxorz, ngoài khối nguyên 1×1×2 thông thường, một số màn chơi nâng cao có chứa **ô công tắc tách (Split Switch)**:
- Khi khối nguyên **đứng thẳng (`Orientation.Z`)** lên ô split switch, khối sẽ bị phân rã thành **hai khối lập phương 1×1×1 độc lập**.
- Hai cube được dịch chuyển tức thời (teleport) đến hai vị trí định trước trên bàn chơi.
- Người chơi (hoặc thuật toán giải) có thể điều khiển riêng lẻ từng cube để vượt qua các địa hình hiểm trở (như hành lang ô cam) hoặc dẫm công tắc mở đường từ xa.
- Khi hai cube di chuyển đến vị trí **cạnh nhau (4-kề)**, chúng sẽ **tự động hợp nhất (recombine)** thành khối nguyên 1×1×2 ban đầu.

---

## 2. Đặc tả định dạng `.data` cho Split Switch

Trong khối `ext 1` ở cuối file `.data`:
```text
ext 1
# split <x> <y> <x1> <y1> <x2> <y2>
split 2 2 1 1 3 3
```
- `(x, y)`: Tọa độ ô công tắc split switch trên lưới.
- `(x1, y1)` và `(x2, y2)`: Tọa độ teleport của Cube 0 và Cube 1 khi khối bị tách.

---

## 3. Mô hình trạng thái `State` & Không gian tìm kiếm

### 3.1. Cấu trúc `State`
Trong [ai/state.py](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/ai/state.py):
```python
@dataclass(frozen=True, slots=True)
class State:
    x: int
    y: int
    orientation: Orientation
    bridge_mask: int = 0
    cubes: tuple[tuple[int, int], tuple[int, int]] | None = None
```
- Khi `cubes is None`: Khối nguyên 1×1×2 (biểu diễn bởi `x, y, orientation`).
- Khi `cubes is not None`: Khối đang ở trạng thái tách (`is_split == True`).

### 3.2. Chuẩn hóa tọa độ (Canonicalization)
Nếu hai cube có tọa độ $(A, B)$ và $(B, A)$ được coi là hai trạng thái khác nhau, không gian trạng thái tìm kiếm sẽ bị nhân đôi một cách vô ích.
Do đó, trong phương thức `__post_init__` của `State`, tọa độ hai cube **luôn được sắp xếp tăng dần**:
$$(c_1, c_2) \text{ sao cho } c_1 \le c_2$$
Nhờ vậy:
$$\text{State}(cubes=((5, 3), (1, 1))) == \text{State}(cubes=((1, 1), (5, 3)))$$
Điều này bảo đảm các thuật toán như BFS hay A* tự động nhận diện chu trình, không bao giờ duyệt lặp trạng thái hoán vị của hai cube.

### 3.3. Không gian 8 hành động khi tách
Khi `state.is_split == True`, tập hành động là:
$$\text{SPLIT\_ACTIONS} = \{0U, 0D, 0L, 0R, 1U, 1D, 1L, 1R\}$$
- Ký tự đầu tiên (`0` hoặc `1`): Chỉ số của cube cần di chuyển theo thứ tự tọa độ đã chuẩn hóa.
- Ký tự thứ hai (`U`, `D`, `L`, `R`): Hướng di chuyển 1 ô của cube đó.

---

## 4. Quy tắc vật lý & Chuyển trạng thái (Transition Rules)

| Tình huống | Quy tắc đối với Cube 1×1×1 | Lý giải |
|---|---|---|
| **Đi vào ô cam (Fragile)** | **AN TOÀN** (không vỡ, không thua) | Áp lực của cube 1×1×1 lên mặt sàn là 1 đơn vị/ô, bằng áp lực của khối 1×1×2 khi nằm. Do đó ô cam không bị sụp. |
| **Đi vào ô đích (Goal)** | **AN TOÀN nhưng KHÔNG THẮNG** | Ô đích chịu lực như sàn thường. Khối tách không thể thắng màn (chỉ khối nguyên đứng thẳng mới thắng). |
| **Dẫm công tắc thường** | **KÍCH HOẠT** | Công tắc thường nhạy với mọi áp lực. |
| **Dẫm công tắc nặng / Split** | **KHÔNG KÍCH HOẠT** | Công tắc nặng và split đòi hỏi toàn bộ khối lượng của khối đứng thẳng (2 đơn vị/ô). |
| **Hai cube đè lên cùng 1 ô** | **KHÔNG HỢP LỆ** (`Outcome.LOST`) | Hai khối rắn không thể chiếm cùng một vị trí không gian. |
| **Đi ra ngoài biên / Cầu đóng** | **RƠI** (`Outcome.LOST`) | Rơi xuống vực. |

---

## 5. Cơ chế Tự động Gộp khối (Recombination)

Ngay sau khi một cube di chuyển, hệ thống tính khoảng cách Manhattan giữa hai cube:
$$d = |x_1 - x_2| + |y_1 - y_2|$$

- **Nếu $d = 1$ (hai cube 4-kề nhau):** Hai cube lập tức hợp nhất thành khối nguyên 1×1×2:
  - Nếu $|x_1 - x_2| = 1$ và $y_1 = y_2$: Hợp nhất thành khối **nằm dọc theo trục X** (`Orientation.X`) với ô neo tại $(\min(x_1, x_2), y_1)$.
  - Nếu $x_1 = x_2$ và $|y_1 - y_2| = 1$: Hợp nhất thành khối **nằm ngang theo trục Y** (`Orientation.Y`) với ô neo tại $(x_1, \min(y_1, y_2))$.
- Trạng thái `State` tự động chuyển về `cubes = None`.
- Khối vừa gộp nếu nằm đè lên công tắc thường sẽ kích hoạt công tắc theo đúng luật game.

---

## 6. Hướng dẫn phối hợp cho các thành viên

1. **Dành cho TV2 (C++ GUI & Autoplay):**
   - Logic tách/gộp trong [ai/problem.py](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/ai/problem.py) là chuẩn mực.
   - Khi phát lại lời giải của solver trên GUI: solver sinh các hành động `0U, 1R, ...`. Khi render, game chỉ cần chuyển cube tương ứng hoặc tự chèn phím Space để đổi cube active cho người chơi.
2. **Dành cho TV4 (Màn chơi G4):**
   - Bạn có thể thiết kế màn chơi nhóm **G4 (Split)**: màn có hành lang ô cam hẹp chỉ cube mới đi qua được, yêu cầu tách khối ở phòng A, điều khiển 1 cube sang phòng B dẫm switch mở cầu, sau đó hai cube gặp nhau ở phòng C để gộp lại trước khi về đích.
3. **Dành cho TV5 (Heuristic & Báo cáo):**
   - Lưu ý khi chứng minh admissibility của heuristic: Trên màn có split switch, khối có thể teleport qua khoảng cách lớn với chi phí 1 nước. Cần ghi chú điều này trong phần chứng minh của Báo cáo.

---

## 7. Kết quả kiểm thử

Chạy kiểm thử chuyên biệt cho split:
```bash
python -m pytest tests/test_split.py -v
python -m pytest -q
```
**Kết quả:**
- `tests/test_split.py`: **7 passed** (chuẩn hóa tọa độ, teleport, di chuyển, không vỡ ô cam, dẫm công tắc, gộp dọc trục X, gộp ngang trục Y, giải hoàn chỉnh puzzle tách-gộp).
- Toàn bộ repo: **100 passed, 14 skipped** (đạt mốc 100 test PASS, 0 hồi quy).
