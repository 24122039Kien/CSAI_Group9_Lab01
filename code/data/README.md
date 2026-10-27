# Bộ màn chơi Bloxorz — TV4

Bộ dữ liệu có **16 màn: 15 màn có lời giải và 1 màn vô nghiệm**. Màn 1–14
dùng định dạng `.data` v1 (sàn, lỗ, ô dễ vỡ, công tắc thường/nặng, cầu
toggle); màn 15–16 (của Đạt) dùng phần mở rộng v2 `ext 1`: một công tắc
điều khiển nhiều ô cầu, công tắc permanent và split switch. Màn 1–2 giữ
nguyên dữ liệu gốc; màn 3–16 bổ sung cho kiểm thử và thực nghiệm. Mỗi lời giải được phát lại trong mô phỏng Python
và đối chiếu độ dài với BFS độc lập trong bộ test.

G1: cơ bản; G2: ô dễ vỡ; G3: công tắc/cầu; G4: split switch; G5: màn lớn
kết hợp; G6: đặc biệt. Cú pháp v2 xem `Docs/v2_walkthrough.md` và
`Docs/split_walkthrough.md`.

## Danh mục

| Màn | Nhóm | Tên | Lưới | Ô có sàn | Dễ vỡ | CT thường/nặng | Ô cầu | Tối ưu |
|---:|---|---|---|---:|---:|---|---:|---:|
| [1](1.data) | G3 | Upstream level 1 | 13×13 | 47 | 1 | 2/1 | 3 | 10 |
| [2](2.data) | G3 | Upstream level 2 | 19×14 | 82 | 39 | 0/3 | 3 | 68 |
| [3](3.data) | G1 | First rolls | 7×13 | 8 | 0 | 0/0 | 0 | 4 |
| [4](4.data) | G1 | Turning courtyard | 11×15 | 59 | 0 | 0/0 | 0 | 16 |
| [5](5.data) | G2 | Fragile detour | 9×13 | 44 | 1 | 0/0 | 0 | 6 |
| [6](6.data) | G2 | Fragile maze | 12×16 | 83 | 1 | 0/0 | 0 | 20 |
| [7](7.data) | G3 | Soft switch crossing | 11×19 | 69 | 0 | 1/0 | 1 | 12 |
| [8](8.data) | G3 | Heavy switch crossing | 11×19 | 69 | 0 | 0/1 | 1 | 15 |
| [9](9.data) | G5 | Three chambers | 19×25 | 284 | 4 | 1/1 | 2 | 36 |
| [10](10.data) | G6 | Open field | 20×30 | 415 | 0 | 0/0 | 0 | 27 |
| [11](11.data) | G6 | Disconnected islands | 11×17 | 76 | 0 | 0/0 | 0 | Vô nghiệm |
| [12](12.data) | G5 | 4-switch vault | 17×35 | 158 | 0 | 2/2 | 4 | 44 |
| [13](13.data) | G5 | 6-switch vault | 21×49 | 278 | 0 | 3/3 | 6 | 66 |
| [14](14.data) | G2 | Amber labyrinth | 21×29 | 363 | 200 | 0/0 | 0 | 36 |
| [15](15.data) | G3 | Dual bridges crossing | 14×17 | 57 | 0 | 2/0 | 3 | 37 |
| [16](16.data) | G4 | The great split | 14×18 | 48 | 0 | 2/0 | 6 | 49 |

Ô có sàn là số ô khác 0 trong lưới ban đầu, gồm công tắc và ô dễ vỡ; không
gồm lỗ đích hoặc cầu đang tắt. `catalog.json` là danh mục để test và công cụ
thực nghiệm đọc tự động. Chi phí đang dùng là **1 mỗi nước**.

![Tổng quan các màn](previews/overview.png)

## Cách chơi và phát lại

Game tự liệt kê các file đánh số trong `data/`, nên các màn mới xuất hiện
trong menu New Game/chọn màn mà không cần đổi mã giao diện.
Chạy từ thư mục `code/` sau khi build theo README của dự án:

```bash
./a.out 3                                 # chơi màn 3 trên Linux/WSL
./a.out 7 --play data/solutions/7.txt       # tự phát lại màn công tắc
./a.out 9 --play data/solutions/9.txt       # màn lớn kết hợp
./a.out 13 --play data/solutions/13.txt     # màn khó 6 công tắc
```

Windows native dùng `./a.exe` hoặc `./Bloxorz.exe` thay cho `./a.out`.
Màn 11 không có file lời giải thắng; dùng để kiểm tra solver trả về
`no_solution` sau khi duyệt hết vùng xuất phát.

Các hình bên dưới là **sơ đồ lưới ban đầu**, không phải ảnh chụp game 3D.
`S` là khối ban đầu, `G` là lỗ đích; số trên công tắc và ô cầu biểu thị
cùng một cặp điều khiển. Tọa độ `x` là dòng, `y` là cột; `U: y+1`,
`D: y−1`, `R: x+1`, `L: x−1` (theo quy ước game C++).

