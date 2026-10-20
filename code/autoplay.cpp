#include "headers.h"
#include "superblock.h"
#include "autoplay.h"

#include <cctype>
#include <cstdio>

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
    ifstream fin(path);
    if(!fin)
    {
        cerr << "Autoplay: cannot open solution file " << path << endl;
        return false;
    }
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

bool AutoPlayer::is_active()
{
    return active;
}

size_t AutoPlayer::total()
{
    return moves.size();
}

bool run_solver(int level, const string &algo, const string &out_path)
{
    // Chỉ cho phép tên thuật toán đã biết -> không chèn được lệnh shell.
    if(algo!="bfs" && algo!="dfs" && algo!="ucs" && algo!="astar")
    {
        cerr << "Unknown algorithm '" << algo << "' (use bfs, dfs, ucs or astar)" << endl;
        return false;
    }

    const char *env = getenv("BLOXORZ_PYTHON");
#ifdef _WIN32
    string python = (env && *env) ? env : "python";
#else
    string python = (env && *env) ? env : "python3";
#endif

    remove(out_path.c_str());
    string cmd = python + " ../main.py data/" + to_string(level) + ".data --algo " + algo
               + " --out " + out_path;
    cout << "Running solver: " << cmd << endl;
    int rc = system(cmd.c_str());
    if(rc != 0)
    {
        cerr << "Solver failed (exit status " << rc << "). See messages above." << endl;
        return false;
    }
    ifstream check(out_path);
    if(!check)
    {
        cerr << "Solver did not write " << out_path << endl;
        return false;
    }
    return true;
}
