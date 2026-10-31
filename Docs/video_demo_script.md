# Kịch bản video demo: Bloxorz – AI Search Algorithms

> Dành cho cả nhóm khi quay và dựng video nộp kèm `Report.pdf` (mục 3.7 của đề, **10 điểm**).
> Thời lượng dự kiến: **8–9 phút**. Thuyết minh tiếng Việt, có phụ đề.

---

## 1. Đề chấm gì, video đáp ứng ở đâu

Đề yêu cầu (trang 7, mục 3.7): video đăng YouTube ở chế độ công khai, link ghi trong báo cáo, **có phụ đề hoặc thuyết minh**.

| Tiêu chí của đề | Điểm | Cảnh |
|---|---:|---|
| Giao diện: bàn chơi với các loại ô (sàn, ô trống, đích, ô dễ vỡ, công tắc/cầu) | 2,5 | Cảnh 3 (màn 1 có đủ mọi loại ô) + bảng chú giải ở Cảnh 2 |
| Giao diện: khối ở các tư thế | (trong 5) | Cảnh 4 (đứng, nằm dọc, nằm ngang) |
| Giao diện: các nút điều khiển (New Game, Restart, Solve BFS/DFS/UCS/A\*) | (trong 5) | Cảnh 2 (Play, Select Stage), Cảnh 3 (Menu, Restart, Pause, bảng AI) |
| Tính năng: chơi tay (lăn khối, kích công tắc, thắng, thua) | 2,5 | Cảnh 4–6 |
| Tính năng: chạy solver (chọn màn, chạy BFS/UCS/A\*, phát lại lời giải) | 2,5 | Cảnh 7–9 |
| (Thêm, không bắt buộc) So sánh thuật toán, Search Visualizer, theme | – | Cảnh 10–12 |

> Năm cảnh đầu đủ cho 10 điểm. Phần "thêm" là điểm nhấn của nhóm; nếu video quá dài thì cắt Cảnh 12 trước, rồi đến Cảnh 11.

---

## 2. Các màn được chọn và lý do

| Màn | Tên | Dùng để | Vì sao chọn |
|---:|---|---|---|
| 1 | Upstream level 1 | Giới thiệu các loại ô | Một màn nhỏ có **đủ mọi loại ô**: sàn, ô dễ vỡ, 2 công tắc thường, 1 công tắc nặng, cầu, đích |
| 3 | First rolls | Tư thế khối + thắng | 4 nước `↑↑↑↑` là thấy đủ chu kỳ đứng → nằm → đứng, rồi thắng |
| 5 | Fragile detour | Thua vì ô dễ vỡ, rồi thắng | `↑↑` là dựng đứng trên ô cam và rơi; lời giải thật phải đi vòng (6 nước). A\* ở màn này chỉ mở rộng 14 node so với 52 của UCS |
| 7 | Soft switch crossing | Kích **công tắc thường** | Nước thứ 3 (khối đang nằm) đè lên công tắc, cầu mở |
| 8 | Heavy switch crossing | Kích **công tắc nặng** (tùy chọn) | Phải **dựng đứng** mới kích được, khác với màn 7 |
| 15 | Dual bridges crossing | Chạy BFS / A\* / DFS rồi phát lại | Kích thước vừa: BFS và A\* cùng ra 37 bước, DFS ra 81 bước, thấy ngay DFS không tối ưu |
| 16 | The great split | Solver giải màn tách khối | Tính năng khó nhất: tách thành 2 khối nhỏ rồi nhập lại (49 bước) |
| 11 | Disconnected islands | Màn vô nghiệm | Cả 4 thuật toán đều báo "No solution" sau khi duyệt hết 113 trạng thái |
| 13 | 6-switch vault | So sánh trên màn lớn | 47.673 trạng thái; DFS ra lời giải **4.295 bước** so với 66 bước tối ưu |

Không dùng: màn 2 (lời giải 68 bước, quá dài để quay), màn 10 (sân trống, không có gì để giải thích), màn 12 và 14 (tương tự màn 13 và 5 nhưng dài hơn).

