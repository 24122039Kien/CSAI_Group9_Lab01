# Cách chạy game và solver trong VS Code (Windows)

> Dành cho cả nhóm. Mọi lệnh dưới đây gõ ngay trong **terminal của VS Code** (`` Ctrl+` ``, mặc định là PowerShell), không cần mở cửa sổ MSYS2 hay Ubuntu riêng.
>
> - **Phần Python** (`main.py`, `pytest`): chỉ cần Python.
> - **Game C++**: build bằng bộ công cụ **MSYS2** (cung cấp `g++`, `make` và thư viện GLFW), chạy thẳng trên Windows bằng GPU thật nên rất mượt.
> - **Chỉ muốn chơi thử**: giải nén `dist/Bloxorz-win64.zip` rồi mở `Bloxorz.exe`, không cần cài gì.
>
> Các tính năng mới (Pause, bảng AI, Search Visualizer, âm thanh, theme Minecraft): xem [report4.md](report4.md).
> Đợt nâng cấp đồ họa (bóng đổ thật, bloom, theme **Original** kiểu game gốc, menu chính mới, camera xoay/zoom bằng chuột): xem mục "Trong màn chơi" và "Phím điều khiển" bên dưới.

---

## 1. Chọn cách chạy

| Bạn muốn | Cần cài | Xem mục |
|---|---|---|
| Chơi thử, không sửa code | Không cần gì (dùng `Bloxorz.exe`) | 7 |
| Chạy solver Python, chạy test | Python ≥ 3.10 | 2.1, 5 |
| Sửa code C++ rồi build lại | Python + MSYS2 | 2, 3 |
| Build bản Linux | WSL Ubuntu | 6 |

---

## 2. Cài đặt một lần duy nhất

### 2.1. Python

Cài **Python ≥ 3.10** (nhớ tích "Add python.exe to PATH"), rồi ở thư mục gốc repo:

```powershell
pip install -r requirements.txt      # chỉ có pytest; phần AI dùng thư viện chuẩn
```

### 2.2. MSYS2 (để build game)

**Vì sao cần MSYS2?** Terminal VS Code chỉ là nơi gõ lệnh, bản thân nó không có trình biên dịch. Game cần `g++` 64-bit đủ mới (có `std::thread` cho âm thanh), thư viện **GLFW** để mở cửa sổ OpenGL, và `make` cùng các lệnh `uname`, `mkdir`, `cp` mà makefile dùng. MSYS2 cung cấp đủ cả ba.

1. Cài MSYS2: `winget install MSYS2.MSYS2` (hoặc tải ở https://www.msys2.org), mặc định vào `C:\msys64`.
2. Cài gói, chỉ làm một lần. Lệnh này cần mở **MSYS2 MinGW x64** từ Start menu:

   ```bash
   pacman -S --needed make zip mingw-w64-x86_64-gcc mingw-w64-x86_64-glfw mingw-w64-x86_64-glm
   ```

### 2.3. Cho terminal VS Code thấy MSYS2

Mở `Ctrl+Shift+P` → **Preferences: Open User Settings (JSON)**, thêm vào trong cặp `{ }`:

```json
"terminal.integrated.env.windows": {
  "PATH": "C:\\msys64\\mingw64\\bin;C:\\msys64\\usr\\bin;${env:PATH}"
},
```

Đóng terminal cũ, mở terminal mới (`` Ctrl+` ``) rồi kiểm tra:

```powershell
(Get-Command g++).Source      # phải ra C:\msys64\mingw64\bin\g++.exe
make --version                # GNU Make 4.x
```

- Hai thư mục MSYS2 phải đứng **đầu** PATH. Nếu máy có sẵn một bộ `g++` khác (ví dụ MinGW.org 32-bit cũ trong `D:\bin` hay `C:\MinGW`), nó sẽ bị dùng nhầm và build lỗi.
- Cấu hình này chỉ áp dụng cho terminal trong VS Code, không đổi PATH của cả Windows.
- Không muốn sửa settings thì gõ dòng sau mỗi lần mở terminal mới:

  ```powershell
  $env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH
  ```

---

## 3. Chạy game

```powershell
cd "D:\study_file\Cơ sở AI\Lab\CSAI_Group9_Lab01\code"    # đổi theo chỗ bạn clone repo
make              # build a.exe: cần khi mới clone, vừa pull, hoặc vừa sửa code C++
.\a.exe 1         # mở game màn 1
```

