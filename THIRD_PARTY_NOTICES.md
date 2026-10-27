# Third-party notices

## Bloxorz (game C++/OpenGL gốc)

- Nguồn: https://github.com/chaitanya100100/Bloxorz
- Tác giả: Chaitanya Patel
- Commit dùng làm gốc: `c0ccb55b273f24abdee4eb4adc0620e016d748ad` (10/07/2019)
- License: **repo gốc không có file license.** Nhóm sử dụng mã nguồn này cho mục đích học tập trong đồ án môn CSC14003 theo sự cho phép của giảng viên phụ trách (buổi Q&A của môn), và ghi nguồn đầy đủ tại đây, trong `README.md` và trong mục References của báo cáo.
- Phạm vi: toàn bộ thư mục `code/` (trừ các phần nhóm bổ sung/sửa). Commit đầu tiên của nhóm sau "Initial commit" (`Import upstream chaitanya100100/Bloxorz @ c0ccb55`) là bản sao **không chỉnh sửa**; các thay đổi của nhóm xem bằng `git diff <commit đó>..HEAD -- code/`.
- Thư mục `demo/` (gif/mp4 của repo gốc) chỉ giữ ở máy, không đưa lên git.

## GLAD (OpenGL loader)

- Sinh bằng glad 0.1.36 (https://github.com/Dav1dde/glad): `--profile core --api gl=3.3 --generator c --spec gl --extensions ""`
- File: `code/glad.c`, `code/include/glad/glad.h`, `code/include/KHR/khrplatform.h`
- Bộ sinh glad: MIT License. Mã sinh ra dựa trên OpenGL Registry của Khronos (Apache 2.0 / MIT); `khrplatform.h`: Khronos license (đi kèm trong file).

## Dear ImGui (giao diện menu/HUD)

- Nguồn: https://github.com/ocornut/imgui — bản phát hành `v1.91.9` (commit `97428e8`)
- File: `code/third_party/imgui/` (core + backend `imgui_impl_glfw`, `imgui_impl_opengl3`), nguyên vẹn, chỉ đổi xuống dòng sang LF
- License: MIT — `code/third_party/imgui/LICENSE.txt` (bản đóng gói chép vào `licenses/imgui_LICENSE.txt`)
- Font giao diện: game đọc font có sẵn của hệ điều hành lúc chạy (Segoe UI trên Windows, DejaVu Sans trên Linux), không đóng gói file font.
- Icon `code/assets/bloxorz.ico` do nhóm tự sinh bằng `code/assets/make_icon.py`.

## miniaudio (âm thanh)

- Nguồn: https://github.com/mackron/miniaudio — bản phát hành `0.11.21` (2023-11-15)
- File: `code/third_party/miniaudio/miniaudio.h` (nguyên vẹn) + `LICENSE`; `miniaudio_impl.c`, `miniaudio_config.h` do nhóm viết để biên dịch
- License: Public Domain (Unlicense) hoặc MIT-0, tùy chọn — `code/third_party/miniaudio/LICENSE` (bản đóng gói chép vào `licenses/miniaudio_LICENSE.txt`)
- Hiệu ứng âm thanh và nhạc nền được **tổng hợp bằng code** trong `code/audio.cpp`; không dùng file âm thanh của Bloxorz gốc (Coolmath Games) vì bản quyền.

## Thư viện liên kết khi build (không đi kèm trong repo)

- GLFW — zlib/libpng License — https://www.glfw.org/
- GLM — MIT License (hoặc Happy Bunny License) — https://github.com/g-truc/glm
- OpenGL (Mesa/driver hệ thống)