---

## 3. Chuẩn bị trước khi quay

- [ ] `git pull`, rồi `cd code` và `make release` (hoặc dùng `Bloxorz.exe` trong `dist/`). Máy quay phải có **Python ≥ 3.10**, vì các nút solver gọi Python.
- [ ] **Sao lưu rồi xóa** `code/settings.ini`, để video bắt đầu từ trạng thái "chưa qua màn nào" và không hiện kỷ lục cũ. Quay xong thì chép lại file cũ.
- [ ] Mở game ở **1920×1080**, cửa sổ toàn màn hình, theme **Minecraft**. Bóng đổ và bloom bật.
- [ ] OBS: ghi 1080p 60 fps, ghi riêng âm thanh game và micro. Hạ nhạc game xuống khoảng 30% trong Options để không lấn giọng.
- [ ] Chạy thử từng màn một lần theo đúng phím trong kịch bản. Phím mũi tên **không đổi theo góc camera**: `↑` luôn là nước `U`.
- [ ] Bảng so sánh ở màn 13 mất khoảng **20 giây**. Khi dựng thì cắt đoạn chờ, hoặc chèn chữ "đang chạy… (đã tua)".
- [ ] Bật bảng phím góc phải (`F1`) để người xem thấy phím đang dùng.

---

## 4. Kịch bản chi tiết

Cột **"Thao tác"** là việc người quay làm trên máy. Cột **"Thuyết minh / phụ đề"** là lời đọc, đồng thời dùng làm phụ đề.

### Cảnh 1 – Mở đầu (0:00 – 0:25)

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn hình tiêu đề tĩnh: tên đồ án, môn CSC14003, nhóm 9, tên các thành viên (chèn khi dựng). | "Xin chào thầy cô và các bạn. Đây là đồ án Project 1 môn Cơ sở Trí tuệ Nhân tạo của nhóm 9: trò chơi Bloxorz kết hợp bốn thuật toán tìm kiếm BFS, DFS, UCS và A\*." |
| Cắt sang game đang ở màn hình chính, camera tự quay chậm phía sau. | "Game được viết bằng C++ và OpenGL; phần trí tuệ nhân tạo viết bằng Python. Mục tiêu là lăn khối 1×1×2 rơi đúng vào lỗ đích." |

### Cảnh 2 – Màn hình chính (0:25 – 1:10)

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Rê chuột lần lượt qua các mục bên trái: **Play**, **Select Stage**, **Options**, **How to Play**, **Credits**. | "Màn hình chính có năm mục. Play vào màn chưa qua tiếp theo. Select Stage để chọn bất kỳ màn nào trong 16 màn." |
| Bấm **Select Stage**, rê chuột qua vài thẻ màn để hiện bản đồ xem trước. | "Mỗi thẻ cho biết nhóm độ khó, và khi rê chuột sẽ hiện bản đồ của màn." |
| Bấm **How to Play**, dừng 3 giây ở phần chú giải ô. | "Trang How to Play giải thích phím điều khiển và từng loại ô: sàn thường, ô dễ vỡ màu cam, công tắc thường hình tròn, công tắc nặng hình chữ X, công tắc tách khối, cầu và lỗ đích." |

