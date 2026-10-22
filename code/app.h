#ifndef APP
#define APP

// Trạng thái và thao tác cấp ứng dụng (định nghĩa trong main.cpp), dùng chung cho phím tắt và HUD.

enum SolveState { SOLVE_IDLE = 0, SOLVE_RUNNING, SOLVE_FAILED };

extern int level;
extern bool cli_mode;        // chạy với --play/--solve: hành vi cũ (tự thoát, mã thoát 0/1)
extern bool menu_open;
extern bool show_minimap;
extern bool show_help;
extern int game_result;      // -1 đang chơi, 0 thua, 1 thắng
extern float current_fps;
extern float current_frame_ms;
extern int last_draw_calls;       // số lệnh vẽ của frame trước
extern int last_boxes;            // số hộp đã vẽ của frame trước
extern std::string gpu_name;      // GL_RENDERER
extern int solve_state;
extern std::string solve_algo;       // thuật toán đang/vừa chạy (để hiện trên HUD)
extern std::string solve_message;    // thông báo lỗi khi solver thất bại
extern std::string replay_label;     // nhãn của lời giải đang phát lại

std::vector<int> available_levels(); // các màn có file data/<n>.data
bool app_input_locked();             // true -> phím mũi tên không lăn khối
void app_start_level(int lv);
void app_restart();
void app_next_level();
void app_toggle_menu();
void app_solve(const std::string &algo);
void app_request_screenshot();
void app_quit();

#endif
