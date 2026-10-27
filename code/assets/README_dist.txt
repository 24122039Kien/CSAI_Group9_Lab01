BLOXORZ 3D - CSC14003 Group 9
=============================

Chơi game
---------
- Mở Bloxorz.exe (Windows) hoặc ./bloxorz (Linux). Không cần cài thêm gì để chơi.
- Menu New Game: chọn màn. Trong màn: thanh New Game / Restart / Pause ở trên, bảng AI bên phải
  (BFS / DFS / UCS / A*, Visualize search để xem cây tìm kiếm chạy từng bước).
- Options: theme Classic / Minecraft, âm lượng nhạc nền và hiệu ứng (lưu ở settings.ini).

Phím
----
  Mũi tên          lăn khối (Visualizer: lùi / tiến một bước)
  Space            đổi khối nhỏ sau khi tách (Visualizer: chạy / dừng)
  R / Enter        chơi lại / sang màn tiếp (khi đã thắng)
  P / Esc          tạm dừng
  C V B X N M      đổi camera (theo khối, từ trên, từ tháp, trực thăng, kéo chuột, theo chuột)
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
Thay âm thanh: đặt sounds/<tên>.wav (roll, stand, switch, heavy, bridge_on, bridge_off, fall, win,
split, merge, crack, click, start, collapse, music) cạnh file chạy.

Ghi nguồn: game gốc chaitanya100100/Bloxorz (Chaitanya Patel); GLFW, GLM, GLAD, Dear ImGui, miniaudio -
xem thư mục licenses/.