### Cảnh 3 – Giao diện trong màn chơi (1:10 – 2:15) – màn 1

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Select Stage → màn **1**. Đợi tên màn hiện giữa màn hình rồi mờ đi. Bấm `B` để nhìn toàn màn. | "Màn 1 tuy nhỏ nhưng có đủ mọi loại ô." |
| Dùng chuột chỉ (hoặc chèn mũi tên khi dựng) lần lượt: ô sàn, khoảng trống, **ô cam dễ vỡ**, **2 công tắc tròn**, **công tắc X**, các **ô cầu** đang đóng, **lỗ đích**. | "Đây là sàn. Đây là khoảng trống: khối lăn ra ngoài sẽ rơi. Ô cam dễ vỡ: khối đứng thẳng trên đó sẽ làm sập ô. Công tắc tròn kích được khi bất kỳ phần nào của khối đè lên; công tắc X chỉ kích khi khối đứng thẳng. Đây là các ô cầu, đóng mở theo công tắc. Còn đây là lỗ đích." |
| Chỉ vào **thanh trên cùng**: Menu, Restart, Pause. | "Thanh trên cùng có Menu để về màn hình chính, Restart để chơi lại, và Pause." |
| Chỉ vào **bảng trái**: Level, Moves, Falls, Mode, FPS. | "Bảng bên trái hiện màn đang chơi, số nước đã đi, số lần rơi và chế độ chơi." |
| Chỉ vào **bảng AI bên phải**: 4 nút BFS, DFS, UCS, A\*; ô chọn heuristic; Visualize search; Compare all 4 algorithms. | "Bảng AI bên phải có bốn nút giải bằng BFS, DFS, UCS và A\*, ô chọn heuristic cho A\*, nút xem quá trình tìm kiếm và nút so sánh cả bốn thuật toán." |
| Bấm `Tab` (bật bản đồ nhỏ), rồi lần lượt `C`, `V`, `N` (kéo chuột xoay), về lại `C`. | "Phím Tab bật bản đồ nhỏ. Có sáu góc camera: bám theo khối, nhìn từ trên cao, nhìn thẳng xuống, xoay tự do… và có thể kéo chuột để xoay, cuộn để zoom ở mọi góc." |

### Cảnh 4 – Lăn khối, các tư thế, thắng màn (2:15 – 2:50) – màn 3

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Select Stage → màn **3**, camera `C`. | "Màn 3 dùng để làm quen cách lăn." |
| Bấm `↑` (dừng) `↑` (dừng) `↑` (dừng). | "Khối đang **đứng**. Lăn một lần thì **nằm dọc**, chiếm hai ô. Lăn tiếp thì lại **đứng**. Trạng thái của bài toán vì vậy gồm cả vị trí lẫn tư thế của khối." |
| Bấm `↑` lần thứ tư → khối rơi vào lỗ, hiện **LEVEL COMPLETE – 4 moves**. | "Khối đứng thẳng đúng trên lỗ đích thì thắng." |
| (Tùy chọn) Bấm `R`, rồi `←` một lần để thấy tư thế **nằm ngang**, rồi `R` lại. | "Lăn sang trái hoặc phải thì khối nằm theo chiều ngang." |

### Cảnh 5 – Thua vì ô dễ vỡ, rồi thắng (2:50 – 3:35) – màn 5

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn **5**. Bấm `↑` `↑`. Khối dựng đứng trên ô cam: ô vỡ, khối rơi, cả sàn sụp theo; số **Falls** ở bảng trái tăng lên 1 và màn tự chơi lại. | "Nếu đi thẳng, khối sẽ dựng đứng trên ô dễ vỡ màu cam: ô vỡ, khối rơi và ta thua. Giống game gốc, cả sàn sụp theo rồi màn tự chơi lại sau khoảng hai giây; bảng bên trái đếm số lần rơi." |
| Bấm `↑` `←` `↑` `↑` `→` `↑` → **LEVEL COMPLETE – 6 moves**, **Best: 6 moves – new record!** | "Phải đi vòng để khối chỉ nằm trên ô cam chứ không dựng đứng trên đó. Sáu nước, cũng là lời giải tối ưu. Game lưu kỷ lục số nước của từng màn." |

