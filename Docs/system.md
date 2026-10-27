# System — Hiểu toàn bộ đồ án Bloxorz Solver từ con số 0 (nhóm 9)

> **Tài liệu này dành cho ai?** Cho cả 5 thành viên, kể cả người chưa từng học thuật toán tìm kiếm. Đề có ghi giảng viên được **vấn đáp ngẫu nhiên** bất kỳ ai (mục 5 của `AI - Project 1.pdf`), nên ai cũng cần hiểu cả hệ thống, không chỉ phần mình code.
>
> **Đọc thế nào?** Đọc **theo thứ tự**. Mỗi phần chỉ dùng những gì đã giải thích ở phần trước. Phần 1–4 không cần biết code. Phần 5 đi qua code. Phần 6–10 giải thích 4 thuật toán bằng cách **chạy tay trên cùng một màn chơi nhỏ**, nhờ vậy bạn thấy rõ chúng khác nhau ở đâu.
>
> **Nguồn:** đề `Docs/AI - Project 1.pdf` và code trên nhánh `main` (sau khi đã merge bộ màn v1 và A\*). Các bảng "chạy tay" được lấy từ mô phỏng thật bằng `ai/problem.py`, không phải tự nghĩ ra.
>
> **Lưu ý:** BFS, DFS, UCS và `manhattan_heuristic` vẫn là phần các thành viên **tự cài** (sẽ bị vấn đáp), nên tài liệu chỉ mô tả cách chúng chạy bằng lời và mã giả, không có code Python để chép.

---

## Mục lục

