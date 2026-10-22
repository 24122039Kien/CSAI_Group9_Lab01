# Implementation plan — Nâng cấp GUI & đóng gói game (nhóm 9)

> Kế hoạch nâng cấp đồ họa, hiệu năng và đóng gói game C++ trong `code/`.
> Liên quan: `Docs/implementation_plan.md` §4.3, `Docs/task.md` (S1-07, S3-01, S3-03, S4-03 — phần GUI của TV2).
> Cập nhật: 22/10/2026. **Trạng thái: đã thực hiện xong Phase A–F**. Kết quả, số liệu và khác biệt so với kế hoạch: [report2.md](report2.md).

## 0. Context — vì sao làm và có khả thi không

**Hiện trạng** (đọc từ `code/`):
- Đồ họa rất thô: mọi khối là hộp tô **màu phẳng**, mỗi tam giác có 1 đỉnh màu đen (`color_buffer_data` trong `block.cpp`, `superblock.cpp`) → vệt gradient đen khó coi; không có ánh sáng, không có viền; nền xanh da trời (`SKYBLUE`); **ô đích không được vẽ** (chỉ là lỗ trống); không có HUD, chữ chỉ in ra console.
- Lag có 3 nguyên nhân chính:
  1. **Chạy qua WSLg** → Mesa rơi về render phần mềm (đúng cảnh báo `MESA`/`ZINK` trong `Docs/play.md`) → FPS thấp. Đây là nguyên nhân lớn nhất.
  2. **Animation tính theo frame, không theo thời gian**: `rotation_unit = 5°/frame` (`superblock.cpp:9`), camera `eye += 0.05*(r-eye)` mỗi frame (`camera.cpp:208`) → FPS tụt thì khối lăn chậm, giật.
  3. Mỗi ô là một VAO riêng (`new Block` trong `Tile::initialize`), cả cảnh vẽ **2 lần/frame** (bản đồ nhỏ ở `main.cpp:237`), mỗi lần vẽ gọi `glPolygonMode` + bật attrib; bật/tắt cầu thì `new Tile` → cấp phát VAO/VBO mới giữa lúc chơi.

**Kết luận: khả thi hoàn toàn.** Mọi thay đổi chỉ nằm ở tầng **vẽ / thời gian / đóng gói**, không đụng luật game → không vi phạm quy tắc "luật nằm ở hai nơi" trong `CLAUDE.md`, `ai/` và `tests/` không cần sửa.

**Quyết định đã chốt với người dùng:**
- Build bản Windows native bằng **MSYS2 MinGW64** (makefile hiện có đã có nhánh Windows) → chạy bằng GPU thật.
- Bản đóng gói **yêu cầu máy có Python ≥ 3.10** cho nút Solve (không dùng PyInstaller).

## 1. Ràng buộc bắt buộc

1. **Không đổi luật**: giữ nguyên phần cập nhật `ix/iy/along` trong `SuperBlock::update`, thứ tự gọi `game.check_after_step()`, `Game::check()` mỗi frame. Animation mới chỉ thay *cách nội suy hình ảnh*.
2. `--play` / `--solve` giữ **hành vi và mã thoát cũ** (0 thắng, 1 thua, in `WON`/`GAME OVER` + `Steps`) — đây là công cụ kiểm chứng luật C++ ↔ Python.
3. Vẫn build được trên **WSL/Linux** (`make`), không chỉ Windows.
4. **Hỏi người dùng trước khi cài MSYS2/pacman** (quy tắc 4 `CLAUDE.md`). Makefile dùng tab; file text LF.
5. Thư viện mới (Dear ImGui – MIT) ghi vào `THIRD_PARTY_NOTICES.md`; giữ ghi nguồn repo gốc.

## 2. Phase A — Build Windows native + đo FPS (khắc phục lag lớn nhất)

- Cài MSYS2 (sau khi hỏi): `pacman -S make mingw-w64-x86_64-{gcc,glfw,glm}`; build trong shell MinGW64: `cd code && make` → `a.exe`.
- `main.cpp`: hiển thị FPS + frame time trên tiêu đề cửa sổ (cập nhật mỗi 0.5 s — dùng luôn khối `if ((current_time - last_update_time) >= 0.5)` đang bỏ trống ở `main.cpp:247`) để so sánh trước/sau.
- `headers.h`: `#include <cmath>` đã có; MinGW cần `_USE_MATH_DEFINES` — đã có.
- Kiểm tra đường dẫn Unicode (`Cơ sở AI`): game chỉ dùng đường dẫn tương đối nên không ảnh hưởng.