## Chi tiết từng màn

### Màn 1 — Upstream level 1

Màn gốc: công tắc, cầu và ô dễ vỡ.

Lời giải tối ưu **10 bước**: [file phát lại](solutions/1.txt).

```text
R U U R U R U R D L
```

![Sơ đồ màn 1](previews/1.png)

### Màn 2 — Upstream level 2

Màn gốc: công tắc, cầu và ô dễ vỡ.

Lời giải tối ưu **68 bước**: [file phát lại](solutions/2.txt).

```text
D R D L D L U R U D L D R U R D R R U R R D R R R R U D L L L L U L L L D D L U R R R R R U U R U R R R U R L U U L L L L L D L U R D L
```

![Sơ đồ màn 2](previews/2.png)

### Màn 3 — First rolls

Hành lang thẳng để học chu kỳ đứng–nằm của khối.

Lời giải tối ưu **4 bước**: [file phát lại](solutions/3.txt).

```text
U U U U
```

![Sơ đồ màn 3](previews/3.png)

### Màn 4 — Turning courtyard

Sân có khoảng trống buộc đổi hướng và tư thế.

Lời giải tối ưu **16 bước**: [file phát lại](solutions/4.txt).

```text
U R U R D L U R D R D D D L L L
```

![Sơ đồ màn 4](previews/4.png)

### Màn 5 — Fragile detour

Ô dễ vỡ chặn điểm dựng đứng trên đường thẳng, buộc đi vòng.

Lời giải tối ưu **6 bước**: [file phát lại](solutions/5.txt).

```text
U L U U R U
```

Kiểm chứng vai trò ô dễ vỡ: nếu thay chúng bằng sàn thường, tối ưu giảm từ 6 xuống 4 bước.

![Sơ đồ màn 5](previews/5.png)

### Màn 6 — Fragile maze

Lối đi có hốc và ô dễ vỡ; phải chọn lại đường dựng đứng.

Lời giải tối ưu **20 bước**: [file phát lại](solutions/6.txt).

```text
U R U U R U L L D D R D D D D R R U U R
```

Kiểm chứng vai trò ô dễ vỡ: nếu thay chúng bằng sàn thường, tối ưu giảm từ 20 xuống 18 bước.

![Sơ đồ màn 6](previews/6.png)

### Màn 7 — Soft switch crossing

Phải kích hoạt công tắc thường để mở cầu sang phòng đích.

Lời giải tối ưu **12 bước**: [file phát lại](solutions/7.txt).

```text
L U U U U R U U U U U U
```

Kiểm chứng vai trò cầu: bỏ riêng từng công tắc và giữ cầu của nó đóng thì màn trở thành vô nghiệm. Lời giải ghi nhận đúng việc đứng trên công tắc nặng.

![Sơ đồ màn 7](previews/7.png)

### Màn 8 — Heavy switch crossing

Phải đứng trên công tắc nặng trước khi băng qua cầu.

Lời giải tối ưu **15 bước**: [file phát lại](solutions/8.txt).

```text
U L L U U R R U U U U U L U R
```

Kiểm chứng vai trò cầu: bỏ riêng từng công tắc và giữ cầu của nó đóng thì màn trở thành vô nghiệm. Lời giải ghi nhận đúng việc đứng trên công tắc nặng.

![Sơ đồ màn 8](previews/8.png)

### Màn 9 — Three chambers

Màn lớn 19×25: hai khe ngăn phòng, công tắc thường/nặng và ô dễ vỡ.

Lời giải tối ưu **36 bước**: [file phát lại](solutions/9.txt).

```text
R U R R R R R R U U L L L L L U U U U L U L R U U R R R R R U U U U R R
```

Kiểm chứng vai trò cầu: bỏ riêng từng công tắc và giữ cầu của nó đóng thì màn trở thành vô nghiệm. Lời giải ghi nhận đúng việc đứng trên công tắc nặng.

![Sơ đồ màn 9](previews/9.png)

### Màn 10 — Open field

Sân trống 20×30 để so sánh lượng trạng thái và thời gian tìm kiếm.

Lời giải tối ưu **27 bước**: [file phát lại](solutions/10.txt).

```text
U U U U U U U U U U U U U U U U R U R R R R R R R R R
```

![Sơ đồ màn 10](previews/10.png)

### Màn 11 — Disconnected islands

Hai đảo không có cầu; vùng xuất phát có nhiều nước đi nhưng không thể tới đích.

**Vô nghiệm**: hai đảo không nối nhau; BFS duyệt hết 113 trạng thái từ điểm đầu mà không gặp đích.

![Sơ đồ màn 11](previews/11.png)