- [Phần 0. Tóm tắt 1 phút + bảng thuật ngữ](#phần-0-tóm-tắt-1-phút--bảng-thuật-ngữ)
- [Phần 1. Đề bài yêu cầu gì?](#phần-1-đề-bài-yêu-cầu-gì)
- [Phần 2. Hệ thống gồm những gì? (bức tranh lớn)](#phần-2-hệ-thống-gồm-những-gì-bức-tranh-lớn)
- [Phần 3. Hiểu game qua hình vẽ](#phần-3-hiểu-game-qua-hình-vẽ)
- [Phần 4. Biến game thành "bài toán tìm đường"](#phần-4-biến-game-thành-bài-toán-tìm-đường)
- [Phần 5. Đi dạo qua code](#phần-5-đi-dạo-qua-code)
- [Phần 6. Khung chung của mọi thuật toán tìm kiếm](#phần-6-khung-chung-của-mọi-thuật-toán-tìm-kiếm)
- [Phần 7. BFS — loang theo từng lớp](#phần-7-bfs--loang-theo-từng-lớp)
- [Phần 8. DFS — đi sâu tới cùng rồi quay lui](#phần-8-dfs--đi-sâu-tới-cùng-rồi-quay-lui)
- [Phần 9. UCS — luôn đi tiếp từ chỗ "rẻ nhất"](#phần-9-ucs--luôn-đi-tiếp-từ-chỗ-rẻ-nhất)
- [Phần 10. A\* — rẻ nhất + biết hướng về đích](#phần-10-a--rẻ-nhất--biết-hướng-về-đích)
- [Phần 11. So sánh 4 thuật toán](#phần-11-so-sánh-4-thuật-toán)
- [Phần 12. Đo đạc cho phần thực nghiệm](#phần-12-đo-đạc-cho-phần-thực-nghiệm)
- [Phần 13. Còn thiếu gì so với đề?](#phần-13-còn-thiếu-gì-so-với-đề)
- [Phần 14. Câu hỏi vấn đáp thường gặp](#phần-14-câu-hỏi-vấn-đáp-thường-gặp)

---

## Phần 0. Tóm tắt 1 phút + bảng thuật ngữ

**Tóm tắt 1 phút:**

1. Game **Bloxorz**: lăn một khối hộp dài 2 ô trên mặt sàn, mục tiêu là cho khối **đứng thẳng** lọt xuống **lỗ đích**.
2. Nhóm có hai phần chương trình:
   - Game 3D viết bằng **C++** (thư mục `code/`): để chơi và nhìn.
   - "Bộ não" viết bằng **Python** (thư mục `ai/`): để máy tự tìm cách thắng.
3. Bấm nút "BFS / DFS / UCS / A\*" trong game → game gọi Python → Python tìm ra chuỗi nước đi như `R U U R` → game tự lăn khối theo chuỗi đó.
4. Bốn thuật toán khác nhau ở **cách chọn chỗ nào để thử tiếp**. Đó là toàn bộ bí mật của chúng.

**Bảng thuật ngữ** (quay lại tra khi gặp từ lạ):

| Thuật ngữ | Nghĩa dễ hiểu |
|---|---|
| **Trạng thái** (state) | Một "ảnh chụp" đầy đủ của ván chơi tại một thời điểm: khối ở đâu, đang đứng hay nằm, cây cầu nào đang mở |
| **Hành động** (action) | Một nước đi: `U`, `D`, `L` hoặc `R` |
| **Nước đi hợp lệ** | Nước đi không làm khối rơi |
| **Lời giải** (solution) | Chuỗi nước đi từ lúc bắt đầu tới lúc thắng, ví dụ `R U U R U R U R D L` |
| **Node** | Trong tài liệu này, hiểu đơn giản là "một trạng thái mà thuật toán đang xét" |
| **Sinh** (generate) | Từ một trạng thái, thử cả 4 hướng để biết đi được tới những trạng thái nào |
| **Mở rộng** (expand) | Lấy một trạng thái ra để **sinh** các trạng thái con của nó. Đề yêu cầu đếm số lần này ("Expanded Nodes") |
| **Frontier** | "Danh sách chờ": các trạng thái đã biết tới nhưng chưa mở rộng |
| **Visited / parent** | "Sổ ghi chép": các trạng thái đã gặp, kèm ghi chú "tới đây từ đâu, bằng nước nào" để truy ngược lời giải |
| **Chi phí** (cost) | "Giá" của một nước đi. Mặc định mỗi nước tốn 1 |
| **g** | Tổng chi phí đã bỏ ra để đi từ đầu tới trạng thái hiện tại |
| **h** (heuristic) | **Ước đoán** chi phí còn lại để tới đích |
| **f = g + h** | Ước đoán tổng chi phí của cả hành trình nếu đi qua trạng thái này (A\* dùng) |
| **Tối ưu** (optimal) | Tìm ra lời giải tốt nhất: ngắn nhất, hoặc rẻ nhất |
| **Đầy đủ** (complete) | Nếu có lời giải thì chắc chắn sẽ tìm ra |
| **Admissible** | Heuristic "không bao giờ nói quá": luôn ước đoán **nhỏ hơn hoặc bằng** chi phí thật |
| **Consistent** | Heuristic "không tụt đột ngột": đi một nước thì ước đoán giảm không quá giá của nước đó |
| **Heap / hàng đợi ưu tiên** | Cấu trúc dữ liệu luôn lấy ra được phần tử **nhỏ nhất** thật nhanh |
| **Bộ nhớ đỉnh** (peak memory) | Lúc thuật toán chiếm nhiều bộ nhớ nhất là bao nhiêu |

---

## Phần 1. Đề bài yêu cầu gì?

Đề `AI - Project 1.pdf` có 5 việc chính:

| Việc | Điểm | Nói đơn giản |
|---|---:|---|
| **Làm lại game** (mục 3.1) | 20 | 10đ luật cơ bản (sàn, ô trống, lỗ đích, khối 3 tư thế, lăn 4 hướng, rơi thì thua, đứng trên lỗ thì thắng) + 10đ ô đặc biệt (ô cam dễ vỡ, cầu, 3 loại công tắc). Có giao diện, có nút New Game, Restart và nút gọi từng thuật toán |
| **BFS** (mục 3.2) | 10 | Tự cài. Ghi lại 4 con số: thời gian, bộ nhớ, số node mở rộng, độ dài lời giải |
| **DFS** (mục 3.3) | 10 | Tự cài. Phải chống đi lặp vòng tròn |
| **UCS** (mục 3.4) | 10 | Tự cài. **Phải tự đặt "giá" cho các nước đi và giải thích vì sao**. Nếu mọi nước giá 1 thì UCS y hệt BFS, nên phải cho các nước giá khác nhau |
| **A\*** (mục 3.5) | 10 | Tự cài. Phải có heuristic và chứng minh nó **không bao giờ đoán quá** |
| **Báo cáo** (mục 3.6) | 30 | Giải thích thuật toán (10) + **thực nghiệm**: chạy nhiều màn, vẽ bảng, biểu đồ, nhận xét (20) |
| **Video** (mục 3.7) | 10 | Quay demo giao diện, chơi tay, chạy thuật toán |

---

## Phần 2. Hệ thống gồm những gì? (bức tranh lớn)

### 2.1. Hai nửa: "tay chân + mắt" và "bộ não"

Theo buổi Q&A với giảng viên (ghi trong `Docs/report0.md`): giao diện phải là **3D** và được viết bằng ngôn ngữ khác, nhưng **thuật toán phải bằng Python**. Vì vậy hệ thống chia làm hai:

| Nửa | Thư mục | Ví von | Làm gì |
|---|---|---|---|
| Game C++ | `code/` | **Tay chân + mắt** | Vẽ 3D, nhận phím, cho khối lăn, hiện menu, tự lăn theo lời giải |
| AI Python | `ai/`, `main.py` | **Bộ não** | Đọc màn chơi, mô phỏng luật, chạy thuật toán tìm lời giải |

Hai nửa **không gọi hàm của nhau**. Chúng nói chuyện qua **dòng lệnh và một file chữ**, giống hai người nhắn tin.

### 2.2. Chuyện gì xảy ra khi bấm nút "A\*"?

```
 Người chơi bấm nút "A*" trong menu
        │
        ▼
 ① Game C++ chạy lệnh:   python ../main.py data/1.data --algo astar --out solution.txt
        │                (chạy ở luồng phụ, nên cửa sổ game không bị đứng)
        ▼
 ② Python đọc file màn chơi data/1.data
 ③ Python chạy A* → tìm được chuỗi nước đi
 ④ Python tự chơi thử chuỗi đó theo luật, đúng thì ghi "R U U R ..." vào solution.txt
        │
        ▼
 ⑤ Game C++ đọc solution.txt
 ⑥ Game chơi lại màn từ đầu, cứ khối đứng yên 0.25 giây thì "bấm" giùm nước tiếp theo
 ⑦ Khối lọt lỗ → hiện "LEVEL COMPLETE"
```

### 2.3. Điểm nguy hiểm nhất: luật nằm ở HAI chỗ

Game C++ có luật của nó (để chơi). Python cũng có luật của nó (để tìm kiếm). **Hai bản luật phải giống hệt nhau.** Nếu lệch, Python sẽ tìm ra lời giải "thắng trong đầu Python" nhưng khối lại rơi trong game.

Quy tắc của nhóm: sửa luật thì sửa **cả hai bên**, thêm test vào `tests/test_problem.py`, và cho game tự chơi lại lời giải bằng lệnh `--play` để kiểm tra. Hiện cả 10 màn có lời giải đều đã được kiểm tra: lời giải của Python thắng đúng số bước trong game C++.

---

## Phần 3. Hiểu game qua hình vẽ

Phần này là **nền móng**. Phần lớn lỗi khi code solver đến từ việc hiểu sai phần này.

### 3.1. Mặt sàn và ký hiệu dùng trong tài liệu

Màn chơi là một lưới ô vuông. Trong tài liệu này ta vẽ bằng các ký hiệu:

| Ký hiệu | Loại ô | Số trong file `.data` |
|---|---|---:|
| `·` | Ô trống (không có sàn, đi vào là rơi) | 0 |
| `□` | Sàn thường | 1 |
| `▒` | Ô cam dễ vỡ | 2 |
| `○` | Công tắc thường (soft switch) | 3 |
| `X` | Công tắc nặng (heavy switch) | 4 |
| `◎` | Lỗ đích (trong file cũng ghi 0) | 0 |
| `■` | Khối của người chơi | — |

### 3.2. Tọa độ: chỗ dễ nhầm nhất

Code dùng **`x` = số dòng, `y` = số cột** (giống game C++ gốc). Khi nhìn bản đồ dạng chữ (như file `.data`), các hướng đi là:

```
                L  (x − 1: đi LÊN một dòng)
                ▲
   D (y − 1) ◄──┼──► U (y + 1)
  (sang TRÁI)   │   (sang PHẢI)
                ▼
                R  (x + 1: đi XUỐNG một dòng)
```

> ⚠️ Trong cửa sổ game 3D, camera được xoay nên `R` trông như sang phải. Nhưng **trên bản đồ chữ, `R` là đi xuống, `U` là sang phải.** Khi đọc code, cứ nhớ: **R/L đổi x (dòng), U/D đổi y (cột).**

### 3.3. Ba tư thế của khối

Khối dài 2 ô nên luôn ở một trong ba tư thế. Code đặt tên `Z`, `X`, `Y`, và luôn lấy **ô có tọa độ nhỏ nhất** làm "ô neo" `(x, y)`:

```
  Z = ĐỨNG              X = NẰM theo dòng         Y = NẰM theo cột
  chiếm 1 ô (x,y)       chiếm (x,y) và (x+1,y)    chiếm (x,y) và (x,y+1)

   · · ·                 · · ·                     · · · ·
   · ■ ·                 · ■ ·    ← (x, y)         · ■ ■ ·
   · · ·                 · ■ ·    ← (x+1, y)       · · · ·
                                                     ↑ ↑
                                                (x,y) (x,y+1)
```

Vì luôn lấy ô nhỏ nhất làm neo, mỗi vị trí của khối chỉ có **đúng một cách ghi**. Nhờ vậy không bị đếm trùng.

### 3.4. Lăn khối: chỉ có 3 kiểu

Đề (mục 2.3) mô tả 3 kiểu lăn. Lấy khối ở ô `(4,4)` làm ví dụ:

**Kiểu 1: đang ĐỨNG → ĐỔ XUỐNG**, nằm trên 2 ô theo hướng lăn.

```
  Z(4,4) lăn R:                     kết quả X(5,4)
   · ■ ·   ← dòng 4                   · · ·   ← dòng 4
   · · ·   ← dòng 5     ──R──►        · ■ ·   ← dòng 5
   · · ·   ← dòng 6                   · ■ ·   ← dòng 6
```

**Kiểu 2: đang NẰM DỌC theo hướng lăn → DỰNG ĐỨNG** lên một ô.

```
  X(4,4) lăn R:                     kết quả Z(6,4)
   · ■ ·   ← dòng 4                   · · ·
   · ■ ·   ← dòng 5     ──R──►        · · ·
   · · ·   ← dòng 6                   · ■ ·   ← dòng 6
```

**Kiểu 3: đang NẰM NGANG so với hướng lăn → LĂN NGANG**, vẫn nằm, dịch 1 ô.

```
  X(4,4) lăn U (sang phải):         kết quả X(4,5)
   · ■ · ·                            · · ■ ·
   · ■ · ·              ──U──►        · · ■ ·
```

Gom lại thành **bảng chuyển trạng thái** (đề mục 3.1 dặn phải cài và test bảng này thật sớm). Trong code nó là `TRANSITIONS` ở `ai/problem.py`:

| Đang ở | lăn L | lăn R | lăn U | lăn D |
|---|---|---|---|---|
| `Z (x,y)` đứng | `X (x−2, y)` | `X (x+1, y)` | `Y (x, y+1)` | `Y (x, y−2)` |
| `X (x,y)` nằm theo dòng | `Z (x−1, y)` | `Z (x+2, y)` | `X (x, y+1)` | `X (x, y−1)` |
| `Y (x,y)` nằm theo cột | `Y (x−1, y)` | `Y (x+1, y)` | `Z (x, y+2)` | `Z (x, y−1)` |

Hai sự thật nhỏ về bảng này sẽ rất quan trọng về sau:

1. **Sau mỗi lần lăn, khối rời khỏi hoàn toàn các ô cũ.** Không có ô nào vừa thuộc vị trí cũ vừa thuộc vị trí mới.
2. **Mỗi lần lăn, khối "tiến" được nhiều nhất 2 ô.** Ô mới xa nhất chỉ cách ô cũ gần nhất 2 bước. Đây là nền tảng của heuristic A\* ở Phần 10.

### 3.5. Ô đặc biệt

- **Ô cam `▒`:** chịu được khối **nằm** (trọng lượng chia ra 2 ô), nhưng khối **đứng** lên là vỡ → rơi → thua.
- **Lỗ đích `◎`:** chỉ **thắng** khi khối **đứng** trên nó. Khối nằm vắt ngang qua lỗ thì **không thắng cũng không thua**: lỗ đích lúc đó coi như sàn.

### 3.6. Công tắc và cầu: giống công tắc đèn

**Cây cầu** là một ô lúc đầu trống (số 0 trong file). Mỗi công tắc điều khiển một hoặc nhiều ô cầu. Cứ **kích hoạt công tắc một lần** thì cầu **đổi trạng thái**: đang trống thì hiện ra thành sàn, đang là sàn thì biến mất. Giống bật/tắt đèn.

- **Công tắc thường `○`:** khối đè lên bằng **bất kỳ phần nào** (đứng hay nằm) là kích hoạt.
- **Công tắc nặng `X`:** chỉ kích hoạt khi khối **đứng** trên nó. Nằm đè lên không có tác dụng.
- **Chế độ** (định dạng v2, màn 15–16): `toggle` đảo cầu như trên; `permanent_on` chỉ **mở** (cầu đang mở giữ nguyên); `permanent_off` chỉ **đóng**.

**Trong code, cầu được nhớ bằng `bridge_mask`.** Hãy tưởng tượng một hàng bóng đèn, **mỗi bóng ứng với một ô cầu**, bóng sáng = ô cầu đang mở:

```
  ô cầu số:         2   1   0
  bridge_mask = 5 = 1   0   1   (viết dạng nhị phân)
                    ↑   ↑   ↑
                   mở  đóng mở   → ô cầu 0 và 2 đang mở, ô cầu 1 đang đóng
```

Mỗi công tắc biết các bóng của mình (`switch.mask`). Kích hoạt công tắc: `toggle` = đảo các bóng đó (`mask ^ switch.mask`), `permanent_on` = bật (`mask | switch.mask`), `permanent_off` = tắt (`mask & ~switch.mask`).

> ⚠️ **Vì sao `bridge_mask` phải là một phần của trạng thái?** Đề (mục 3.1) nhấn mạnh điều này. Hai ván có khối ở **cùng chỗ** nhưng cầu **khác nhau** thì các nước đi được cũng khác nhau, nên phải coi là **hai trạng thái khác nhau**. Nếu quên, thuật toán sẽ nghĩ "chỗ này đến rồi" và bỏ qua những lời giải kiểu "đi bấm công tắc, quay lại chỗ cũ, giờ cầu đã mở".

### 3.7. Một nước đi được xử lý theo thứ tự nào?

Python phải làm **đúng thứ tự** như game C++, nếu không hai bên sẽ lệch:

```
  ① Lăn khối theo bảng chuyển
  ② Kiểm tra:  có ô nào của khối không có sàn (và không phải lỗ đích)?  → THUA
               đứng trên ô cam?                                         → THUA
               đứng trên lỗ đích?                                       → THẮNG
               (thua được ưu tiên hơn thắng)
  ③ Nếu vừa thắng hoặc thua → DỪNG, không đụng tới công tắc
  ④ Nếu chưa → kích hoạt các công tắc khối đang đè (theo đúng luật thường/nặng)
  ⑤ Kiểm tra lại: lỡ công tắc vừa làm biến mất chính cây cầu dưới chân khối → THUA
```

### 3.8. File màn chơi `.data`

Mỗi màn là một file chữ trong `code/data/`. Cả game C++ và Python đều đọc file này, nên mọi thuật toán giải **cùng một màn** (đề mục 3.1 yêu cầu vậy). Ví dụ `code/data/1.data` có chú thích:

```
13 13                           ← 13 dòng, 13 cột
0 0 0 0 0 0 0 0 0 0 0 0 0       ← dòng x = 0
0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 1 1 1 3 0 0 0 0 0 0 0       ← (2,5) là công tắc thường
0 0 1 1 1 1 0 0 0 0 0 0 0
0 0 1 1 1 1 0 0 0 0 0 0 0
0 0 1 1 1 1 0 1 1 1 0 0 0       ← (5,6) là ô cầu (lúc đầu trống)
0 0 0 0 0 1 1 1 1 1 1 0 0       ← (6,4) là ô cầu
0 0 0 0 0 1 1 0 1 1 1 0 0       ← (7,7) là lỗ đích
0 0 0 0 0 1 1 1 1 1 1 0 0
0 0 0 0 0 1 1 1 2 1 1 0 0       ← (9,8) là ô cam
0 0 0 0 0 0 4 1 1 1 3 0 0       ← (10,6) công tắc nặng, (10,10) công tắc thường
0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0
3 3 Z                           ← xuất phát ở (3,3), tư thế đứng
7 7                             ← lỗ đích ở (7,7)
2                               ← có 2 công tắc thường:
2 5 6 4                         ←   công tắc ở (2,5) bật/tắt cầu (6,4)
10 10 9 11                      ←   công tắc ở (10,10) bật/tắt cầu (9,11)
1                               ← có 1 công tắc nặng:
10 6 5 6                        ←   công tắc ở (10,6) bật/tắt cầu (5,6)
```

Hiện có **11 màn** (`1.data` … `11.data`). Danh mục, ảnh sơ đồ và lời giải tối ưu nằm trong `code/data/README.md`, `code/data/catalog.json`, `code/data/previews/`, `code/data/solutions/`. Màn 11 được thiết kế **vô nghiệm**, để kiểm tra thuật toán biết dừng và báo "không có lời giải".

---

## Phần 4. Biến game thành "bài toán tìm đường"

### 4.1. Ý tưởng: giống tìm đường trên bản đồ

Hãy tưởng tượng mỗi **trạng thái** là một **thành phố**, mỗi **nước đi** là một **con đường** nối hai thành phố. Giải Bloxorz chính là **tìm đường từ thành phố "xuất phát" tới thành phố "thắng"**. Bản đồ đó gọi là **đồ thị trạng thái**.

| Bản đồ | Bloxorz | Trong code |
|---|---|---|
| Thành phố | Trạng thái `(x, y, tư thế, bridge_mask)` | `State` |
| Thành phố xuất phát | Vị trí đầu, mọi cầu đóng | `problem.initial_state()` |
| Các con đường từ một thành phố | Các nước đi hợp lệ (không rơi) | `problem.successors(state)` |
| Tới thành phố nào khi đi đường đó | Kết quả lăn + công tắc | `problem.simulate(state, action)` |
| Thành phố đích | Khối đứng trên lỗ đích | `problem.is_goal(state)` |
| Độ dài / phí của con đường | Chi phí một nước (mặc định 1) | `cost_fn` |

Điều khác với bản đồ thật: ta **không có sẵn bản đồ**. Thuật toán phải **tự khám phá** từng thành phố bằng cách đứng ở một trạng thái và thử 4 hướng.

### 4.2. Màn đồ chơi dùng xuyên suốt tài liệu

Để chạy tay được, ta dùng một màn **rất nhỏ** (3 dòng × 4 cột):

```
          y=0  y=1  y=2  y=3
  x=0      □    □    □    □
  x=1      □    ▒    ▒    □        ▒ = ô cam
  x=2      ■    □    ◎    □        ■ = khối đứng ở (2,0)    ◎ = lỗ đích (2,2)
```

Màn này có **27 trạng thái** đi tới được. Ta đặt tên chúng bằng chữ cái A, B, C… (theo thứ tự BFS gặp). Đây là những trạng thái sẽ xuất hiện nhiều:

| Tên | Trạng thái | Hình | Đi được tới (nước → trạng thái) |
|---|---|---|---|
| **A** | `(2,0,Z)` đứng | `■` ở (2,0) | U→B, L→C |
| **B** | `(2,1,Y)` nằm | `■■` ở (2,1),(2,2) — vắt qua lỗ đích | U→D, D→A, L→E |
| **C** | `(0,0,X)` nằm | `■` ở (0,0) và (1,0) | U→F, R→A |
| **D** | `(2,3,Z)` đứng | `■` ở (2,3) | D→B, L→G |
| **E** | `(1,1,Y)` nằm | trên 2 ô cam (1,1),(1,2) | U→H, D→I, L→J, R→B |
| **F** | `(0,1,X)` nằm | (0,1) và (1,1) cam | U→K, D→C, R→L |
| **G** | `(0,3,X)` nằm | (0,3) và (1,3) | D→K, R→D |
| **H** | `(1,3,Z)` đứng | (1,3) | D→E |
| **I** | `(1,0,Z)` đứng | (1,0) | U→E |
| **J** | `(0,1,Y)` nằm | (0,1),(0,2) | U→M, D→N, R→E |
| **K** | `(0,2,X)` nằm | (0,2) và (1,2) cam | U→G, D→F, **R→O** |
| **L** | `(2,1,Z)` đứng | (2,1) | U→P, L→F |
| **O** | `(2,2,Z)` | **đứng trên lỗ đích = THẮNG** | — |

Hai lời giải đáng chú ý (bạn có thể tự lăn bằng bảng 3.4 để kiểm tra):

```
Lời giải 1 (4 nước):  A ─L→ C ─U→ F ─U→ K ─R→ O       chạm ô cam 2 lần (ở F và K)
Lời giải 2 (5 nước):  A ─U→ B ─U→ D ─L→ G ─D→ K ─R→ O  chạm ô cam 1 lần (ở K)
```

Lời giải 1 vẽ từng bước:

```
   A (bắt đầu)       C (sau L)         F (sau U)         K (sau U)         O (sau R)
   □ □ □ □           ■ □ □ □           □ ■ □ □           □ □ ■ □           □ □ □ □
   □ ▒ ▒ □    L→     ■ ▒ ▒ □    U→     □ ■ ▒ □    U→     □ ▒ ■ □    R→     □ ▒ ▒ □
   ■ □ ◎ □           □ □ ◎ □           □ □ ◎ □           □ □ ◎ □           □ □ ■ □  ← THẮNG
```

Ở các phần sau, cả 4 thuật toán sẽ chạy trên **đúng màn này**, để bạn thấy mỗi thuật toán chọn lời giải nào và tốn bao nhiêu công.

---

## Phần 5. Đi dạo qua code

### 5.1. Nửa Python: "bộ não" (`ai/`, `main.py`, `tests/`)

Đọc theo thứ tự này là dễ hiểu nhất:

#### `ai/state.py`: một "ảnh chụp" ván chơi

- `State(x, y, orientation, bridge_mask)`: đúng 4 thông tin của một trạng thái (Phần 3).
- `State.cells()`: trả về 1 hoặc 2 ô khối đang chiếm.
- `Orientation`: ba tư thế `X`, `Y`, `Z`.
- `ACTIONS = ("U", "D", "L", "R")`: thứ tự thử các hướng **luôn cố định**, nên chạy lại bao nhiêu lần cũng ra cùng kết quả.

`State` được khai báo `frozen=True` (không sửa được sau khi tạo). Nhờ vậy Python cho phép dùng nó làm **khóa của `dict`/`set`**. Đây chính là cách làm "sổ ghi chép đã đi qua" mà đề yêu cầu ("visited set or hashing").

#### `ai/level.py`: đọc và hiểu file màn chơi

- `Level.from_file("code/data/1.data")`: đọc file thành đối tượng `Level` (lưới, xuất phát, đích, công tắc).
- `Level.validate()`: bắt lỗi file màn sai (đích không phải 0, công tắc trỏ ra ngoài…).
- `Level.has_tile(x, y, mask)`: **hàm quan trọng nhất của file.** Trả lời câu hỏi "ô `(x, y)` có sàn không, khi các cầu đang như `mask`?". Ô ngoài lưới thì luôn là "không có sàn".
- `Level.tile_type(x, y, mask)`: ô đó là loại gì lúc này.

#### `ai/problem.py`: luật chơi bản Python

Đây là file **mọi thuật toán đều dùng**. Các hàm thuật toán hay gọi:

| Hàm | Trả lời câu hỏi |
|---|---|
| `initial_state()` | Bắt đầu ở trạng thái nào? |
| `successors(state)` | **Từ đây đi được tới đâu?** Trả về danh sách `(nước đi, trạng thái mới, chi phí)`, đã tự bỏ các nước làm rơi |
| `is_goal(state)` | Trạng thái này đã thắng chưa? |
| `simulate(state, action)` | Đi thử một nước, theo đúng 5 bước ở mục 3.7 |
| `is_valid_solution(moves)` | Chuỗi nước này có thắng đúng ở nước cuối không? (`main.py` dùng để kiểm tra lại lời giải) |
| `path_cost(moves)` | Tổng chi phí của một chuỗi nước |

Ngoài ra: `TRANSITIONS` (bảng chuyển 3.4), `check()` (thắng/thua), `unit_cost` (mọi nước giá 1). `BloxorzProblem(level, cost_fn=...)` cho phép **cắm hàm chi phí khác** vào, phục vụ UCS (Phần 9).

#### `ai/heuristics.py`: các hàm "ước đoán" cho A\*

- `zero_heuristic`: luôn đoán 0. A\* với heuristic này chạy y hệt UCS (dùng để đối chứng).
- `manhattan_heuristic`: **TODO của TV5** (Phần 10).
- `HEURISTICS`: bảng tên → hàm, để `main.py --heuristic zero|manhattan` chọn.

#### `ai/solvers/`: các thuật toán

| File | Trạng thái | Người làm |
|---|---|---|
| `base_solver.py` | ✅ Khung chung: bấm giờ, đo bộ nhớ, đóng gói kết quả, truy ngược lời giải | (có sẵn) |
| `bfs_solver.py` | ⏳ TODO | TV1 |
| `dfs_solver.py` | ⏳ TODO | TV2 |
| `ucs_solver.py` | ⏳ TODO | TV3 |
| `astar_solver.py` | ✅ Đã cài (Sang), giải thích chi tiết ở `Docs/astar_walkthrough.md` | TV4 |
| `__init__.py` | Bảng `SOLVERS = {"bfs": ..., "dfs": ..., "ucs": ..., "astar": ...}` | |

Trong `base_solver.py` có hai thứ mọi người cần biết:

- **`SearchResult`**: kết quả một lần chạy, chứa đủ **4 con số đề yêu cầu**: `time_ms` (thời gian), `peak_memory_kb` (bộ nhớ đỉnh), `nodes_expanded` (số node mở rộng), `solution_length` (độ dài lời giải). Thêm `path`, `cost`, `nodes_generated`, `max_frontier`, `status`.
- **`BaseSolver`**: lớp cha. Mỗi thuật toán chỉ cần viết hàm `solve()`. Hàm `run()` lo bấm giờ và đo bộ nhớ. Hàm `reconstruct_path()` lo truy ngược lời giải từ "sổ ghi chép" `parent`.

#### `main.py`: cửa ngõ dòng lệnh

```bash
python main.py code/data/1.data --algo bfs                  # chạy 1 thuật toán
python main.py code/data/1.data --algo astar --heuristic zero
python main.py code/data/1.data --compare --memory          # chạy cả 4 và in bảng so sánh
python main.py code/data/2.data --algo astar --out sol.txt  # ghi lời giải ra file (game dùng cách này)
```

`main.py` luôn **cho chạy thử lại lời giải theo luật** trước khi ghi ra file, và trả về mã thoát: `0` có lời giải, `1` vô nghiệm, `2` thuật toán chưa cài, `3` lời giải sai luật, `4` file màn hỏng. Game C++ dựa vào mã này để hiện thông báo.

> ⚠️ Hiện `main.py` luôn dùng **chi phí 1 cho mọi nước**, chưa có tùy chọn chọn hàm chi phí khác. Khi TV3 có hàm chi phí riêng, cần thêm tùy chọn này (xem 9.6).

#### `tests/`: kiểm tra tự động

Chạy `python -m pytest -q`. Hiện tại: **88 passed, 14 skipped** (phần bị bỏ qua là test của BFS, DFS, UCS và heuristic Manhattan chưa cài).

| File | Kiểm tra gì |
|---|---|
| `test_problem.py` | Luật Python có giống C++ không: 12 ô của bảng chuyển, rơi, ô cam, công tắc, cầu, lời giải đã biết |
| `test_solvers.py` | Mỗi thuật toán có tìm được lời giải hợp lệ không; BFS/UCS/A\* có tìm được lời giải ngắn nhất không (màn 1: 10 bước, màn 2: 68 bước) |
| `test_levels.py` | 11 màn: thông tin trong catalog đúng, lời giải tối ưu đúng, màn 11 vô nghiệm, công tắc nào cũng cần thiết… |
| `test_astar_solver.py`, `test_astar_integration.py` | A\* đúng trong các tình huống khó; heuristic không đoán quá trên **mọi** trạng thái của màn 1, 2 (tự chạy khi TV5 cài xong Manhattan) |
| `helpers.py` | Hàm tiện ích: nạp màn, tạo màn nhỏ từ chuỗi để test |

### 5.2. Nửa C++: "tay chân + mắt" (`code/`)

Bạn **không cần hiểu hết** phần này để làm AI. Chỉ cần biết file nào làm gì.

**Nhóm LUẬT** (phải khớp với `ai/problem.py`):

| File | Làm gì | Ứng với Python |
|---|---|---|
| `game.cpp` / `game.h` | Lớp `Game`: `check()` kiểm tra thắng/thua **mỗi khung hình**; `check_after_step()` chạy sau mỗi lần lăn xong: đếm bước, kích hoạt công tắc | `check()`, phần cuối của `simulate()` |
| `tile.cpp` / `tile.h` | `Floor`: đọc file `.data`, lưu lưới; `has_tile()` (có sàn không); `toggle()` bật/tắt cầu. Còn lại là vẽ ô | `Level` |
| `superblock.cpp` / `superblock.h` | `SuperBlock`: khối người chơi. `move()` bắt đầu lăn; `update()` quay dần đủ 90° rồi cập nhật vị trí theo **bảng chuyển** và gọi `check_after_step()` | `TRANSITIONS` |

**Nhóm CẦU NỐI AI:**

| File | Làm gì |
|---|---|
| `autoplay.cpp` / `autoplay.h` | `AutoPlayer` đọc file lời giải `U D L R…` rồi "bấm phím" giùm từng nước khi khối đứng yên. `run_solver()` chạy lệnh `python main.py …`, tự tìm Python (`python`, `py -3`, hoặc biến môi trường `BLOXORZ_PYTHON`), đọc mã thoát |

**Nhóm ỨNG DỤNG:**

| File | Làm gì |
|---|---|
| `main.cpp` | Điểm bắt đầu chương trình. Có 2 chế độ: **chơi bình thường** (menu, chọn màn, Restart, nút giải) và **kiểm tra** (`--play <file>` hoặc `--solve <thuật toán>`: tự chơi, in `WON`/`GAME OVER`, tự thoát). Vòng lặp mỗi khung hình: cập nhật → kiểm tra thắng/thua → vẽ. Chạy solver ở luồng phụ để cửa sổ không đứng |
| `app.h` | Khai báo các thao tác chung: bắt đầu màn, chơi lại, sang màn kế, gọi solver… |
| `controls.cpp` / `controls.h` | Phím tắt: mũi tên lăn khối, `R` chơi lại, `Esc` menu, `C V B X N M` đổi camera, `Tab` bản đồ nhỏ, `F12` chụp màn hình |
| `camera.cpp` / `camera.h` | 6 kiểu camera (bám theo khối, nhìn từ trên, từ tháp…) |

**Nhóm HIỂN THỊ** (chỉ để đẹp, **không chứa luật**):

| File | Làm gì |
|---|---|
| `hud.cpp` / `hud.h` | Giao diện 2D bằng thư viện Dear ImGui: bảng thông số (màn, số bước, chế độ), **menu** (chọn màn, **4 nút BFS / DFS / UCS / A\***, tùy chọn), bảng "LEVEL COMPLETE / GAME OVER" với nút Retry / Next / Menu. Đây là phần đáp ứng yêu cầu "New Game, Restart, nút gọi solver" của đề |
| `render.cpp` / `render.h`, `shaders.h` | Vẽ 3D: mọi ô và khối đều là hình hộp, vẽ hàng loạt bằng một lệnh cho nhanh |
| `effects.cpp` / `effects.h` | Hiệu ứng: ô rơi xuống khi mở màn, bụi khi khối chạm sàn, pháo sáng khi thắng |
| `colors.cpp` / `colors.h` | Bảng màu |
| `screenshot.cpp` / `screenshot.h` | Chụp màn hình thành file PNG (F12) |
| `noedit.cpp` / `noedit.h`, `headers.h` | Tiện ích nhỏ: mở file có đường dẫn tiếng Việt, khai báo chung |

**Build và các file khác:**

| File | Làm gì |
|---|---|
| `makefile` | Lệnh build: `make` (bản thử), `make release` (ra `Bloxorz.exe`), `make dist` (đóng gói zip kèm màn chơi và solver Python) |
| `bloxorz.rc`, `assets/` | Icon và thông tin của file `.exe`; `make_icon.py` vẽ icon |
| `glad.c`, `include/` | Thư viện nạp OpenGL |
| `third_party/imgui/` | Thư viện Dear ImGui (vẽ menu) |
| `data/` | 11 màn chơi + danh mục + ảnh + lời giải |
| `a.out`, `a.exe`, `Bloxorz.exe` | File chạy đã build (không đưa lên git) |
| `solution.txt`, `solver_log.txt` | File tạm: lời giải vừa tìm, nhật ký lần chạy solver gần nhất |
| `readme.MD` | README của tác giả game gốc |

---

## Phần 6. Khung chung của mọi thuật toán tìm kiếm

### 6.1. Ví von: đi tìm đường trong một tòa nhà lạ

Bạn đứng ở cửa một tòa nhà lạ, cần tìm phòng "ĐÍCH". Bạn mang theo hai tờ giấy:

- **Tờ "Danh sách chờ" (frontier):** những phòng **đã biết là có** nhưng **chưa vào xem**.
- **Tờ "Sổ ghi chép" (visited/parent):** những phòng **đã biết tới**, kèm ghi chú "tới phòng này từ phòng nào, đi cửa nào".

Cách làm:

```
1. Ghi phòng xuất phát vào cả hai tờ giấy.
2. Lặp lại:
   a. Chọn MỘT phòng trong "danh sách chờ", gạch khỏi danh sách.      ← mở rộng (expand)
   b. Nếu đó là phòng ĐÍCH → dò ngược sổ ghi chép để biết đường đi. XONG.
   c. Nhìn các cửa của phòng đó, xem dẫn tới những phòng nào.           ← sinh (generate)
   d. Phòng nào CHƯA có trong sổ → ghi vào sổ ("tới từ đây, cửa này")
      và thêm vào danh sách chờ.
3. Danh sách chờ hết sạch mà chưa thấy ĐÍCH → không có đường. XONG.
```

**Cả 4 thuật toán đều làm y như vậy.** Chúng chỉ khác nhau ở **bước 2a: chọn phòng nào trong danh sách chờ để vào tiếp?**

| Thuật toán | Chọn phòng nào tiếp? | Danh sách chờ là |
|---|---|---|
| **BFS** | Phòng được ghi vào **sớm nhất** | Hàng đợi (xếp hàng mua vé: ai tới trước được phục vụ trước) |
| **DFS** | Phòng được ghi vào **muộn nhất** | Ngăn xếp (chồng đĩa: đĩa đặt sau cùng được lấy ra trước) |
| **UCS** | Phòng có **tổng chi phí đã đi `g` nhỏ nhất** | Hàng đợi ưu tiên (heap) theo `g` |
| **A\*** | Phòng có **`g + h` nhỏ nhất** (đã đi + ước đoán còn lại) | Hàng đợi ưu tiên theo `f = g + h` |

### 6.2. Vì sao cần "sổ ghi chép"?

Không có sổ thì bạn sẽ đi vòng tròn mãi. Ví dụ ở màn đồ chơi: từ **A** lăn `U` tới **B**, từ **B** lăn `D` lại về **A**, rồi lại `U` tới **B**… Sổ ghi chép giúp nhận ra "A đến rồi, khỏi thêm vào danh sách chờ nữa".

Sổ còn có công dụng thứ hai: **truy ngược lời giải**. Sổ ghi `parent[K] = (F, "U")` nghĩa là "tới K từ F bằng nước U". Khi tới đích O, ta dò ngược `O ← K ← F ← C ← A`, rồi đảo lại thành `L U U R`. Trong code, việc này do `BaseSolver.reconstruct_path()` làm.

### 6.3. Ba con số đếm

- **Sinh (generated):** mỗi lần nhìn thấy một trạng thái con (bước 2c). `nodes_generated`.
- **Mở rộng (expanded):** mỗi lần lấy một trạng thái ra và sinh con của nó (bước 2a, không tính lần lấy ra đích). `nodes_expanded`. **Đề yêu cầu đếm con số này.**
- **Frontier lớn nhất:** lúc đông nhất, danh sách chờ có bao nhiêu trạng thái. `max_frontier`.

> Các thành viên cần **thống nhất cách đếm** (ví dụ: có tính lần lấy ra đích không, có tính bản ghi cũ bị bỏ qua không). A\* của Sang **không tính** lần lấy ra đích và **không tính** bản ghi cũ. BFS, DFS, UCS nên đếm giống vậy để bảng so sánh công bằng.

---

## Phần 7. BFS — loang theo từng lớp

### 7.1. Ý tưởng

Giống **ném hòn đá xuống mặt hồ**: sóng lan ra thành từng vòng. BFS xét **hết** các trạng thái cách xuất phát 1 nước, rồi **hết** các trạng thái cách 2 nước, rồi 3 nước… Danh sách chờ là **hàng đợi**: vào trước, ra trước.

Mã giả:

```
BFS:
    hàng_đợi ← [xuất_phát];   sổ ← {xuất_phát: không có cha}
    lặp khi hàng_đợi còn phần tử:
        s ← lấy phần tử ĐẦU hàng_đợi
        nếu s là đích: truy ngược sổ → trả về lời giải
        với mỗi (nước a, trạng thái con t) trong successors(s):
            nếu t chưa có trong sổ:
                sổ[t] ← (s, a);   thêm t vào CUỐI hàng_đợi
    trả về "không có lời giải"
```

> **Mẹo:** ghi vào sổ **ngay lúc thêm vào hàng đợi**, không đợi tới lúc lấy ra. Như vậy một trạng thái không bị thêm vào hàng đợi hai lần.

### 7.2. Chạy tay trên màn đồ chơi

(Tên trạng thái xem bảng ở mục 4.2. "đã thấy" = đã có trong sổ, bỏ qua.)

| Bước | Lấy ra | Sinh con | Hàng đợi sau bước này |
|---:|---|---|---|
| 1 | A | U→**B**, L→**C** | B C |
| 2 | B | U→**D**, D→A (đã thấy), L→**E** | C D E |
| 3 | C | U→**F**, R→A (đã thấy) | D E F |
| 4 | D | D→B (đã thấy), L→**G** | E F G |
| 5 | E | U→**H**, D→**I**, L→**J**, R→B (đã thấy) | F G H I J |
| 6 | F | U→**K**, D→C (đã thấy), R→**L** | G H I J K L |
| 7 | G | D→K (đã thấy), R→D (đã thấy) | H I J K L |
| 8 | H | D→E (đã thấy) | I J K L |
| 9 | I | U→E (đã thấy) | J K L |
| 10 | J | U→**M**, D→**N**, R→E (đã thấy) | K L M N |
| 11 | K | U→G, D→F (đều đã thấy), R→**O** | L M N O |
| 12 | L | U→**P**, L→F (đã thấy) | M N O P |
| 13 | M | D→J (đã thấy), R→**Q** | N O P Q |
| 14 | N | U→J (đã thấy), R→**R** | O P Q R |
| 15 | **O** | = **ĐÍCH** | |

Truy ngược sổ: `O ← K (R) ← F (U) ← C (U) ← A (L)` → lời giải **`L U U R`**, **4 nước**, mở rộng **14** trạng thái.

Để ý cách BFS đi theo lớp:

```
  Lớp 0 (0 nước):  A
  Lớp 1 (1 nước):  B C
  Lớp 2 (2 nước):  D E F
  Lớp 3 (3 nước):  G H I J K L
  Lớp 4 (4 nước):  M N O P …      ← O nằm ở lớp 4, nên lời giải ngắn nhất dài 4 nước
```

### 7.3. Vì sao BFS luôn tìm ra lời giải ngắn nhất?

Vì BFS xét **xong hết lớp 3** rồi mới tới lớp 4. Nếu có lời giải 3 nước, BFS đã gặp nó trong lớp 3. Vậy lần đầu gặp đích ở lớp 4 thì không thể có lời giải nào ngắn hơn 4.

Lưu ý: BFS ngắn nhất theo **số nước**, nó không quan tâm "giá" của nước đi. Lời giải `L U U R` chạm ô cam **2 lần**. BFS không biết điều đó là "tệ" (Phần 9 sẽ quay lại chuyện này).

### 7.4. Trên màn thật

Màn 1 có tới **1028** trạng thái đi tới được (vì có 3 công tắc → 2³ = 8 kiểu cấu hình cầu). Lời giải ngắn nhất dài **10** nước. Số trạng thái mỗi lớp tăng dần: 1, 2, 4, 7, 10, 16, 24, … Ví dụ ở lớp 2 có trạng thái `(2,4,Y)` với `bridge_mask = 1`: khối nằm đè lên công tắc ở (2,5), làm cầu (6,4) hiện ra.

### 7.5. "Memory usage" (bộ nhớ) của BFS là gì?

**Định nghĩa:** đề yêu cầu đo **bộ nhớ đỉnh** (peak memory): trong suốt lúc chạy, **thời điểm thuật toán chiếm nhiều bộ nhớ nhất là bao nhiêu**. Không phải tổng bộ nhớ đã từng dùng, cũng không phải bộ nhớ của cả chương trình.

**Ví von:** bạn dọn nhà, bày đồ ra sàn. "Bộ nhớ đỉnh" là **lúc sàn nhà bừa nhất** thì chiếm bao nhiêu mét vuông.

**BFS phải giữ những gì trong bộ nhớ?**

| Thứ phải giữ | Lớn cỡ nào? |
|---|---|
| **Sổ ghi chép** (`parent`) | Một dòng cho **mỗi trạng thái đã gặp**. Chỉ tăng, không bao giờ giảm |
| **Hàng đợi** | Cùng lắm bằng một lớp "sóng" lớn nhất. Màn đồ chơi: lúc đông nhất có 6 trạng thái (bước 6) |
| Mỗi đối tượng `State` | Khoảng 64 byte |

**Vì sao BFS tốn bộ nhớ?** Vì nó phải nhớ **cả một vòng sóng** đang lan, và vòng sóng càng ra xa càng to. Nếu mỗi trạng thái có tối đa 4 con thì lớp thứ `d` có thể có tới cỡ 4^d trạng thái. Ngược lại, DFS chỉ cần nhớ **một nhánh** đang đi (về lý thuyết), nhưng vì cũng phải có sổ ghi chép để chống lặp nên thực tế cũng tốn gần như BFS.

**Đo trong code thế nào?** `BaseSolver.run(measure_memory=True)` bật công cụ `tracemalloc` của Python ngay trước khi thuật toán chạy, rồi hỏi "lúc đỉnh đã cấp phát bao nhiêu byte?". Kết quả lưu vào `SearchResult.peak_memory_kb` (đơn vị KB). Vì chỉ bắt đầu đo khi thuật toán chạy, phần có sẵn trước đó (màn chơi, Python) **không bị tính**.

**Độ lớn để tham khảo** (BFS cài thử ngoài repo, Python 3.12):

| Màn | Độ dài lời giải | Trạng thái đã gặp | Hàng đợi lúc đông nhất | Bộ nhớ đỉnh |
|---|---:|---:|---:|---:|
| 1 | 10 | 207 | 47 | khoảng 26 KB |
| 2 | 68 | 385 | 14 | khoảng 50 KB |

Màn 2 có lời giải dài hơn nhiều nhưng hàng đợi lại nhỏ: màn hẹp, gần như chỉ có một con đường, nên sóng không loang rộng được.

**Hai lưu ý khi đo:**

- **Đo bộ nhớ ở một lần chạy riêng.** `tracemalloc` làm Python chạy chậm hẳn đi. Nếu đo chung thì con số thời gian bị sai. Lệnh `main.py --memory` đã làm đúng như vậy.
- `max_frontier` đếm **số phần tử**, còn `peak_memory_kb` đếm **byte**. Báo cáo nên có cả hai.

---

## Phần 8. DFS — đi sâu tới cùng rồi quay lui

### 8.1. Ý tưởng

Giống **đi trong mê cung**: cứ chọn một lối và đi thẳng vào sâu nhất có thể. Gặp ngõ cụt thì **quay lui** về chỗ rẽ gần nhất và thử lối khác. Danh sách chờ là **ngăn xếp**: thứ bỏ vào sau cùng được lấy ra trước.

Mã giả: giống hệt BFS, chỉ đổi **lấy phần tử ĐẦU hàng đợi** thành **lấy phần tử ĐỈNH ngăn xếp**:

```
DFS:
    ngăn_xếp ← [xuất_phát];   sổ ← {xuất_phát: không có cha}
    lặp khi ngăn_xếp còn phần tử:
        s ← lấy phần tử TRÊN CÙNG ngăn_xếp
        nếu s là đích: truy ngược sổ → trả về lời giải
        với mỗi (a, t) trong successors(s), duyệt theo thứ tự NGƯỢC (R, L, D, U):
            nếu t chưa có trong sổ: sổ[t] ← (s, a);  đặt t lên TRÊN CÙNG ngăn_xếp
    trả về "không có lời giải"
```

Vì sao duyệt ngược? Muốn **lấy ra** theo thứ tự U, D, L, R thì phải **đặt vào** theo thứ tự R, L, D, U (cái đặt sau cùng nằm trên cùng).

Hai lưu ý khi cài:
- Dùng ngăn xếp tự làm (list + `pop()`), **không dùng đệ quy**. Python giới hạn độ sâu đệ quy khoảng 1000, màn lớn sẽ bị lỗi.
- **Bắt buộc có sổ ghi chép** (đề mục 3.3). Không có sổ, DFS sẽ lăn qua lại A ↔ B mãi mãi.

### 8.2. Chạy tay trên màn đồ chơi

| Bước | Lấy ra | Đặt lên ngăn xếp (mới) | Ngăn xếp (đáy → đỉnh) |
|---:|---|---|---|
| 1 | A | C, B | C **B** |
| 2 | B | E, D | C E **D** |
| 3 | D | G | C E **G** |
| 4 | G | K | C E **K** |
| 5 | K | **O** (đích!), F | C E O **F** |
| 6 | F | L | C E O **L** |
| 7–22 | L, P, S, V, X, U, T, Q, M, J, N, W, Y, … , R | (cứ đi sâu vào nhánh F mãi) | O vẫn nằm dưới đáy |
| 23 | **O** | = **ĐÍCH** | |

Kết quả: lời giải **`U U L D R`**, **5 nước**, mở rộng **22** trạng thái.

Điều thú vị: ngay ở **bước 5**, DFS đã **nhìn thấy** đích O. Nhưng O bị đè dưới F trong ngăn xếp, nên DFS cứ lấy F ra và đi sâu thêm 17 bước nữa vào nhánh đó. Hết đường mới quay lại lấy O. (Nếu kiểm tra đích ngay lúc sinh con thì DFS đã dừng ở bước 5. Nhóm chọn cách nào thì phải ghi rõ trong báo cáo.)

### 8.3. Đặc điểm

- **Không tối ưu:** ở đây ra 5 nước, trong khi ngắn nhất là 4. Trên màn lớn, DFS thường cho lời giải dài hơn rất nhiều.
- **Đầy đủ** (có sổ ghi chép và số trạng thái hữu hạn): nếu có lời giải thì chắc chắn tìm ra.
- **Thứ tự thử hướng ảnh hưởng mạnh** tới kết quả. Phải cố định (U, D, L, R) và ghi trong báo cáo.
- **Có thể thay bằng IDS** (đề cho phép, phải ghi rõ): chạy DFS nhưng giới hạn độ sâu 0, rồi 1, rồi 2… Cách này tìm được lời giải ngắn nhất như BFS mà tốn ít bộ nhớ, đổi lại phải đi lại các lớp nông nhiều lần.

---

## Phần 9. UCS — luôn đi tiếp từ chỗ "rẻ nhất"

### 9.1. Ý tưởng

BFS đếm **số bước**. UCS đếm **tiền**. Hãy tưởng tượng đi taxi: mỗi con đường có giá khác nhau (đường thường rẻ, đường có trạm thu phí đắt). UCS luôn đi tiếp từ chỗ mà **tổng tiền đã trả `g` ít nhất**. Danh sách chờ là **hàng đợi ưu tiên (heap)**: lúc nào cũng lấy ra được phần tử có `g` nhỏ nhất.

Mã giả:

```
UCS:
    heap ← [(0, xuất_phát)];   g_tốt_nhất ← {xuất_phát: 0};   sổ ← {xuất_phát: không có cha}
    lặp khi heap còn phần tử:
        (g, s) ← lấy phần tử có g NHỎ NHẤT
        nếu g > g_tốt_nhất[s]: bỏ qua (đây là bản ghi cũ, xem 9.4)
        nếu s là đích: truy ngược sổ → trả về lời giải           ← kiểm tra KHI LẤY RA
        với mỗi (a, t, giá) trong successors(s):
            g_mới ← g + giá
            nếu g_mới < g_tốt_nhất của t (chưa có thì coi là vô cùng):
                g_tốt_nhất[t] ← g_mới;  sổ[t] ← (s, a);  đưa (g_mới, t) vào heap
    trả về "không có lời giải"
```

(Khi cài bằng `heapq`, phần tử nên là `(g, số_thứ_tự, trạng_thái)`. `số_thứ_tự` tăng dần để khi hai phần tử cùng `g`, Python không phải so sánh hai `State` với nhau.)

### 9.2. Nếu mọi nước đều giá 1 thì sao?

Thì "tiền đã trả" = "số bước đã đi", và UCS chạy **y hệt BFS**. Trên màn đồ chơi, UCS chi phí 1 cũng mở rộng 14 trạng thái và ra `L U U R`, giống bảng 7.2. Đề nói rõ điều này (mục 3.4): *"if every move has cost 1, UCS reduces to BFS"*. Vì vậy phải **thiết kế giá khác nhau** cho các nước đi.

### 9.3. "Hàm chi phí không đồng nhất" và "bộ trọng số thích hợp" nghĩa là gì?

**Hàm chi phí** là quy tắc trả lời câu hỏi "nước đi này giá bao nhiêu?". Trong code là `cost_fn(trạng_thái_cũ, nước_đi, trạng_thái_mới) → số`.

- **Đồng nhất:** nước nào cũng giá 1 (`unit_cost`).
- **Không đồng nhất:** các **loại** nước đi có giá **khác nhau**.

**Bộ trọng số** là bảng giá cụ thể cho từng loại nước, giống bảng giá cước taxi. Ví dụ:

| Loại nước | Giá (ví dụ A: phạt ô cam) | Giá (ví dụ B: theo sức lực) |
|---|---:|---:|
| Nước bình thường | 1 | — |
| Nước làm khối chạm ô cam | 3 (= 1 + phạt 2) | — |
| Đổ khối từ đứng xuống nằm | — | 1.118 |
| Lăn ngang khi đang nằm | — | 1.207 |
| Dựng khối từ nằm lên đứng | — | 1.618 |

**"Thích hợp"** nghĩa là bộ trọng số thỏa 4 điều:

1. **Mọi giá đều dương** (lớn hơn 0). Giá 0 hoặc âm làm UCS và A\* chạy sai.
2. **Giải thích được bằng lời** trong báo cáo (đề mục 3.4: *"as long as you give a reasonable explanation"*). Ví dụ A: "lăn lên ô cam nguy hiểm, lỡ tay là vỡ, nên tính đắt". Ví dụ B: "dựng khối lên phải nâng trọng tâm cao hơn nên tốn sức hơn".
3. **Thực sự làm UCS chọn khác BFS** trên các màn thử. Nếu không, phần UCS mất ý nghĩa (xem 9.5).
4. **A\* dùng chung bảng giá với UCS**, và heuristic phải "không đoán quá" **theo bảng giá đó** (đề mục 3.5). Khi đó UCS và A\* phải ra **cùng tổng chi phí** trên mọi màn, một cách tự kiểm tra rất tốt.

Theo Q&A với giảng viên (câu 10 trong `report0.md`), nhóm **được dùng học máy để tìm ra bộ trọng số**, miễn trình bày rõ cách làm.

### 9.4. Chạy tay UCS với giá "phạt ô cam" trên màn đồ chơi

Bảng giá: **mỗi nước giá 1, cộng thêm 2 nếu sau nước đó khối chạm ô cam.** Ví dụ B → E giá 3 (E nằm trên 2 ô cam), C → F giá 3 (F chạm 1 ô cam), G → K giá 3.

| Bước | Lấy ra (g) | Sinh con (g mới) | Ghi chú |
|---:|---|---|---|
| 1 | A (0) | B (1), C (1) | |
| 2 | B (1) | D (2), **E (4)** | B→E chạm ô cam: 1 + 3 = 4 |
| 3 | C (1) | **F (4)** | C→F chạm ô cam: 1 + 3 = 4 |
| 4 | D (2) | G (3) | đường "vòng bên phải" đang rẻ |
| 5 | G (3) | **K (6)** | G→K chạm ô cam: 3 + 3 = 6 |
| 6 | E (4) | H (5), I (5), J (5) | |
| 7 | F (4) | L (5) | F→K sẽ là 4 + 3 = 7, không rẻ hơn 6, nên **không cập nhật** |
| 8–11 | H, I, J, L (5) | M (6), N (6), P (6) | |
| 12 | K (6) | **O (7)** | đích được sinh ra với g = 7 |
| 13–15 | M, N, P (6) | … | vẫn còn trạng thái g = 6 rẻ hơn 7, phải xét hết |
| 16 | **O (7)** | = **ĐÍCH** | |

Kết quả: lời giải **`U U L D R`**, **5 nước**, **tổng giá 7**, mở rộng 15 trạng thái.

So sánh với BFS:

| | Lời giải | Số nước | Chạm ô cam | Tổng giá (phạt ô cam) |
|---|---|---:|---:|---:|
| BFS | `L U U R` | **4** | 2 lần | 1 + 3 + 3 + 1 = 8 |
| UCS | `U U L D R` | 5 | 1 lần | 1 + 1 + 1 + 3 + 1 = **7** |

**Đây chính là điều đề muốn thấy:** UCS chấp nhận đi **thêm 1 nước** để **bớt 1 lần** chạm ô cam nguy hiểm. BFS không làm được vậy vì nó chỉ đếm số bước.

### 9.5. Hai câu hỏi hay bị hỏi về UCS

**(a) Vì sao phải kiểm tra đích lúc LẤY RA, không phải lúc SINH?**

Ví dụ nhỏ: từ S có đường thẳng tới đích G giá **10**, và đường vòng S → A → G giá **1 + 1 = 2**. Lúc mở rộng S, ta **sinh** ra G với g = 10. Nếu dừng ngay thì chọn đường giá 10, sai. Còn nếu đợi: heap lấy A (g = 1) ra trước, sinh G với g = 2, rồi lấy G (g = 2) ra. Lần đầu **lấy ra** đích là lúc giá đã nhỏ nhất. (A\* của Sang có test `test_astar_waits_for_cheapest_goal_to_be_popped` kiểm tra đúng tình huống này.)

**(b) "Bản ghi cũ" (lazy deletion) là gì?**

Heap của Python không cho sửa phần tử đã đưa vào. Khi tìm được đường rẻ hơn tới một trạng thái, ta **đưa thêm bản mới** (giá rẻ hơn) vào heap và **để bản cũ nằm đó**. Lúc lấy bản cũ ra, thấy `g > g_tốt_nhất` thì bỏ qua, không tính là mở rộng. Giống như có hai vé cho cùng một chuyến: dùng vé rẻ, vé đắt vứt đi.

### 9.6. Cảnh báo: bộ trọng số "theo sức lực" gần như không đổi được lời giải

Ví dụ B ở trên (1.118 / 1.207 / 1.618) nghe rất hợp lý, nhưng có một cái bẫy.

- Mỗi lần **đổ** khối xuống, về sau phải có đúng một lần **dựng** khối lên (vì cuối cùng phải đứng trên lỗ đích). Vậy số lần đổ **luôn bằng** số lần dựng. Gọi số đó là `k`, số lần lăn ngang là `r`.
- Số nước `L = 2k + r`, và tổng giá là: `1.118·k + 1.618·k + 1.207·r = 1.207·L + 0.322·k`.
- Nghĩa là tổng giá **gần như chỉ phụ thuộc số nước `L`**. Phần `0.322·k` rất nhỏ. Muốn một lời giải dài hơn 1 nước mà vẫn rẻ hơn, nó phải có **ít hơn ít nhất 4 lần** đổ-dựng. Rất hiếm khi xảy ra.

Thực tế đã kiểm tra: trên màn 1 và màn 2, UCS với bảng giá này ra **đúng lời giải của BFS**. Thử 40.000 màn nhỏ ngẫu nhiên cũng không màn nào khác.

**Lời khuyên cho TV3:** nên dùng kiểu phạt ô cam (hoặc kết hợp), hoặc làm các giá chênh nhau nhiều hơn, và cần có màn chơi có **nhiều đường đi khác nhau** (TV4 thiết kế) để thấy UCS khác BFS. Đưa phép phân tích này vào báo cáo cũng là một điểm cộng.

**Cắm vào code:** viết hàm `(trạng_thái_cũ, nước, trạng_thái_mới) → giá`, rồi truyền vào `BloxorzProblem(level, cost_fn=...)`. `successors()` sẽ tự trả về giá theo hàm đó. Còn thiếu: thêm tùy chọn trong `main.py` (ví dụ `--cost unit|fragile`) để nút UCS trong game dùng được hàm này.

---

## Phần 10. A\* — rẻ nhất + biết hướng về đích

### 10.1. Ý tưởng

UCS giống người đi đường **bịt mắt**: chỉ biết mình đã tốn bao nhiêu (`g`), không biết đích ở hướng nào, nên loang đều ra mọi phía. A\* giống người có **la bàn**: ngoài `g` còn có **ước đoán `h`** "từ đây tới đích còn khoảng bao nhiêu". A\* chọn trạng thái có

```
f = g + h  =  (đã tốn)  +  (ước đoán còn lại)
```

nhỏ nhất. Giống GPS: chọn hướng mà **tổng quãng đường dự kiến** ngắn nhất, không chỉ quãng đã đi ngắn nhất.

Code A\* đã có trong `ai/solvers/astar_solver.py` (Sang cài): giống UCS, chỉ khác khóa của heap là `(f, h, số_thứ_tự, g, trạng_thái)`. Hai trạng thái cùng `f` thì ưu tiên cái có `h` nhỏ hơn (gần đích hơn). Giải thích từng dòng ở `Docs/astar_walkthrough.md`.

### 10.2. Heuristic Manhattan (việc của TV5)

**Bước 1: đo khoảng cách.** Gọi `d` là **khoảng cách Manhattan nhỏ nhất** từ các ô khối đang chiếm tới lỗ đích. Khoảng cách Manhattan = số ô phải đi nếu chỉ được đi ngang/dọc: `|x₁ − x₂| + |y₁ − y₂|`.

**Bước 2: đổi ra số nước.** Theo sự thật nhỏ số 2 ở mục 3.4, mỗi nước lăn làm `d` **giảm tối đa 2**. Vậy muốn `d` về 0 cần **ít nhất** `⌈d / 2⌉` nước (`⌈ ⌉` là làm tròn **lên**). Chưa thắng thì cần ít nhất 1 nước. Ta có:

```
h = 0                       nếu đang đứng trên đích (đã thắng)
h = max( ⌈d / 2⌉ , 1 )      nếu chưa
```

**Ví dụ trên màn đồ chơi** (đích ở (2,2)):

| Trạng thái | Ô chiếm | Khoảng cách tới (2,2) | d | h |
|---|---|---|---:|---:|
| A `(2,0,Z)` | (2,0) | 0 + 2 = 2 | 2 | ⌈2/2⌉ = 1 |
| B `(2,1,Y)` | (2,1), (2,2) | 1 và **0** | 0 | max(0, 1) = **1** |
| C `(0,0,X)` | (0,0), (1,0) | 4 và **3** | 3 | ⌈3/2⌉ = 2 |
| K `(0,2,X)` | (0,2), (1,2) | 2 và **1** | 1 | ⌈1/2⌉ = 1 |
| O `(2,2,Z)` | (2,2) | đã thắng | — | **0** |

B nằm vắt qua lỗ đích nên `d = 0`, nhưng chưa thắng (chưa đứng), nên vẫn cần ít nhất 1 nước. Đó là lý do có `max(…, 1)`.

### 10.3. Admissible và consistent: nói bằng lời thường

**Admissible = "không bao giờ hứa quá".** Heuristic luôn đoán **≤** chi phí thật còn lại. Giống GPS nói "còn ít nhất 5 km" thì thật sự không thể dưới 5 km. Nếu heuristic hứa quá (đoán 10 khi thật ra chỉ cần 7), A\* có thể bỏ qua đường tốt nhất và **ra lời giải không tối ưu**.

> Vì sao Manhattan ở trên không hứa quá? Vì mỗi nước làm `d` giảm **nhiều nhất** 2, nên thật sự cần **ít nhất** `⌈d/2⌉` nước. `h` chính là con số "ít nhất" đó, không thể vượt chi phí thật.

**Consistent = "không tụt đột ngột".** Đi một nước có giá `c` thì `h` giảm **không quá `c`**. Viết thành công thức: `h(trước) ≤ c + h(sau)`. Heuristic consistent thì mỗi trạng thái chỉ cần mở rộng **một lần**. Consistent thì chắc chắn admissible.

> Vì sao Manhattan ở trên consistent (với giá 1)? Mỗi nước làm `d` giảm tối đa 2, nên `⌈d/2⌉` giảm tối đa 1, nên `h` giảm tối đa 1 = giá của nước đó.

Bộ test `tests/test_astar_integration.py` (Sang viết) sẽ **tự kiểm tra cả hai tính chất** trên mọi trạng thái của màn 1 và 2 khi TV5 cài xong hàm.

### 10.4. Hai cái bẫy khi thiết kế heuristic

**Bẫy 1: "Đang nằm thì cộng thêm 1".** Đề gợi ý thêm "phạt" khi khối chưa đứng. Nhưng công thức `⌈d/2⌉ + 1` **sai** (hứa quá). Phản ví dụ (đã chạy thử): hành lang dọc 4 ô, khối **nằm** chiếm (1,0) và (2,0), đích ở (3,0). Ta có `d = 1`, công thức cho `⌈1/2⌉ + 1 = 2`, nhưng chỉ cần **1** nước (`R` → dựng đứng đúng trên (3,0)). Dùng `max(…, 1)` như trên thì an toàn.

**Bẫy 2: "Khoảng cách điểm" mà quên chia 2.** Đề gợi ý tính đường đi ngắn nhất của một "điểm" trên lưới (bỏ qua tư thế khối). Ý này hay hơn Manhattan vì biết tránh ô trống. Nhưng **phải chia 2 rồi làm tròn lên**. Phản ví dụ (đã chạy thử): khối đứng ở (0,0), đích ở (3,0) trên hành lang thẳng. Điểm phải đi 3 ô, nhưng khối chỉ cần **2** nước (`R` đổ xuống chiếm (1,0),(2,0), rồi `R` dựng lên ở (3,0)). Đoán 3 là hứa quá.

### 10.5. Chạy tay A\* trên màn đồ chơi (giá 1, heuristic Manhattan)

| Bước | Lấy ra (f = g + h) | Sinh con: g / h / f | Ghi chú |
|---:|---|---|---|
| 1 | A (0 + 1) | B 1/1/**2**, C 1/2/**3** | B gần đích hơn nên f nhỏ hơn |
| 2 | B (1 + 1) | D 2/1/3, E 2/1/3 | |
| 3 | D (2 + 1) | G 3/1/4 | D, E, C cùng f = 3; D và E có h = 1 < h(C) = 2 nên được chọn trước |
| 4 | E (2 + 1) | H 3/1/4, I 3/2/5, J 3/1/4 | |
| 5 | C (1 + 2) | F 2/1/3 | |
| 6 | F (2 + 1) | K 3/1/4, L 3/1/4 | |
| 7–9 | G, H, J (f = 4) | M 4/2/6, N 4/2/6 | các con mới đều không rẻ hơn |
| 10 | K (3 + 1) | **O 4/0/4** | đích được sinh ra |
| 11 | **O** (4 + 0) | = **ĐÍCH** | |

Kết quả: **`L U U R`**, 4 nước (tối ưu, giống BFS), nhưng chỉ mở rộng **10** trạng thái thay vì 14. A\* **không cần mở rộng** I, L, M, N: lúc tìm ra đích, chúng vẫn còn nằm trong heap. I, M, N có f lớn hơn 4 ("đi hướng đó chắc chắn tốn hơn"). L cũng có f = 4 nhưng O có h = 0 nên được lấy ra trước.

### 10.6. Khi dùng bảng giá không đồng nhất

Heuristic phải không hứa quá **theo bảng giá đang dùng**:

- **Cách đơn giản:** lấy số nước tối thiểu nhân với **giá rẻ nhất của một nước**. Với bảng phạt ô cam, giá rẻ nhất là 1, nên dùng nguyên heuristic ở trên là được.
- **Cẩn thận:** nếu có loại nước rẻ hơn 1 (ví dụ bấm công tắc giá 0.5) thì phải nhân với 0.5. Nếu không, heuristic sẽ hứa quá.
- **Tự kiểm tra:** cùng bảng giá thì A\* và UCS phải ra **cùng tổng giá**. Khác nhau nghĩa là heuristic đang hứa quá hoặc có lỗi.

### 10.7. Kết quả A\* trên 11 màn thật (heuristic = 0, tức là chạy như UCS giá 1)

| Màn | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Số nước | 10 | 68 | 4 | 16 | 6 | 20 | 12 | 15 | 36 | 27 | vô nghiệm |
| Số node mở rộng | 160 | 377 | 4 | 127 | 52 | 215 | 238 | 232 | 1393 | 1201 | 113 |

Khi TV5 cài xong Manhattan, số node mở rộng phải **giảm** (độ dài lời giải giữ nguyên). Đó là bằng chứng heuristic có ích.

---

## Phần 11. So sánh 4 thuật toán

### 11.1. Trên màn đồ chơi

| Thuật toán | Danh sách chờ | Lời giải | Số nước | Tổng giá (phạt ô cam) | Số node mở rộng |
|---|---|---|---:|---:|---:|
| BFS | Hàng đợi | `L U U R` | **4** | 8 | 14 |
| DFS | Ngăn xếp | `U U L D R` | 5 | 7 | 22 |
| UCS (giá 1) | Heap theo g | `L U U R` | **4** | 8 | 14 |
| UCS (phạt ô cam) | Heap theo g | `U U L D R` | 5 | **7** | 15 |
| A\* (giá 1, Manhattan) | Heap theo g + h | `L U U R` | **4** | 8 | **10** |

DFS ra đúng lời giải mà UCS (phạt ô cam) chọn chỉ là **may mắn** do thứ tự thử hướng. DFS không hề biết gì về giá.

### 11.2. Bảng lý thuyết (dùng cho báo cáo)

| | BFS | DFS | UCS | A\* |
|---|---|---|---|---|
| Chọn tiếp | Vào sớm nhất | Vào muộn nhất | g nhỏ nhất | g + h nhỏ nhất |
| Đầy đủ? | Có | Có (khi có sổ ghi chép) | Có (giá > 0) | Có |
| Tối ưu? | Theo **số nước** | **Không** | Theo **tổng giá** | Theo tổng giá (khi h không hứa quá) |
| Bộ nhớ | Lớn (cả vòng sóng) | Lý thuyết nhỏ, thực tế lớn do sổ ghi chép | Lớn | Lớn, nhưng thường xét ít node nhất |
| Điểm mạnh | Đơn giản, chắc chắn ngắn nhất | Có thể gặp đích rất nhanh | Tính được "giá" | Nhanh nhất nếu h tốt |
| Điểm yếu | Loang mọi hướng | Lời giải dài, phụ thuộc may rủi | Cũng loang mọi hướng | Cần thiết kế và chứng minh h |

---

## Phần 12. Đo đạc cho phần thực nghiệm

Phần thực nghiệm chiếm **20 điểm** báo cáo. Đề (mục 3.6) yêu cầu: chạy trên một **bộ màn cố định**, chia nhóm theo độ khó, đo **4 con số** cho mỗi thuật toán, trình bày bằng **bảng + biểu đồ + nhận xét**.

| Con số | Lấy ở đâu | Đo cho đúng |
|---|---|---|
| Thời gian | `SearchResult.time_ms` | Chạy vài lần, lấy giá trị giữa (trung vị); không in log trong lúc tìm |
| Bộ nhớ đỉnh | `SearchResult.peak_memory_kb` | Đo ở một lần chạy riêng (`--memory`) |
| Số node mở rộng | `SearchResult.nodes_expanded` | Cả nhóm thống nhất cách đếm (mục 6.3) |
| Độ dài lời giải | `SearchResult.solution_length` | Ghi thêm `cost` khi dùng bảng giá không đồng nhất |

Bộ màn hiện có 11 màn, chia nhóm: G1 cơ bản (màn 3, 4), G2 có ô cam (5, 6), G3 có công tắc (1, 2, 7, 8), G5 kết hợp (9), G6 đặc biệt (10 sân rộng 20×30, 11 vô nghiệm).

Lệnh nhanh: `python main.py code/data/<n>.data --compare --memory`. Bản chính thức nên có script riêng (ví dụ `experiments/run_all.py`) chạy mọi màn × mọi thuật toán, ghi ra file CSV, rồi vẽ biểu đồ (trục số node nên dùng thang log).

**Nhận xét cần kiểm chứng bằng số liệu thật** (đừng viết trước): BFS và A\* ra cùng độ dài nhưng A\* mở rộng ít node hơn; DFS có lúc nhanh nhưng lời giải dài; UCS chỉ khác BFS khi bảng giá đủ chênh và màn có nhiều đường đi.

---

## Phần 13. Còn thiếu gì so với đề?

| Việc | Mục đề | Ai | Trạng thái |
|---|---|---|---|
| Game 3D, luật cơ bản, ô cam, công tắc thường/nặng, cầu bật/tắt | 3.1 | (đã có) | ✅ |
| Menu: chọn màn, Restart, 4 nút thuật toán | 3.1 | (đã có) | ✅ |
| Bộ ≥ 10 màn | Q&A | TV4 | ✅ 11 màn |
| A\* | 3.5 | TV4 | ✅ |
| `manhattan_heuristic` + chứng minh | 3.5 | **TV5** | ⏳ |
| BFS / DFS / UCS | 3.2–3.4 | TV1 / TV2 / TV3 | ⏳ |
| Bảng giá không đồng nhất + tùy chọn trong `main.py` | 3.4 | TV3 | ⏳ |
| **Split switch** (tách khối thành 2 khối nhỏ), cầu **permanent**, một công tắc điều khiển **nhiều** cầu | 3.1 (10đ ô đặc biệt) | TV1 (+ TV2 phần vẽ) | ⏳ phải sửa **cả** C++ lẫn Python |
| Thêm màn nhiều công tắc, nhiều ô cam (để thực nghiệm thấy rõ khác biệt) | 3.6 | TV4 | ⏳ gợi ý |
| Script thực nghiệm, CSV, biểu đồ | 3.6 | TV5 | ⏳ |
| Báo cáo PDF, video YouTube | 3.6, 3.7 | Cả nhóm | ⏳ |

---

## Phần 14. Câu hỏi vấn đáp thường gặp

**Hỏi: Trạng thái gồm những gì? Vì sao cần `bridge_mask`?**
Đáp: Gồm `(x, y, tư thế, bridge_mask)`. Cùng vị trí khối nhưng cầu khác thì nước đi được cũng khác, nên phải là trạng thái khác. Thiếu `bridge_mask` thì thuật toán bỏ sót những lời giải phải quay lại chỗ cũ sau khi bấm công tắc.

**Hỏi: Nước đi làm rơi được xử lý ở đâu?**
Đáp: `simulate()` trả về kết quả "thua", và `successors()` tự bỏ những nước đó. Thuật toán không bao giờ nhìn thấy trạng thái "đã rơi".

**Hỏi: Vì sao BFS ra lời giải ngắn nhất còn DFS thì không?**
Đáp: BFS xét hết các trạng thái cách `k` nước rồi mới tới `k+1`, nên lần đầu gặp đích là ở lớp nông nhất. DFS lao sâu theo một nhánh và trả về đích đầu tiên nó lấy ra, bất kể dài hay ngắn (màn đồ chơi: DFS ra 5 nước, BFS ra 4).

**Hỏi: Khi nào UCS giống BFS?**
Đáp: Khi mọi nước cùng giá. Bảng giá "theo sức lực" cũng làm UCS gần như giống BFS, vì tổng giá = `1.207·L + 0.322·k` (mục 9.6).

**Hỏi: Vì sao UCS kiểm tra đích lúc lấy ra, không phải lúc sinh?**
Đáp: Lúc sinh, đích có thể đang đi theo đường đắt. Đường rẻ hơn có thể vẫn nằm trong heap. Lấy ra theo g nhỏ nhất thì lần đầu lấy ra đích chắc chắn là rẻ nhất (ví dụ S→G giá 10 so với S→A→G giá 2).

**Hỏi: Admissible và consistent khác nhau thế nào?**
Đáp: Admissible: không bao giờ đoán quá chi phí thật. Consistent: đi một nước giá `c` thì `h` giảm không quá `c`. Consistent thì chắc chắn admissible. Heuristic `max(⌈d/2⌉, 1)` có cả hai với giá 1, vì mỗi nước làm `d` giảm tối đa 2.

**Hỏi: Vì sao không cộng "+1 nếu đang nằm"?**
Đáp: Vì sẽ hứa quá. Khối nằm cách đích 1 ô có thể dựng đứng vào đích chỉ bằng 1 nước, nhưng công thức đoán 2 (mục 10.4).

**Hỏi: "Memory usage" là gì, đo thế nào?**
Đáp: Bộ nhớ **đỉnh** thuật toán chiếm trong lúc tìm kiếm. Đo bằng `tracemalloc` trong `BaseSolver.run(measure_memory=True)`, ở một lần chạy riêng để không làm sai thời gian. Với BFS, phần lớn là sổ ghi chép (mọi trạng thái đã gặp) và hàng đợi (mục 7.5).

**Hỏi: Làm sao chắc luật Python giống game C++?**
Đáp: Python chép đúng thứ tự kiểm tra của C++; có test luật trong `tests/test_problem.py`; và lời giải của cả 10 màn có lời giải đều được cho game C++ tự chơi lại bằng `--play`, thắng đúng số bước. `main.py` cũng tự chơi lại mọi lời giải trước khi ghi ra file.

**Hỏi: Game viết bằng C++, có vi phạm "source code must be written in Python" không?**
Đáp: Theo Q&A với giảng viên (câu 7), giao diện được dùng ngôn ngữ khác, nhưng **thuật toán phải bằng Python**. Toàn bộ phần tìm kiếm nằm trong `ai/`, viết bằng Python thuần, tự cài. Game C++ chỉ gọi `main.py` và chơi lại kết quả. Mã nguồn game gốc được dùng theo phép giảng viên và ghi nguồn đầy đủ.