**Mốc A:** `a.exe 1 --play sol1.txt` → `WON, Steps : 10`; FPS ổn định = tần số màn hình (vsync).

## 3. Phase B — Tối ưu & animation theo thời gian thực (delta time)

| File | Thay đổi |
|---|---|
| `main.cpp` | Tính `dt = now - prev` mỗi frame, kẹp `dt ≤ 0.05` (tránh nhảy khi kéo cửa sổ). Truyền `dt` cho `camera.update(dt)`, `superblock.update(dt)`, effects. Bật MSAA: `glfwWindowHint(GLFW_SAMPLES, 4)` + `glEnable(GL_MULTISAMPLE)`. |
| `superblock.cpp` | `update(float dt)`: `done_rotation += ROLL_SPEED*dt` (≈ 90° trong 0.16–0.2 s), **kẹp** `done_rotation = min(done_rotation, 90)`; khi đủ 90° chạy y nguyên khối chuyển trạng thái hiện có rồi `game.check_after_step()`. Trong `draw`, góc hiển thị = `90 * easeInOutCubic(done/90)`; công thức `real_pos` dùng cùng góc đã ease (camera bám theo mượt). |
| `camera.cpp` | Follow: `eye += (1 - exp(-k*dt)) * (r - eye)` (k ≈ 6); di chuyển helicopter/zoom nhân `dt`. |
| `block.cpp`, `tile.cpp` | Bỏ VAO riêng cho từng ô: mọi ô/khối dùng **một mesh hộp đơn vị dùng chung** (Phase C), vẽ bằng ma trận model scale+translate. Bật cầu không còn cấp phát GPU. |
| `noedit.cpp` | `draw3DObject`: bỏ `glPolygonMode` + `glEnableVertexAttribArray` lặp mỗi lần vẽ (đặt một lần khi tạo VAO). |
| `main.cpp` | Bản đồ nhỏ (minimap) bật/tắt bằng phím `Tab` (mặc định bật, như cũ). |

`autoplay.cpp` đã dùng thời gian thực (`autoplay_delay = 0.25 s`) — giữ nguyên.

**Mốc B:** lăn khối mất cùng thời gian dù FPS 30 hay 144; `--play` màn 1/2 vẫn ra 10/68 bước.

## 4. Phase C — Giao diện mới: nền đen, ánh sáng, hình ảnh từng loại ô

### C1. Module vẽ mới `code/render.h` / `render.cpp`
- `Mesh` hộp đơn vị 24 đỉnh: vị trí (loc 0), **pháp tuyến** (loc 1), tọa độ cục bộ 0..1 trên mặt (loc 2) để shader vẽ viền/hoa văn.
- `Renderer`: nạp shader, cache uniform location; hàm `draw_box(model, color, pattern, emissive, alpha)` — dùng chung cho ô, khối người chơi, sau này cho **split** (cube 1×1×1, task S2-04).
- Shader **nhúng vào code** (`code/shaders.h`, raw string C++11) để bản đóng gói không cần file `.vert/.frag`; thêm hàm `LoadShadersFromSource` cạnh `LoadShaders` trong `noedit.cpp`. Xóa `Sample_GL.vert/.frag` khi xong.

### C2. Shader (`shaders.h`)
- Vertex: `uModel`, `uViewProj`; xuất pháp tuyến thế giới, vị trí, tọa độ mặt.
- Fragment: Blinn-Phong (1 đèn hướng + ambient) + **rim light** + **viền tối/bevel** tính từ khoảng cách tới mép mặt (giống Bloxorz gốc) + `uPattern` để vẽ hoa văn theo loại ô + `uEmissive`, `uTime` (nhấp nháy), sương mù nhẹ theo độ sâu hòa vào nền đen.

