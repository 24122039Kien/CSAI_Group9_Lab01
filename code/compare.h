#ifndef COMPARE
#define COMPARE

#include <string>
#include "app.h"

// So sánh 4 thuật toán trên một màn: chạy lần lượt BFS, DFS, UCS, A* bằng solver Python (main.py --stats)
// trong luồng nền, giữ kết quả của từng màn để bảng so sánh + biểu đồ trong game đọc lại và xuất CSV.
enum { COMPARE_ALGOS = 4 };
enum CompareRowState { ROW_WAITING = 0, ROW_RUNNING, ROW_DONE };

struct CompareRow
{
    std::string algo;      // bfs | dfs | ucs | astar
    int state;             // CompareRowState
    int status;            // SolverStatus khi xong
    SolverStats stats;     // chỉ số do main.py đo (valid = false nếu không đọc được)
    double wall_ms;        // thời gian thực của cả lần gọi Python (kể cả khởi động trình thông dịch)
};

struct CompareResult
{
    int level;
    std::string heuristic;  // heuristic của A* trong lần chạy này
    bool running;
    double started_at, finished_at;
    CompareRow rows[COMPARE_ALGOS];
};

extern const char *COMPARE_ALGO_IDS[COMPARE_ALGOS];

// Bắt đầu chạy (bỏ qua nếu đang có một lượt so sánh khác chạy).
bool compare_start(int level, const std::string &heuristic, double now);
bool compare_running();
bool compare_has(int level);
CompareResult compare_get(int level);        // bản sao an toàn luồng (level = -1 nếu chưa có)
// File lời giải của thuật toán trong lượt so sánh GẦN NHẤT ("" nếu lượt đó không phải màn này/không có lời giải).
std::string compare_solution_file(int level, const std::string &algo);
// Ghi kết quả của màn ra compare_stage_<level>.csv (để dán vào phần thực nghiệm của báo cáo).
bool compare_export_csv(int level, std::string &path_out);

#endif
