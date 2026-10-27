# Hướng dẫn và giải thích bộ màn chơi v2 (Level 12 & 13)

Tài liệu đi cùng nhánh `feat/dat/v2-mechanics`, tổng kết việc bổ sung hai màn chơi v2 đầy thử thách
vào bộ dữ liệu chuẩn của dự án: **Màn 12** (Multi-bridge & Permanent Switch) và **Màn 13** (Split Switch & Recombination).

---

## 1. Tổng quan bổ sung

| Màn | Nhóm | Tên | Kích thước | Ô sàn | Dễ vỡ | Công tắc | Ô cầu | Độ dài tối ưu |
|:---:|:---:|---|:---:|---:|---:|:---:|:---:|:---:|
| **12** | G3 | Dual bridges crossing | 11×17 | 49 | 0 | 1 normal / 1 heavy | 3 | **12 bước** |
| **13** | G4 | The great split | 12×20 | 36 | 2 | 1 normal / 0 heavy | 1 | **21 bước** |

---

## 2. Chi tiết thiết kế từng màn

### 2.1. Màn 12 — Dual Bridges Crossing (G3)
- **Vấn đề câu đố:**
  - Khối xuất phát ở Đảo Tây $(5, 2)$.
  - Tại $(4, 4)$ có một công tắc thường điều khiển cùng lúc **2 ô cầu** $(5, 7)$ và $(5, 8)$ bắc sang Đảo Đông.
  - Trên Đảo Đông, tại $(4, 11)$ là một công tắc nặng **`permanent_on`** điều khiển cây cầu một chiều tại $(5, 13)$ dẫn vào lỗ đích $(5, 14)$.
  - Người chơi phải lật khối khéo léo để đứng thẳng $Z$ lên công tắc nặng này.
- **Lời giải tối ưu (12 bước):**
  ```text
  U L R U U U U L U U R U
  ```
- **File:** [code/data/12.data](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/code/data/12.data), [code/data/solutions/12.txt](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/code/data/solutions/12.txt).

---

### 2.2. Màn 13 — The Great Split (G4)
- **Vấn đề câu đố:**
  - Khối xuất phát ở Đảo Tây $(5, 2)$, lăn 2 bước $U$ để đứng thẳng $Z$ lên ô split switch tại $(5, 5)$.
  - Khối phân rã thành **Cube 0** tại $(3, 7)$ và **Cube 1** tại $(7, 7)$.
  - **Hành lang hiểm trở:** Cube 0 phải đi dọc hành lang phía Bắc chứa 2 ô cam dễ vỡ $(3, 9)$ và $(3, 10)$. Khối nguyên không thể đi qua đây mà không bị rơi, nhưng cube 1×1×1 đi qua hoàn toàn an toàn!
  - Tại cuối hành lang $(3, 12)$, Cube 0 dẫm lên công tắc thường để mở cây cầu tại $(7, 10)$ cho Cube 1 vượt hào nước.
  - Cả hai cube tiến vào Quảng trường Đông và hội ngộ tại vị trí kề nhau $\rightarrow$ **tự động gộp lại thành khối $1 \times 1 \times 2$**.
  - Khối sau khi gộp tiếp tục lăn 4 bước cuối cùng vào lỗ đích $(5, 16)$.
- **Lời giải tối ưu (21 bước):**
  ```text
  U U 0U 0U 0U 0U 0U 0R 0U 0R 1U 1U 1U 1U 1U 1L 1U L U R U
  ```
- **File:** [code/data/13.data](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/code/data/13.data), [code/data/solutions/13.txt](file:///d:/hcmus/HK5/csAI/CSAI_Group9_Lab01/code/data/solutions/13.txt).

---

## 3. Tiêu chuẩn kỹ thuật đã đạt được

1. **Tuân thủ quy chuẩn hình học:**
   - Cả 2 màn đều chừa đúng 2 ô viền rỗng (0) xung quanh để camera 3D không bị tràn viền.
   - Ô đích là lỗ rỗng (0) nằm bên trong nội viễn bàn chơi.
2. **Kiểm thử tự động:**
   - `tests/test_levels.py`: **43 passed**.
   - Toàn bộ repo: **106 passed, 14 skipped**.
3. **Ảnh sơ đồ & Catalog:**
   - Đã sinh ảnh preview sắc nét: `code/data/previews/12.png`, `13.png` và cập nhật `overview.png`.
   - Cập nhật đầy đủ `catalog.json` và `code/data/README.md`.