### Cảnh 6 – Kích công tắc mở cầu (3:35 – 4:30) – màn 7 (và 8)

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn **7**, bấm `B` để thấy cả hai phòng và cây cầu đang đóng. Về `C`. | "Màn 7: phòng đích bị ngăn bởi một cây cầu đang đóng." |
| Bấm `←` `↑` `↑`. Ở nước thứ 3, khối nằm đè lên công tắc tròn: có tiếng "tách", **cầu nháy xanh và hiện ra**. | "Đè lên công tắc tròn, dù khối đang nằm, cầu sẽ mở." |
| Bấm `↑` `↑` `→` `↑` `↑` `↑` `↑` `↑` `↑` → thắng (12 nước). | "Giờ có thể băng qua cầu để tới đích." |
| *(Tùy chọn, nếu còn thời gian)* Màn **8**: `↑` `←` `←` rồi `↑`. Khối dựng đứng trên công tắc X thì cầu mở. Đi tiếp `↑` `→` `→` `↑` `↑` `↑` `↑` `↑` `←` `↑` `→`. | "Công tắc X là công tắc nặng: chỉ kích được khi khối **đứng thẳng** trên nó." |

### Cảnh 7 – Chạy solver và phát lại lời giải (4:30 – 5:40) – màn 15

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Menu → Select Stage → màn **15**. Bấm `B` để thấy cả màn. | "Giờ đến phần trí tuệ nhân tạo. Chọn màn 15, có hai cây cầu và công tắc mở nhiều cầu cùng lúc." |
| Bấm nút **BFS** trong bảng AI. Khối tự lăn tới đích. Dừng hình ở **4 chỉ số** dưới các nút (thời gian, bộ nhớ, node mở rộng, độ dài). | "Bấm BFS: game gọi solver Python chạy nền, nhận lời giải rồi tự phát lại từng nước. BFS tìm được lời giải 37 bước, là ngắn nhất vì mỗi bước có chi phí bằng 1. Bên dưới là thời gian chạy, bộ nhớ và số node đã mở rộng." |
| Bấm `R`, rồi bấm **A\*** (heuristic: *Manhattan (split-aware)*). | "A\* với heuristic Manhattan cũng cho 37 bước, tức là vẫn tối ưu, nhưng mở rộng ít node hơn." |
| Bấm `R`, rồi bấm **DFS**. Để khối lăn khoảng 10 giây (tua nhanh khi dựng). | "Còn DFS cho lời giải 81 bước: vẫn hợp lệ nhưng dài hơn hẳn, vì DFS không đảm bảo tối ưu." |

### Cảnh 8 – Solver giải màn tách khối (5:40 – 6:20) – màn 16

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn **16**, bấm **A\***. Để phát lại, tua nhanh ×2 khi dựng, dừng lại ở khoảnh khắc khối **tách đôi** và lúc **nhập lại**. | "Màn 16 có công tắc tách khối. Khi khối đứng lên công tắc này, nó tách thành hai khối nhỏ 1×1×1, lần lượt điều khiển từng khối bằng phím Space. Solver vẫn giải được: hai khối nhỏ đi riêng rồi nhập lại thành khối lớn để vào đích, tổng cộng 49 bước." |

### Cảnh 9 – Màn vô nghiệm (6:20 – 6:40) – màn 11

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn **11**, bấm `B`, rồi bấm **BFS**. Dừng ở dòng **"No solution exists for this level"**. | "Màn 11 cố ý không có lời giải: hai đảo không có cầu nối. BFS duyệt hết 113 trạng thái tới được rồi kết luận không có đường tới đích, thay vì chạy mãi." |

### Cảnh 10 – So sánh 4 thuật toán (6:40 – 7:45) – màn 5 và 13

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn **5**, bấm `Q`. Đợi bảng chạy xong (vài giây). Rê chuột qua bảng chỉ số, rồi qua biểu đồ cột. | "Phím Q mở bảng so sánh: game chạy lần lượt cả bốn thuật toán trên màn hiện tại. Ô xanh là giá trị tốt nhất mỗi cột. Ở màn 5, A\* chỉ mở rộng 14 node, trong khi UCS mở rộng 52 node: heuristic giúp A\* đi thẳng về phía đích." |
| Màn **13**, bấm `Q`. Cắt đoạn chờ. Dừng ở cột **Length** và **Expanded**, đọc các dòng nhận xét bên dưới. Bật thang log cho biểu đồ. | "Màn 13 là màn lớn nhất, có sáu công tắc với gần 48 nghìn trạng thái. BFS, UCS và A\* đều cho 66 bước. DFS mở rộng ít node nhất nhưng ra lời giải tới 4.295 bước. Ở màn này A\* gần như không cắt tỉa được, vì hầu hết các trạng thái đều gần điểm xuất phát hơn đích; phần giải thích chi tiết nằm trong báo cáo." |
| Bấm **Replay** ở dòng A\*, cho khối lăn vài giây. Bấm **Save CSV**. | "Có thể phát lại lời giải của từng thuật toán, và xuất bảng ra file CSV để đưa vào phần thực nghiệm." |

