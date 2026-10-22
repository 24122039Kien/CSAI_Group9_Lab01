#ifndef AUTOPLAY
#define AUTOPLAY

#include <string>
#include <vector>

// Phát lại lời giải: đọc chuỗi U/D/L/R và tự gọi superblock.move() từng nước.
class AutoPlayer
{
    private:
        std::vector<char> moves;
        size_t next_index;
        double rest_since;      // thời điểm khối bắt đầu đứng yên
        bool active;
    public:
        AutoPlayer();
        bool load(const std::string &path);   // false nếu không đọc được file
        void start(double now);
        void stop();
        void update(double now);              // gọi mỗi frame, trước superblock.update()
        bool is_active();
        size_t total();
        size_t played();                      // số nước đã phát
};

// Kết quả gọi solver. 0..4 trùng mã thoát của main.py.
enum SolverStatus
{
    SOLVER_OK = 0, SOLVER_NO_SOLUTION = 1, SOLVER_NOT_IMPLEMENTED = 2, SOLVER_INVALID = 3,
    SOLVER_BAD_LEVEL = 4, SOLVER_NO_PYTHON = 5, SOLVER_ERROR = 6
};

// Gọi solver Python: <python> <main.py> data/<level>.data --algo <algo> --out <out_path>
// <python> lấy từ biến môi trường BLOXORZ_PYTHON, mặc định python3 (Linux/macOS) / python rồi py -3 (Windows).
// <main.py> là solver/main.py (bản đóng gói) nếu có, ngược lại ../main.py (chạy trong repo).
// quiet = true: không in ra console, ghi output vào solver_log.txt và không mở cửa sổ console (GUI).
// Hàm chặn cho tới khi solver chạy xong — GUI gọi trong luồng riêng.
int run_solver(int level, const std::string &algo, const std::string &out_path, bool quiet = false);
const char *solver_status_text(int status);

extern AutoPlayer autoplayer;
extern double autoplay_delay;   // giây chờ giữa hai nước (mặc định 0.25)

#endif
