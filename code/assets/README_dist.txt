BLOXORZ 3D - CSC14003 Group 9
=============================

Chơi game
---------
- Mở Bloxorz.exe (Windows) hoặc ./bloxorz (Linux). Không cần cài thêm gì để chơi.
- Menu: chọn màn, bấm BFS / DFS / UCS / A* để AI giải màn hiện tại rồi tự phát lại lời giải.

Phím
----
  Mũi tên          lăn khối
  R / Enter        chơi lại / sang màn tiếp (khi đã thắng)
  Esc              mở / đóng menu
  C V B X N M      đổi camera (theo khối, từ trên, từ tháp, trực thăng, kéo chuột, theo chuột)
  Space            đổi phép chiếu phối cảnh / song song
  Tab              bật / tắt bản đồ nhỏ
  F1               ẩn / hiện bảng phím
  F12              chụp màn hình (screenshot_*.png cạnh file chạy)
  Q                thoát

Nút Solve (AI)
--------------
- Cần Python >= 3.10 (https://www.python.org/downloads/ - nhớ tick "Add python.exe to PATH").
- Game gọi solver/main.py; nếu Python ở chỗ khác, đặt biến môi trường BLOXORZ_PYTHON
  (ví dụ: BLOXORZ_PYTHON=C:\Python312\python.exe).
- Output của lần giải gần nhất: solver_log.txt.

Dòng lệnh (kiểm chứng lời giải)
-------------------------------
  Bloxorz.exe 2 --play solution.txt    phát lại file lời giải U/D/L/R
  Bloxorz.exe 2 --solve astar          gọi solver rồi phát lại
  Mã thoát: 0 thắng, 1 thua.

Thêm màn chơi: đặt file data/<số>.data (định dạng trong README của repo).

Ghi nguồn: game gốc chaitanya100100/Bloxorz (Chaitanya Patel); GLFW, GLM, GLAD, Dear ImGui -
xem thư mục licenses/.
