#ifndef HUD
#define HUD

#include <string>

// Giao diện 2D vẽ đè lên cảnh 3D (Dear ImGui): màn hình chính (logo, menu, chọn màn, Options, hướng dẫn),
// thanh công cụ (Menu / Restart / Pause), bảng AI, bảng Pause, bảng kết thúc, thông báo solver, Search Visualizer.
void hud_init(GLFWwindow *window);
void hud_apply_theme(int theme);  // đổi kiểu giao diện theo theme (Classic / Original / Minecraft)
void hud_frame(double now);       // dựng + vẽ HUD; gọi sau khi vẽ cảnh 3D, trước swap
bool hud_wants_keyboard();
bool hud_wants_mouse();
bool hud_menu_back();             // Esc trong màn hình chính: về trang đầu (true nếu đã xử lý)
void hud_open_page(const std::string &name);   // levels | settings | howto | credits (cờ dev --ui)
void hud_shutdown();

#endif