> **Luôn đứng trong thư mục `code`** khi chạy game, vì game đọc `data/<số>.data` theo đường dẫn tương đối.
>
> **Sau mỗi lần `git pull`** nhớ `make` lại: nhóm thường thêm file `.cpp` mới (ví dụ `audio.cpp`, `viz.cpp`).

### Các cách chạy

| Lệnh | Ý nghĩa |
|---|---|
| `.\a.exe` | Mở **màn hình chính**: Play / Select Stage / Options / How to Play / Credits |
| `.\a.exe 5` | Vào thẳng màn 5 để chơi bằng tay |
| `.\a.exe 1 --play data/solutions/1.txt` | Khối **tự lăn** theo file lời giải |
| `.\a.exe 15 --solve astar` | Gọi solver Python (`bfs`, `dfs`, `ucs`, `astar`) rồi khối tự lăn tới đích |
| `.\a.exe 5 --visualize bfs` | Mở thẳng **Search Visualizer** của BFS ở màn 5 |
| `.\a.exe 1 --theme original` | Mở với theme Original (kiểu game gốc); còn `classic`, `minecraft`. Đổi được trong Options |
| `.\a.exe 1 --no-shadows --no-bloom` | Tắt bóng đổ thật / bloom (máy yếu) |
| `.\a.exe 1 --mute` | Tắt âm thanh |
| `.\a.exe --help` | Xem tất cả tùy chọn |

- Với `--play`/`--solve`: khi thắng/thua, terminal in `WON` hoặc `GAME OVER` kèm số bước, cửa sổ giữ 2 giây rồi tự đóng. Thêm `--keep-open` để phát lại như chơi bình thường (rơi thì chơi lại, không tự thoát).
- Lời giải có sẵn cho 15 màn nằm ở `data/solutions/<số>.txt`. Màn 11 cố ý vô nghiệm.
- File lời giải là các nước `U D L R` cách nhau bởi khoảng trắng, đúng định dạng `main.py --out` ghi ra. Màn có split dùng thêm nước dạng `0U`, `1R`: số đầu là khối nhỏ thứ mấy.

### Trong màn chơi

- **Màn hình chính:** logo, menu bên trái (chuột hoặc `↑ ↓` + `Enter`), bên phải là nội dung trang. Phía sau là màn chơi với camera tự quay chậm.
  - **Play**: vào màn chưa qua tiếp theo (có bản đồ xem trước và thanh tiến độ).
  - **Select Stage**: thẻ cho từng màn (nhóm độ khó, kỷ lục số bước), rê chuột để xem bản đồ màn.
  - **Options**: chọn theme bằng thẻ hình, bật/tắt bóng đổ, bloom, hiệu ứng, âm lượng.
  - **How to Play**: phím điều khiển và chú giải từng loại ô. **Credits**: nguồn gốc, thư viện.
  - `Esc` ở trang con quay về trang đầu.
- **Thanh trên cùng:** **Menu** (về màn hình chính), **Restart**, **Pause**. Khi vào màn, tên màn hiện lớn giữa màn hình (như game gốc).
- **Kỷ lục:** tự chơi thắng thì số bước ít nhất được lưu (Best), màn đã qua hiện "cleared" trong Select Stage.
- **Bảng AI** bên phải có các phần:
  - Nút **BFS / DFS / UCS / A\***: chạy solver nền rồi khối tự lăn.
  - Ô chọn heuristic cho A\* (`manhattan` hoặc `zero`).
  - Nút **Visualize search**.
  - 4 chỉ số của lần chạy gần nhất: thời gian, bộ nhớ, số node mở rộng, độ dài lời giải.