### C3. Bảng màu & hình ảnh (`colors.cpp`, `tile.cpp`)
| Loại | Hình ảnh mới |
|---|---|
| Nền | Đen (`glClearColor(0.02,0.02,0.04)`), vignette nhẹ trong shader; tùy chọn: vài hạt sao trôi chậm |
| Sàn thường (1) | Xám thép xanh, viền tối, bevel sáng ở mép trên |
| Ô cam / fragile (2) | Cam, hoa văn sọc chéo hoặc vết nứt |
| Công tắc thường (3) | Sàn xám + **vòng tròn** xanh lá phát sáng nhẹ |
| Công tắc nặng (4) | Sàn xám + **dấu X** xanh đậm |
| Ô cầu (bật) | Xám ánh vàng để phân biệt sàn cố định |
| **Ô đích** (mới vẽ) | Khung vuông phát sáng nhấp nháy quanh lỗ đích (`stage.tx, ty`) |
| Khối người chơi | Đỏ sẫm/kim loại, viền sáng; **bóng mờ** (quad alpha) dưới khối |

`Floor::get_color` thay bằng hàm trả về `(màu, pattern)`; ô cầu cần cờ `is_bridge` (chỉ dùng cho hiển thị, `tile_type` vẫn là 1 như cũ).

**Mốc C:** chụp màn hình màn 1, 2 ở camera `C`, `V`, `B`; mọi loại ô nhận ra được bằng mắt.

## 5. Phase D — Hiệu ứng (`code/effects.h` / `effects.cpp`)

Tất cả chỉ là hình ảnh, chạy theo `dt`, không đổi trạng thái luật.

| Hiệu ứng | Cách làm |
|---|---|
| Mở màn | Các ô rơi từ trên xuống theo sóng (trễ theo `i+j`), ~0.8 s; khóa phím trong lúc này; `autoplayer.start()` gọi **sau** khi xong |
| Lăn khối | Easing (Phase B) + hạt bụi nhỏ khi chạm đất |
| Cầu bật/tắt | `Floor` ghi thời điểm toggle cho ô; ô mới **trồi lên + scale** 0→1; ô bị xóa được chuyển vào danh sách "ghost" để **rơi + mờ dần** (logic `tiles[a][b]=NULL` giữ nguyên) |
| Thắng | Khối **chìm dần** vào lỗ + chùm hạt sáng + chữ "LEVEL COMPLETE" |
| Thua | Khối **rơi tự do** có xoay; ô cam bị đứng lên thì vỡ rơi theo; chữ "GAME OVER" |
| Kết thúc | `END_HOLD_SECONDS` 1.5 → ~2.0 s cho đủ animation (chỉ ở chế độ `--play/--solve`) |
| Hạt (particles) | Một mảng tối đa ~512 hạt, cập nhật CPU, vẽ bằng `draw_box` cỡ nhỏ (đủ nhanh cho quy mô này) |

Tùy chọn (làm cuối nếu còn thời gian): bloom bằng FBO cho ô đích/hạt — bật/tắt được, mặc định tắt nếu FPS < 60.

## 6. Phase E — HUD & menu bằng Dear ImGui (trùng task S1-07/S3-01 của TV2 — phối hợp, không làm hai lần)

- Thêm `code/third_party/imgui/` (core + `imgui_impl_glfw` + `imgui_impl_opengl3`), makefile thêm `-Ithird_party/imgui` và nguồn `.cpp`; `THIRD_PARTY_NOTICES.md`.
- `code/hud.h/.cpp`: góc trên trái: Màn, Số bước, Trạng thái, FPS; góc dưới: gợi ý phím. Style tối, bo góc, trong suốt nhẹ.
- Màn hình kết thúc (chế độ tương tác): nút **Retry / Next level / Menu** thay vì tự thoát.
- Menu chính: chọn màn (quét `data/*.data`), **Restart** (phím `R`), **Solve BFS/DFS/UCS/A\*** — gọi `run_solver` trong `std::thread`, hiện "Đang giải…", xong thì `autoplayer.load()+start()`; lỗi "chưa cài"/không có Python hiện thành thông báo trong game.
- Cần thêm `Game::reset(level)`: giải phóng `Floor` cũ đúng cách (hiện `Floor::initialize` cấp phát mà không giải phóng, `Game::done` gọi destructor thủ công — sửa lại), reset `gameover/won/number_of_steps`, `game_result`, `autoplayer`.
- `controls.cpp`: bỏ qua phím khi `ImGui::GetIO().WantCaptureKeyboard`.

**Hai chế độ chạy** (`main.cpp`):
- Có `--play`/`--solve` → hành vi CLI cũ, tự thoát, mã thoát 0/1 (giữ cho kiểm chứng).
- Không tham số (double-click `.exe`) → chế độ tương tác: menu, không tự thoát.

## 7. Phase F — Đóng gói thành game app (Windows `.exe`)

