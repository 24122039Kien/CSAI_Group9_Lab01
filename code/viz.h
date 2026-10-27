#ifndef VIZ
#define VIZ

#include <string>

// Trực quan hóa quá trình tìm kiếm (nút "Visualize search"): đọc file trace do
// `python main.py ... --trace trace.txt` ghi (ai/tracing.py), vẽ cây tìm kiếm / đồ thị trạng thái
// trong cửa sổ ImGui và phủ bản đồ nhiệt lên bàn chơi 3D, cho chạy từng bước.
// Chỉ đọc file và vẽ: không đụng tới luật chơi hay solver.

bool viz_load(const std::string &path, std::string &error);
bool viz_is_open();
void viz_close();
void viz_update(double now);     // tự tiến bước khi đang Play
void viz_step(int delta);        // +1 / -1 bước (phím mũi tên khi cửa sổ đang mở)
void viz_toggle_play();
void viz_draw_window(float x, float y, float w, float h);   // cửa sổ ImGui, gọi trong hud_frame
void viz_draw_board();           // lớp phủ 3D, gọi trong lượt vẽ trong suốt

#endif