- **Search Visualizer:** xem cây tìm kiếm hoặc đồ thị trạng thái chạy từng bước, kèm g/h/f. Trên bàn chơi 3D có bản đồ nhiệt và khối mờ. Màn lớn chỉ ghi 4000 bước mở rộng đầu tiên.
- **Rơi khỏi sàn:** sàn sụp theo, khoảng 1,6 giây sau màn tự chơi lại (như Bloxorz gốc). Bảng góc trái đếm số lần rơi (FALLS).
- **Theme:** **Classic** (nền không gian tối), **Original** (trời đỏ cam có khói, ô đá xám, khối sắt gỉ, công tắc kim loại nổi O / X / ( ), như Bloxorz gốc nhưng vẽ hoàn toàn bằng shader) và **Minecraft**.
- **Đồ họa:** bóng đổ thật (shadow map), cạnh vát bắt sáng, phản chiếu bầu trời trên khối, bloom (vùng sáng tỏa quầng). Cầu nháy xanh khi mở, nháy đỏ khi đóng. Máy dùng driver vẽ bằng CPU (WSLg) tự tắt bóng đổ và bloom để giữ FPS.
- **Pause → Options:** đổi theme, bật/tắt bóng đổ, bloom, hiệu ứng, chỉnh âm lượng. Cài đặt và kỷ lục lưu vào `settings.ini`.
- **Âm thanh:** tổng hợp bằng code. Muốn thay thì đặt `sounds/<tên>.wav|.mp3|.flac` trong thư mục chạy game (bản dev: `code/sounds/`, bản đóng gói: cạnh `Bloxorz.exe`; tên: `roll stand switch heavy bridge_on bridge_off fall win split merge crack click start collapse music`).

### Phím điều khiển

| Phím | Tác dụng |
|---|---|
| ↑ ↓ ← → | Lăn khối (bị khóa khi đang tự phát lại) |
| `Space` | Sau khi tách (split): đổi khối nhỏ đang điều khiển. Chưa tách: đổi phép chiếu |
| `R` | Chơi lại |
| `P` / `Esc` | Tạm dừng (`Esc` cũng đóng màn hình chính / Visualizer; ở chế độ `--play`/`--solve` thì thoát) |
| `Enter` | Sang màn tiếp (chỉ sau khi thắng) |
| `C` | Camera bám khối (mặc định): chỉ trượt theo khối, hướng nhìn giữ nguyên nên không lắc |
| `B` | Nhìn toàn màn từ trên cao, lăn chuột để zoom gần / xa |
| `N` | Xoay tự do quanh màn: kéo chuột trái để xoay, kéo chuột phải hoặc `W A S D` để dời điểm nhìn |
| `M` | Góc nhìn đi theo con trỏ chuột (không cần bấm), xoay quanh khối |
| `X` | Bay tự do: di chuyển `W A S D`, lên/xuống theo hướng nhìn `T G`, xoay `I J K L` hoặc kéo chuột |
| `V` | Nhìn thẳng từ trên xuống (phép chiếu song song), lăn chuột để zoom |
| Kéo chuột / lăn chuột | Ở mọi góc nhìn 3D: kéo để xoay quanh, lăn để zoom. Góc đã chỉnh được giữ khi đổi qua lại |
| `Tab` | Bật/tắt bản đồ nhỏ |
| `F1` / `F12` | Ẩn/hiện bảng phím / chụp màn hình |
| `Q` | Thoát |

**Khi Search Visualizer đang mở:** `←` / `→` lùi/tiến một bước, `Space` chạy/dừng. Kéo chuột để di chuyển khung cây, cuộn để zoom.

---

## 4. Build bản phát hành và đóng gói

Vẫn trong thư mục `code`:

```powershell
make release      # Bloxorz.exe: tự chứa (không cần DLL), không mở console, có icon
make dist         # ../dist/win64/Bloxorz/ + ../dist/Bloxorz-win64.zip để gửi cho người khác
```

- `a.exe` (bản dev) cần `glfw3.dll` trong `C:\msys64\mingw64\bin`, nên chỉ chạy được trong terminal đã cấu hình ở mục 2.3. Gửi cho người khác thì dùng `Bloxorz.exe` hoặc file zip.
- Laptop 2 GPU: game tự xin GPU rời (NVIDIA/AMD). Bảng góc trên trái hiện tên GPU đang dùng.
- Nút Solve gọi `python` (hoặc `py -3`) của Windows. Muốn đổi thì đặt biến `BLOXORZ_PYTHON`.

---

## 5. Chạy phần Python

Đứng ở **thư mục gốc repo** (`CSAI_Group9_Lab01`):

