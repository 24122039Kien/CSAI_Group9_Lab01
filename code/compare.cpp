#include "headers.h"
#include "autoplay.h"
#include "compare.h"

#include <map>
#include <mutex>
#include <thread>
#include <chrono>

const char *COMPARE_ALGO_IDS[COMPARE_ALGOS] = {"bfs", "dfs", "ucs", "astar"};

static std::mutex compare_mutex;
static std::map<int, CompareResult> results;   // kết quả theo màn (giữ trong lúc chơi)
static bool running = false;
static int last_run_level = -1;                 // các file compare_<algo>.txt thuộc màn này

static string solution_path(const string &algo) { return "compare_" + algo + ".txt"; }
static string stats_path(const string &algo) { return "compare_" + algo + "_stats.txt"; }

bool compare_start(int level, const string &heuristic, double now)
{
    std::lock_guard<std::mutex> lock(compare_mutex);
    if(running)
        return false;
    CompareResult r;
    r.level = level;
    r.heuristic = heuristic;
    r.running = true;
    r.started_at = now;
    r.finished_at = 0.0;
    for(int k=0; k<COMPARE_ALGOS; k++)
    {
        r.rows[k].algo = COMPARE_ALGO_IDS[k];
        r.rows[k].state = ROW_WAITING;
        r.rows[k].status = SOLVER_ERROR;
        r.rows[k].stats.valid = false;
        r.rows[k].wall_ms = 0.0;
    }
    results[level] = r;
    running = true;
    last_run_level = level;

    // Chạy tuần tự (không song song) để số đo thời gian/bộ nhớ của từng thuật toán không ảnh hưởng nhau.
    std::thread([level, heuristic]() {
        for(int k=0; k<COMPARE_ALGOS; k++)
        {
            string algo = COMPARE_ALGO_IDS[k];
            {
                std::lock_guard<std::mutex> lock(compare_mutex);
                results[level].rows[k].state = ROW_RUNNING;
            }
            remove(stats_path(algo).c_str());
            auto t0 = std::chrono::steady_clock::now();
            int status = run_solver(level, algo, solution_path(algo), true, heuristic, "--stats " + stats_path(algo));
            double wall = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
            SolverStats st;
            bool ok = read_solver_stats(stats_path(algo), st);
            std::lock_guard<std::mutex> lock(compare_mutex);
            CompareRow &row = results[level].rows[k];
            row.state = ROW_DONE;
            row.status = status;
            row.stats = st;
            row.stats.valid = ok;
            row.wall_ms = wall;
        }
        std::lock_guard<std::mutex> lock(compare_mutex);
        results[level].running = false;
        results[level].finished_at = glfwGetTime();
        running = false;
    }).detach();
    return true;
}

bool compare_running()
{
    std::lock_guard<std::mutex> lock(compare_mutex);
    return running;
}

bool compare_has(int level)
{
    std::lock_guard<std::mutex> lock(compare_mutex);
    return results.count(level) > 0;
}

CompareResult compare_get(int level)
{
    std::lock_guard<std::mutex> lock(compare_mutex);
    std::map<int, CompareResult>::iterator it = results.find(level);
    if(it != results.end())
        return it->second;
    CompareResult empty;
    empty.level = -1;
    empty.running = false;
    empty.started_at = empty.finished_at = 0.0;
    return empty;
}

string compare_solution_file(int level, const string &algo)
{
    std::lock_guard<std::mutex> lock(compare_mutex);
    if(level != last_run_level || !results.count(level))
        return "";
    const CompareResult &r = results[level];
    for(int k=0; k<COMPARE_ALGOS; k++)
        if(r.rows[k].algo == algo && r.rows[k].state == ROW_DONE && r.rows[k].status == SOLVER_OK)
            return solution_path(algo);
    return "";
}

bool compare_export_csv(int level, string &path_out)
{
    CompareResult r = compare_get(level);
    if(r.level < 0 || r.running)
        return false;
    path_out = "compare_stage_" + to_string(level) + ".csv";
    ofstream out(path_out.c_str());
    if(!out)
        return false;
    out << "level,algorithm,heuristic,status,length,cost,expanded,generated,max_frontier,time_ms,memory_kb\n";
    for(int k=0; k<COMPARE_ALGOS; k++)
    {
        const CompareRow &row = r.rows[k];
        const SolverStats &s = row.stats;
        out << level << "," << row.algo << "," << (row.algo == "astar" ? r.heuristic : "-") << ","
            << (s.valid ? s.status : string(solver_status_text(row.status))) << ",";
        if(s.valid)
            out << s.length << "," << s.cost << "," << s.expanded << "," << s.generated << "," << s.frontier << ","
                << s.time_ms << "," << s.memory_kb << "\n";
        else
            out << ",,,,,,\n";
    }
    return true;
}
