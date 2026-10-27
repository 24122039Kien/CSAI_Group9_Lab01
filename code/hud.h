#ifndef HUD
#define HUD

// Giao diện 2D vẽ đè lên cảnh 3D (Dear ImGui): thanh công cụ (New Game / Restart / Pause), bảng AI,
// bảng Pause + Options, màn hình chọn màn, bảng kết thúc, thông báo solver, Search Visualizer.
void hud_init(GLFWwindow *window);
void hud_apply_theme(int theme);  // đổi kiểu giao diện theo theme (Classic / Minecraft)
void hud_frame(double now);       // dựng + vẽ HUD; gọi sau khi vẽ cảnh 3D, trước swap
bool hud_wants_keyboard();
bool hud_wants_mouse();
void hud_shutdown();

#endif