```powershell
python main.py code/data/1.data --algo bfs                          # giải màn 1 bằng BFS
python main.py code/data/15.data --algo astar --out sol.txt         # A* + Manhattan (mặc định), ghi lời giải
python main.py code/data/15.data --algo astar --heuristic zero      # A* với h = 0 để so sánh
python main.py code/data/1.data --compare --memory                  # bảng so sánh các thuật toán
python main.py code/data/5.data --algo bfs --trace trace.txt --stats stats.txt   # vết tìm kiếm + chỉ số (game dùng)
python -m pytest -q                                                 # chạy test: 220 passed, 1 skipped
```

Muốn xem lời giải đó trong game: chép `sol.txt` vào thư mục `code` rồi `.\a.exe 15 --play sol.txt`.

Cả 4 thuật toán đã được cài. `DFS` cho lời giải hợp lệ nhưng thường dài hơn nhiều (ví dụ màn 15: 81 bước so với 37), vì DFS không đảm bảo tối ưu. `UCS` cho cùng độ dài với `BFS` và `A*`, vì chi phí mỗi bước hiện là 1. Test duy nhất bị skip là test dành cho khung cũ khi BFS chưa cài.

---

## 6. Cách khác: build bản Linux bằng WSL

Không bắt buộc, chỉ dùng khi cần bản Linux (`make dist` ra `Bloxorz-linux.zip`). Cửa sổ game của WSL vẽ bằng CPU, chỉ khoảng 40 FPS.

```powershell
wsl --install -d Ubuntu                    # bỏ qua nếu đã có (kiểm tra: wsl -l -v)
wsl sudo apt update
wsl sudo apt install -y g++ make zip libglfw3-dev libglm-dev libgl-dev python3 python3-pytest
cd "D:\study_file\Cơ sở AI\Lab\CSAI_Group9_Lab01\code"
wsl make
wsl ./a.out 1
```

- Build WSL và MSYS2 dùng chung thư mục được, vì file object nằm riêng ở `build/linux/` và `build/win64/`.
- Làm lâu trong Ubuntu thì cài extension **WSL** cho VS Code, rồi chọn `F1` → **WSL: Open Folder in WSL**. Khi đó gõ lệnh không cần chữ `wsl` phía trước.

---

## 7. Chỉ chơi thử (không cài gì)

Giải nén `dist/Bloxorz-win64.zip` rồi mở `Bloxorz/Bloxorz.exe`. Các nút solver cần máy có Python ≥ 3.10; không có Python thì vẫn chơi bằng tay bình thường.

---

## 8. Lỗi thường gặp

| Lỗi | Nguyên nhân | Cách sửa |
|---|---|---|
| `make : The term 'make' is not recognized` | Terminal chưa thấy MSYS2 | Làm mục 2.3 rồi **mở terminal mới** |
| Lỗi lạ khi build (`std::thread`, `-std=c++11`, `cannot find -lglfw3`) | Đang dùng nhầm một `g++` khác | `(Get-Command g++).Source` phải ra `C:\msys64\mingw64\bin`; đưa MSYS2 lên đầu PATH |
| `glfw3.h: No such file` | Chưa cài gói GLFW | Chạy lại lệnh `pacman -S ...` ở mục 2.2 |
| `a.exe` không mở, báo thiếu `glfw3.dll` (hoặc mở xong tắt ngay) | Chạy `a.exe` ngoài terminal đã cấu hình | Chạy trong terminal VS Code đã làm mục 2.3, hoặc dùng `make release` → `Bloxorz.exe` |
| `undefined reference to audio_...` / `viz_...` sau khi pull | Thiếu object của file mới | `make` lại; nếu vẫn lỗi thì `make clean` rồi `make` |
| `Cannot open data/1.data (run the game from the code/ directory)` | Không đứng trong thư mục `code` | `cd` vào `code` rồi chạy lại |
| Nút Solve báo `Python not found` | Game không tìm thấy Python | Cài Python ≥ 3.10 có PATH, hoặc đặt `BLOXORZ_PYTHON` |
| Không có tiếng | Máy không có thiết bị âm thanh, hoặc âm lượng bằng 0 | Kiểm tra Pause → Options; game vẫn chạy bình thường khi không có âm thanh |
| `cd : Cannot find path 'D:\mnt\d\...'` | Gõ đường dẫn Linux (`/mnt/d/...`) trong PowerShell | Dùng đường dẫn Windows `cd "D:\..."` |
