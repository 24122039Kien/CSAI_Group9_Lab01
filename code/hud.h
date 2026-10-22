#ifndef HUD
#define HUD

// Giao diện 2D vẽ đè lên cảnh 3D (Dear ImGui): thông số, menu, bảng kết thúc, thông báo solver.
void hud_init(GLFWwindow *window);
void hud_frame(double now);       // dựng + vẽ HUD; gọi sau khi vẽ cảnh 3D, trước swap
bool hud_wants_keyboard();
bool hud_wants_mouse();
void hud_shutdown();

#endif
