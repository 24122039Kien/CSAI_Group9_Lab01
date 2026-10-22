# Cách chạy game và solver trong VS Code (Windows)

> Dành cho cả nhóm. Game 3D được build và chạy qua **WSL Ubuntu**, nhưng mọi lệnh đều gõ được ngay trong **terminal PowerShell của VS Code**: chỉ cần thêm chữ `wsl` phía trước. Cửa sổ game hiện lên trên desktop Windows như một ứng dụng bình thường (nhờ WSLg của Windows 11).
>
> Phần Python (`main.py`, `pytest`) thì chạy thẳng trên Windows, không cần WSL.
>
> **Muốn chơi mượt:** WSLg vẽ bằng CPU (`llvmpipe`, khoảng 40 FPS). Bản Windows native chạy bằng GPU thật (144 FPS trên màn 144 Hz) — xem mục 5 hoặc dùng bản đóng gói `Bloxorz.exe` (`dist/Bloxorz-win64.zip`). Chi tiết: [report2.md](report2.md).

---

## 1. Cài đặt một lần duy nhất

Mở **PowerShell** (hoặc terminal của VS Code) và chạy:

```powershell
wsl --install -d Ubuntu      # bỏ qua nếu máy đã có Ubuntu (kiểm tra bằng: wsl -l -v)
```

Khởi động lại máy nếu được yêu cầu, rồi cài thư viện cho game bên trong Ubuntu:

```powershell
wsl sudo apt update
wsl sudo apt install -y g++ make libglfw3-dev libglm-dev libgl-dev python3 python3-pytest
```

Phần Python trên Windows cần **Python ≥ 3.10**:

```powershell
pip install -r requirements.txt
```

---

## 2. Chạy game