### Màn 12 — 4-switch vault

Phòng công tắc 4 bit độc lập; phải mở đủ các cầu trên hành lang dẫn tới đích.

Lời giải tối ưu **44 bước**: [file phát lại](solutions/12.txt).

```text
L L L R U R R D R R R R U U U U L U R L L L L L L L R R R U R U U U U U U U U U U U U U
```

BFS/A* `h=0` mở rộng **6,663 node** và gặp đủ **16 cấu hình cầu**. Bỏ riêng từng công tắc thì màn vô nghiệm.

![Sơ đồ màn 12](previews/12.png)

### Màn 13 — 6-switch vault

Phòng công tắc 6 bit độc lập; phải mở đủ các cầu trên hành lang dẫn tới đích.

Lời giải tối ưu **66 bước**: [file phát lại](solutions/13.txt).

```text
U U U U U U U L U L L L L D D D D D D D L D R R R R R R R R R R U U U U U U U U L D L L L U L U U U U U U U U U U U U U U U U U U U
```

BFS/A* `h=0` mở rộng **47,675 node** và gặp đủ **64 cấu hình cầu**. Bỏ riêng từng công tắc thì màn vô nghiệm.

![Sơ đồ màn 13](previews/13.png)

### Màn 14 — Amber labyrinth

Mê cung nhiều ô dễ vỡ: vừa chọn đường qua hốc trống, vừa tránh dựng đứng trên ô cam.

Lời giải tối ưu **36 bước**: [file phát lại](solutions/14.txt).

```text
R U R U R R R U U L U L U U R U L U L U U R R U U U U U R U R U R R R R
```

Có **200 ô dễ vỡ**; thay chúng bằng sàn thường làm tối ưu giảm từ **36** xuống **25 bước**. Lời giải thực sự đi qua nhiều ô cam ở tư thế nằm.

![Sơ đồ màn 14](previews/14.png)

### Màn 15 — Dual bridges crossing

Màn v2 của Đạt: công tắc thường `(4,8)` bật/tắt **hai** ô cầu `(4,14)`,
`(5,14)` cùng lúc; công tắc `(2,14)` là **permanent on** (bấm lại không đóng
cầu `(6,8)`). Lời giải tối ưu **37 bước**: [file phát lại](solutions/15.txt).

![Sơ đồ màn 15](previews/15.png)

### Màn 16 — The great split

Màn v2 của Đạt dựng theo Stage 10 của Bloxorz gốc: đứng lên split switch
`(3,14)` thì khối tách thành hai khối 1×1×1 ở `(3,11)` và `(3,14)`; điều
khiển từng khối (`0U`… / `1R`… trong file lời giải, phím Space trong game)
tới khi hai khối kề nhau thì tự gộp lại. Lời giải tối ưu **49 bước**:
[file phát lại](solutions/16.txt).

![Sơ đồ màn 16](previews/16.png)

## Kiểm chứng và dựng lại hình

Từ thư mục gốc repository:

```bash
python -m pytest tests/test_levels.py tests/test_hard_levels.py -q
python -m pytest -q
python -m pip install -r scripts/requirements-previews.txt
python scripts/render_level_previews.py
```

Test kiểm tra danh mục/định dạng, lời giải thắng đúng ở bước cuối, tối ưu
theo BFS độc lập, viền 2 ô trống của màn mới, cầu bắt buộc, ảnh PNG hợp lệ
và tác dụng của ô dễ vỡ. Pillow chỉ phục vụ dựng lại hình; các file ảnh
đã được lưu sẵn, solver và pytest không cần cài Pillow.

Màn 1–10 đã được Kiên phát lại và xác nhận thắng trong game C++ theo
[report3](../../Docs/report3.md). Ba màn khó 12–14 đã kiểm chứng mô phỏng
Python, chờ nhóm nghiệm thu phát lại C++; xem [thiết kế màn khó](../../Docs/hard_levels.md).

## Nguồn và phụ lục sử dụng AI

Màn 1–2 lấy từ game nền `chaitanya100100/Bloxorz`, giữ nguyên theo
[THIRD_PARTY_NOTICES](../../THIRD_PARTY_NOTICES.md). Màn 3–14 được thiết kế
cho bộ test của nhóm với hỗ trợ ChatGPT/Codex, dựa trên luật đã có trong repo.
Prompt chính: **‘Thiết kế thêm màn chơi theo phân công đầy đủ TV4’**.
Ghi thông tin này vào phụ lục sử dụng AI chung của nhóm và review lại
màn chơi, lời giải, số liệu trước khi đưa vào báo cáo chính thức.

Bổ sung màn khó 12–14 theo prompt **‘Làm thêm màn khó theo góp ý của Kiên’**;
thiết kế và số liệu node được giải thích trong `Docs/hard_levels.md`.