### F1. Đường dẫn tài nguyên
- Đầu `main()`: chuyển thư mục làm việc về thư mục chứa exe (Windows: `GetModuleFileNameW` + `_wchdir` — an toàn với đường dẫn tiếng Việt; Linux: `/proc/self/exe`). Shader đã nhúng (C1) nên chỉ còn cần `data/`.
- `run_solver` (`autoplay.cpp`): tìm `solver/main.py` cạnh exe (bản đóng gói), không có thì dùng `../main.py` (bản dev). Trình thông dịch: `$BLOXORZ_PYTHON` → `python` → `py -3` (Windows). Trên bản `-mwindows` gọi bằng `CreateProcessW` + `CREATE_NO_WINDOW` để không nháy cửa sổ console; Linux giữ `system()`.

### F2. Makefile (nhánh MSYS2)
- `make release`: `-O2 -DNDEBUG -static -static-libgcc -static-libstdc++ -mwindows`, link `libglfw3.a`; `windres bloxorz.rc` → icon + version info (`code/assets/bloxorz.ico`).
- `make dist`: tạo
  ```
  dist/Bloxorz/
    Bloxorz.exe
    data/*.data
    solver/main.py, solver/ai/**      (copy từ gốc repo)
    README.txt                        (yêu cầu Python ≥ 3.10, phím điều khiển)
  ```
  rồi nén `dist/Bloxorz-win64.zip`. Thêm `dist/` vào `.gitignore`.
- Kiểm tra `ldd Bloxorz.exe` trong MSYS2: chỉ còn DLL hệ thống Windows.
- Linux: `make dist` tương tự (binary + data + solver), không cần static.

### F3. Tài liệu
- `README.md`, `Docs/play.md`: thêm mục build native Windows (MSYS2), phím mới (`R`, `Tab`, menu), cách tạo bản đóng gói. Cập nhật bảng trạng thái.

## 8. File chính bị ảnh hưởng

- Sửa: `code/main.cpp`, `superblock.cpp/.h`, `tile.cpp/.h`, `block.cpp/.h`, `camera.cpp/.h`, `colors.cpp`, `controls.cpp`, `game.cpp/.h`, `autoplay.cpp/.h`, `noedit.cpp/.h`, `makefile`, `README.md`, `Docs/play.md`, `THIRD_PARTY_NOTICES.md`, `.gitignore`.
- Mới: `code/render.*`, `code/shaders.h`, `code/effects.*`, `code/hud.*`, `code/third_party/imgui/`, `code/bloxorz.rc`, `code/assets/bloxorz.ico`.
- Không đụng: `ai/`, `tests/`, `main.py`, `code/data/*.data`.

## 9. Thứ tự & ước lượng

| Phase | Nội dung | Ước lượng |
|---|---|---|
| A | MSYS2 build + FPS | 0.5 ngày |
| B | Delta time, easing, bỏ VAO từng ô | 1 ngày |
| C | Renderer + shader + bảng màu, ô đích | 1.5–2 ngày |
| D | Hiệu ứng | 1.5 ngày |
| E | ImGui HUD/menu/Restart/Solve nền (phối hợp TV2) | 2 ngày |
| F | Đóng gói + tài liệu | 1 ngày |

Mỗi phase kết thúc bằng bước kiểm chứng ở mục 10 trước khi sang phase sau.

## 10. Kiểm chứng

```bash
python -m pytest -q                                   # phải không đổi (ai/ không bị sửa)
cd code && make                                       # Linux/WSL vẫn build được
echo "R U U R U R U R D L" > sol1.txt
./a.out 1 --play sol1.txt ; echo $?                   # WON, Steps : 10, exit 0
./a.out 2 --play <lời giải 68 bước> ; echo $?         # WON, Steps : 68, exit 0
echo "L" > lose.txt && ./a.out 1 --play lose.txt ; echo $?   # GAME OVER, exit 1
```
Trên MSYS2: lặp lại với `a.exe`; rồi `make dist`, giải nén zip sang thư mục khác (đường dẫn có dấu tiếng Việt), **double-click** `Bloxorz.exe`: mở menu, chơi tay thắng/thua màn 1–2, Restart, chọn màn, Solve (khi solver chưa cài phải hiện thông báo gọn, không treo), không nháy console. Đo FPS trên tiêu đề cửa sổ trước/sau Phase A–B và ghi lại vào báo cáo.
