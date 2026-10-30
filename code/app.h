#ifndef APP
#define APP

#include <map>
#include <string>
#include <vector>

// Trạng thái và thao tác cấp ứng dụng (định nghĩa trong main.cpp), dùng chung cho phím tắt, HUD và Visualizer.

enum SolveState { SOLVE_IDLE = 0, SOLVE_RUNNING, SOLVE_FAILED };

// Kết quả solver gần nhất (đọc từ stats.txt do `main.py --stats` ghi), hiện trong bảng AI.
struct SolverStats
{
    bool valid;
    std::string algorithm, status;
    int length;
    double cost, time_ms, memory_kb;
    long expanded, generated, frontier;
};

extern int level;
extern bool cli_mode;        // chạy với --play/--solve: hành vi cũ (tự thoát, mã thoát 0/1)
extern bool menu_open;       // màn hình chính / chọn màn (New Game)
extern bool paused;          // bảng Pause trong màn
extern bool options_open;    // khung Options (theme, âm lượng...)
extern bool show_minimap;
extern bool show_help;
extern bool show_ai_panel;
extern bool compare_open;    // bảng so sánh 4 thuật toán của màn hiện tại
extern int game_result;      // -1 đang chơi, 0 thua, 1 thắng
extern int fall_count;       // số lần rơi ở màn hiện tại (tự chơi lại như game gốc)
extern double level_started_at;          // lúc bắt đầu màn (HUD hiện tên màn như game gốc)
extern std::map<int, int> best_moves;    // kỷ lục số bước của từng màn (lưu trong settings.ini)
extern float current_fps;
extern float current_frame_ms;
extern int last_draw_calls;       // số lệnh vẽ của frame trước
extern int last_boxes;            // số hộp đã vẽ của frame trước
extern std::string gpu_name;      // GL_RENDERER
extern int solve_state;
extern std::string solve_algo;       // thuật toán đang/vừa chạy (để hiện trên HUD)
extern std::string solve_heuristic;  // heuristic cho A*: "manhattan" | "zero"
extern std::string solve_message;    // thông báo lỗi khi solver thất bại
extern std::string replay_label;     // nhãn của lời giải đang phát lại
extern SolverStats last_stats;
extern float music_volume, sfx_volume;

std::vector<int> available_levels(); // các màn có file data/<n>.data
std::string level_title(int lv);     // tên màn từ data/catalog.json ("" nếu không có)
std::string level_group(int lv);     // nhóm độ khó G1..G6
bool app_input_locked();             // true -> phím mũi tên không lăn khối
void app_start_level(int lv);
void app_restart();
void app_next_level();
void app_open_menu();                // New Game: mở màn hình chọn màn
void app_toggle_menu();
void app_toggle_pause();
void app_solve(const std::string &algo);
void app_visualize(const std::string &algo);
void app_close_visualizer();
void app_play_solution();            // phát lại lời giải của lần Visualize
void app_toggle_compare();           // mở/đóng bảng so sánh (lần đầu mở ở một màn: tự chạy cả 4 thuật toán)
void app_compare_rerun();            // chạy lại so sánh cho màn hiện tại (vd sau khi đổi heuristic)
void app_replay_compare(const std::string &algo);   // phát lại lời giải của một thuật toán trong bảng
void app_set_theme(int theme);
void app_apply_volumes();
void app_save_settings();
void app_request_screenshot();
void app_quit();

#endif
