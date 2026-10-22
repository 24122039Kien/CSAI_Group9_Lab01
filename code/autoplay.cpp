#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "headers.h"
#include "superblock.h"
#include "autoplay.h"
#include "noedit.h"

#include <cctype>
#include <sstream>
#include <cstdio>
#ifndef _WIN32
#include <sys/wait.h>
#endif

AutoPlayer autoplayer;
double autoplay_delay = 0.25;

AutoPlayer::AutoPlayer()
{
    next_index = 0;
    rest_since = 0.0;
    active = false;
}

bool AutoPlayer::load(const string &path)
{
    string text;
    if(!read_text_file(path, text))   // đường dẫn UTF-8, mở được cả thư mục có dấu trên Windows
    {
        cerr << "Autoplay: cannot open solution file " << path << endl;
        return false;
    }
    istringstream fin(text);
    moves.clear();
    string tok;
    while(fin >> tok)
    {
        for(size_t i=0; i<tok.size(); i++)
        {
            char c = toupper(tok[i]);
            if(c=='U' || c=='D' || c=='L' || c=='R')
                moves.push_back(c);
            else
            {
                cerr << "Autoplay: invalid move '" << tok[i] << "' in " << path << endl;
                return false;
            }
        }
    }
    cout << "Autoplay: loaded " << moves.size() << " moves from " << path << endl;
    return true;
}

void AutoPlayer::start(double now)
{
    next_index = 0;
    rest_since = now;
    active = !moves.empty();
}

void AutoPlayer::update(double now)
{
    if(!active)
        return;
    if(superblock.is_moving())
    {
        rest_since = now;
        return;
    }
    if(next_index >= moves.size())
    {
        active = false;
        cout << "Autoplay: finished all " << moves.size() << " moves" << endl;
        return;
    }
    if(now - rest_since < autoplay_delay)
        return;

    char c = moves[next_index++];
    int key = GLFW_KEY_UP;
    if(c=='D') key = GLFW_KEY_DOWN;
    else if(c=='L') key = GLFW_KEY_LEFT;
    else if(c=='R') key = GLFW_KEY_RIGHT;
    cout << "Autoplay: move " << next_index << "/" << moves.size() << " : " << c << endl;
    superblock.move(key);
}

void AutoPlayer::stop()
{
    active = false;
    next_index = 0;
}

size_t AutoPlayer::played()
{
    return next_index;
}

bool AutoPlayer::is_active()
{
    return active;
}

size_t AutoPlayer::total()
{
    return moves.size();
}

const char *solver_status_text(int status)
{
    switch(status)
    {
        case SOLVER_OK:              return "Solution found";
        case SOLVER_NO_SOLUTION:     return "No solution exists for this level";
        case SOLVER_NOT_IMPLEMENTED: return "This algorithm is not implemented yet (TODO in ai/solvers)";
        case SOLVER_INVALID:         return "Solver returned an invalid solution";
        case SOLVER_BAD_LEVEL:       return "Solver could not read the level file";
        case SOLVER_NO_PYTHON:       return "Python not found - install Python 3.10+ or set BLOXORZ_PYTHON";
        default:                     return "Solver failed - see solver_log.txt";
    }
}

#ifdef _WIN32
static std::wstring widen(const string &s)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    std::wstring w(len > 0 ? len - 1 : 0, L'\0');
    if(len > 1)
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], len);
    return w;
}
#endif

// Chạy lệnh và trả về mã thoát (-1 nếu không chạy được).
static int run_command(const string &cmd, bool quiet)
{
#ifdef _WIN32
    if(!quiet)
        return system(cmd.c_str());
    // GUI: chạy qua cmd.exe nhưng không mở cửa sổ console, output ghi vào solver_log.txt.
    std::wstring line = widen("cmd.exe /S /C \"" + cmd + " > solver_log.txt 2>&1\"");
    std::vector<wchar_t> buf(line.begin(), line.end());
    buf.push_back(L'\0');
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if(!CreateProcessW(NULL, &buf[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int)code;
#else
    int rc = system((quiet ? cmd + " > solver_log.txt 2>&1" : cmd).c_str());
    if(rc == -1 || !WIFEXITED(rc))
        return -1;
    return WEXITSTATUS(rc);
#endif
}

int run_solver(int level, const string &algo, const string &out_path, bool quiet)
{
    // Chỉ cho phép tên thuật toán đã biết -> không chèn được lệnh shell.
    if(algo!="bfs" && algo!="dfs" && algo!="ucs" && algo!="astar")
    {
        cerr << "Unknown algorithm '" << algo << "' (use bfs, dfs, ucs or astar)" << endl;
        return SOLVER_ERROR;
    }

    // Output của solver có tiếng Việt: ép UTF-8 để Python không lỗi khi ghi ra file/console.
#ifdef _WIN32
    _putenv("PYTHONIOENCODING=utf-8");
#else
    setenv("PYTHONIOENCODING", "utf-8", 1);
#endif

    // Bản đóng gói đặt solver ở solver/main.py cạnh file chạy; trong repo là ../main.py.
    string script = ifstream("solver/main.py") ? "solver/main.py" : "../main.py";
    string args = " " + script + " data/" + to_string(level) + ".data --algo " + algo + " --out " + out_path;

    const char *env = getenv("BLOXORZ_PYTHON");
    vector<string> pythons;
    if(env && *env)
        pythons.push_back(env);
    else
    {
#ifdef _WIN32
        pythons.push_back("python");
        pythons.push_back("py -3");
#else
        pythons.push_back("python3");
#endif
    }

    remove(out_path.c_str());
    int rc = -1;
    for(size_t k=0; k<pythons.size(); k++)
    {
        string cmd = pythons[k] + args;
        if(!quiet)
            cout << "Running solver: " << cmd << endl;
        rc = run_command(cmd, quiet);
        if(rc != 9009 && rc != 127)   // 9009 (cmd.exe) / 127 (sh): không tìm thấy lệnh -> thử cái tiếp theo
            break;
    }

    int status;
    if(rc == 9009 || rc == 127)
        status = SOLVER_NO_PYTHON;
    else if(rc >= SOLVER_OK && rc <= SOLVER_BAD_LEVEL)
        status = rc;
    else
        status = SOLVER_ERROR;
    if(status == SOLVER_OK && !ifstream(out_path))
        status = SOLVER_ERROR;

    if(status != SOLVER_OK)
        cerr << "Solver failed (exit status " << rc << "): " << solver_status_text(status) << endl;
    return status;
}
