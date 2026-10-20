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
        void update(double now);              // gọi mỗi frame, trước superblock.update()
        bool is_active();
        size_t total();
};

// Gọi solver Python: <python> ../main.py data/<level>.data --algo <algo> --out <out_path>
// <python> lấy từ biến môi trường BLOXORZ_PYTHON, mặc định python3 (Linux/macOS) / python (Windows).
// Chạy từ thư mục code/ (giống game). Trả về true nếu solver ghi được lời giải.
bool run_solver(int level, const std::string &algo, const std::string &out_path);

extern AutoPlayer autoplayer;
extern double autoplay_delay;   // giây chờ giữa hai nước (mặc định 0.25)

#endif