### Cảnh 11 – Search Visualizer (7:45 – 8:25) – màn 5 *(có thể cắt)*

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| Màn **5**, bấm **Visualize search**, chọn A\*. Bấm `→` vài lần, rồi `Space` cho chạy tự động. | "Search Visualizer cho xem cây tìm kiếm mở rộng từng bước, kèm giá trị g, h, f của mỗi node. Trên bàn chơi, bản đồ nhiệt cho biết vùng nào đã được duyệt." |
| Đổi sang BFS, chạy lại vài bước. | "So với BFS, A\* tập trung về phía đích thay vì loang đều mọi hướng." |

### Cảnh 12 – Theme và kết thúc (8:25 – 9:00) *(có thể cắt phần theme)*

| Thao tác | Thuyết minh / phụ đề |
|---|---|
| `P` → **Options**, bấm lần lượt thẻ **Classic**, **Original**, **Minecraft**. Mỗi theme dừng 2–3 giây ở màn 15, camera `C`. | "Ngoài ra game có ba giao diện: Classic, Original giống game gốc, và Minecraft. Tất cả đều có bóng đổ thật và hiệu ứng phát sáng." |
| Màn hình kết: link GitHub, tóm tắt (chèn khi dựng). | "Mã nguồn, hướng dẫn cài đặt và báo cáo chi tiết có tại GitHub của nhóm. Cảm ơn thầy cô và các bạn đã theo dõi." |

---

## 5. Gợi ý chia việc

| Phần | Gợi ý người thuyết minh |
|---|---|
| Cảnh 1–6 (giao diện, chơi tay) | Người làm GUI |
| Cảnh 7 (BFS, DFS) | Người làm BFS và DFS |
| Cảnh 7–8 (A\*, heuristic, split) | Người làm A\* và heuristic |
| Cảnh 9–10 (vô nghiệm, so sánh, UCS) | Người làm UCS |
| Dựng video, phụ đề, upload | Một người phụ trách chung |

Mỗi người thu giọng phần của mình. Người dựng ghép giọng vào hình đã quay và làm phụ đề từ cột thuyết minh ở trên.

---

## 6. Lưu ý khi dựng và upload

- **Phụ đề là bắt buộc** theo đề. Dùng luôn cột "Thuyết minh / phụ đề" làm file `.srt`, hoặc phụ đề tự động của CapCut rồi sửa lại các thuật ngữ (BFS, A\*, heuristic).
- Cắt mọi đoạn chờ solver hoặc bảng so sánh; tua ×2 các đoạn phát lại dài (DFS ở màn 15, màn 16).
- Chèn chữ chú thích ngắn khi giới thiệu từng loại ô ở Cảnh 3 (ví dụ "Ô dễ vỡ", "Công tắc nặng"), vì đây là phần đề chấm kỹ nhất.
- Upload YouTube ở chế độ **Public** (đề ghi "public link"), rồi mở link bằng trình duyệt ẩn danh để chắc chắn ai cũng xem được. Dán link vào `Report.pdf`.
- Các số liệu trong lời thoại (37/81 bước ở màn 15, 14/52 node ở màn 5, 66/4.295 bước ở màn 13, 113 trạng thái ở màn 11) lấy từ [report6.md](report6.md). Thời gian chạy (ms) thay đổi theo máy nên không đọc con số cụ thể.