Mở terminal trong VS Code (`` Ctrl+` ``), rồi:

```powershell
cd "D:\study_file\Cơ sở AI\Lab\CSAI_Group9_Lab01\code"    # đổi theo chỗ bạn clone repo
wsl make            # build game — chỉ cần khi mới clone hoặc vừa sửa code C++
wsl ./a.out 1       # mở game màn 1
```

> **Luôn đứng trong thư mục `code`** khi chạy game, vì game đọc `data/<số>.data` và file shader theo đường dẫn tương đối.

### Các cách chạy

| Lệnh | Ý nghĩa |
|---|---|
| `wsl ./a.out` | Chơi màn mặc định (màn 2) |
| `wsl ./a.out 1` | Chơi màn 1 bằng tay |
| `wsl ./a.out 1 --play sol1.txt` | Khối **tự lăn** theo file lời giải |
| `wsl ./a.out 2 --solve bfs` | Gọi solver Python (`bfs`, `dfs`, `ucs`, `astar`) rồi khối tự lăn tới đích |
| `wsl ./a.out --help` | Xem hướng dẫn |

- Không có `--play`/`--solve`: game mở **menu** (chọn màn, nút Solve BFS/DFS/UCS/A\*, tùy chọn); `wsl ./a.out` bắt đầu ở màn 1.

- Với `--play`/`--solve`: khi thắng/thua, terminal in `WON` hoặc `GAME OVER` kèm số bước, cửa sổ giữ 2 giây rồi tự đóng.
- `--solve` sẽ báo **"chưa cài"** cho tới khi thuật toán tương ứng được nhóm cài xong — bình thường.

### Thử nhanh chế độ tự lăn

Tạo file `sol1.txt` trong thư mục `code` với nội dung:

```
R U U R U R U R D L
```

rồi chạy:

```powershell
wsl ./a.out 1 --play sol1.txt      # khối tự lăn 10 bước → WON, Steps : 10
```

File lời giải là các nước `U`, `D`, `L`, `R` cách nhau bởi khoảng trắng (chính là định dạng mà `main.py --out` ghi ra).

### Phím điều khiển

| Phím | Tác dụng |
|---|---|
| ↑ ↓ ← → | Lăn khối (bị khóa khi đang tự phát lại) |
| `C` | Camera đi theo khối (mặc định) |
| `V` | Nhìn từ trên xuống |
| `B` | Nhìn từ tháp (góc chéo) |
| `X` | Camera trực thăng — di chuyển bằng `W A S D`, xoay bằng `I J K L`, zoom `T G` |
| `N` | Kéo chuột để xoay, cuộn chuột để zoom |
| `M` | Camera xoay tự do theo chuột |
| `Space` | Đổi phép chiếu |
| `R` / `Enter` | Chơi lại / sang màn tiếp (sau khi thắng) |
| `Esc` | Mở/đóng menu (chế độ `--play`/`--solve`: thoát) |
| `Tab` | Bật/tắt bản đồ nhỏ |
| `F1` / `F12` | Ẩn/hiện bảng phím / chụp màn hình |
| `Q` | Thoát |

---

## 3. Chạy phần Python (trên Windows, không cần `wsl`)

Đứng ở **thư mục gốc repo** (`CSAI_Group9_Lab01`):

```powershell
python main.py code/data/1.data --algo bfs                  # giải màn 1 bằng BFS
python main.py code/data/2.data --algo astar --out sol.txt  # ghi lời giải ra file
python main.py code/data/1.data --compare --memory          # so sánh cả 4 thuật toán
python -m pytest -q                                         # chạy test
```

Muốn xem lời giải đó trong game: chép `sol.txt` vào thư mục `code` rồi `wsl ./a.out 2 --play sol.txt`.

---

## 4. Cách khác: mở cả thư mục trong WSL

Nếu làm lâu với code C++, tiện hơn là để VS Code chạy hẳn trong Ubuntu:

1. Cài extension **WSL** (của Microsoft) trong VS Code.
2. Bấm `F1` → gõ **WSL: Open Folder in WSL** → chọn thư mục repo.
3. Lúc này terminal của VS Code là Ubuntu, gõ lệnh **không cần** chữ `wsl`:

```bash
cd code
make
./a.out 1
```

---

## 5. Chạy bản Windows native (mượt nhất)

Cài **MSYS2** (https://www.msys2.org hoặc `winget install MSYS2.MSYS2`), mở **MSYS2 MinGW x64** rồi:

```bash
pacman -S --needed make zip mingw-w64-x86_64-gcc mingw-w64-x86_64-glfw mingw-w64-x86_64-glm
cd "/d/study_file/Cơ sở AI/Lab/CSAI_Group9_Lab01/code"
make && ./a.exe            # bản dev (có console)
make dist                  # Bloxorz.exe + ../dist/Bloxorz-win64.zip để gửi cho người khác
```

- Build WSL và MSYS2 dùng chung thư mục được: object nằm riêng ở `build/linux/` và `build/win64/`.
- Laptop 2 GPU: game tự xin GPU rời (NVIDIA/AMD); HUD góc trên trái hiện tên GPU đang dùng.
- Nút Solve trong bản native gọi `python` (hoặc `py -3`) của Windows; đổi bằng biến `BLOXORZ_PYTHON`.

---

## 6. Lỗi thường gặp

| Lỗi | Nguyên nhân | Cách sửa |
|---|---|---|
| `cd : Cannot find path 'D:\mnt\d\...'` | Gõ đường dẫn Linux (`/mnt/d/...`) trong **PowerShell** | Dùng đường dẫn Windows `cd "D:\..."` rồi thêm `wsl` trước lệnh; hoặc gõ `wsl` trước để vào Ubuntu rồi mới `cd "/mnt/d/..."` |
| `Cannot open data/1.data (run the game from the code/ directory)` | Không đứng trong thư mục `code` | `cd` vào thư mục `code` rồi chạy lại |
| `make: command not found` / `GLFW/glfw3.h: No such file` | Chưa cài thư viện trong Ubuntu | Chạy lại lệnh `wsl sudo apt install ...` ở mục 1 |
| `wsl: command not found` hoặc không có Ubuntu | Máy chưa cài WSL | `wsl --install -d Ubuntu` (mục 1) |
| Cảnh báo `MESA` / `libEGL` / `ZINK` khi mở game | Driver đồ họa của WSL chuyển sang chế độ phần mềm | Vô hại, bỏ qua |
| `--solve` in "chưa cài (TODO)" | Thuật toán đó chưa được nhóm cài | Bình thường; dùng `--play` với file lời giải có sẵn để thử |
| Không thấy cửa sổ game | Windows 10 cũ không có WSLg | Cập nhật Windows 11 / WSL (`wsl --update`) |
