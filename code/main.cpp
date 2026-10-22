#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>   // CommandLineToArgvW
#endif

#include "headers.h"
#include "noedit.h"
#include "controls.h"
#include "colors.h"
#include "camera.h"
#include "render.h"
#include "tile.h"
#include "superblock.h"
#include "game.h"
#include "autoplay.h"
#include "effects.h"
#include "hud.h"
#include "screenshot.h"
#include "app.h"

#include <atomic>
#include <thread>
#include <ctime>
#ifndef _WIN32
#include <unistd.h>
#endif

#if defined(_WIN32) && !defined(BLOXORZ_NO_DGPU_HINT)
// Laptop có 2 GPU (Intel + NVIDIA/AMD): yêu cầu driver chạy game trên GPU rời thay vì GPU tích hợp.
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

// Màn mặc định (đổi bằng tham số dòng lệnh: ./a.out <level>)
int level = 2;

// Hai chế độ chạy:
//  - cli_mode (có --play/--solve): giữ hành vi cũ để kiểm chứng luật — tự phát lại, in WON/GAME OVER,
//    giữ màn hình END_HOLD_SECONDS rồi thoát với mã 0 (thắng) / 1 (thua).
//  - tương tác (mở bằng double-click): có menu chọn màn, Restart, Solve chạy nền; không tự thoát.
bool cli_mode = false;
bool menu_open = false;
bool show_minimap = true;
bool show_help = true;

// Kết quả ván: -1 đang chơi, 0 thua, 1 thắng.
int game_result = -1;
double game_end_time = 0.0;
const double END_HOLD_SECONDS = 2.0;

float current_fps = 0.0f;
float current_frame_ms = 0.0f;
int last_draw_calls = 0;
int last_boxes = 0;
string gpu_name;

int solve_state = SOLVE_IDLE;
string solve_algo, solve_message, replay_label;
static int solver_level = 0;
static std::atomic<bool> solver_done(false);
static std::atomic<int> solver_status(0);
static bool pending_autoplay = false;     // có lời giải chờ phát sau hiệu ứng mở màn
static bool screenshot_requested = false;

struct GLMatrices Matrices;
GLFWwindow* window;

glm::mat4 world_model;
Floor stage;
SuperBlock superblock;
Game game;

/**************************
 * Thao tác cấp ứng dụng  *
 **************************/

vector<int> available_levels()
{
    // Menu gọi hàm này mỗi frame: chỉ quét lại thư mục data/ sau mỗi 2 giây (thêm màn mới vẫn hiện ra).
    static vector<int> cached;
    static double scanned_at = -1e9;
    double now = glfwGetTime();
    if(now - scanned_at < 2.0)
        return cached;
    cached.clear();
    for(int i=1; i<100; i++)
        if(ifstream("data/" + to_string(i) + ".data"))
            cached.push_back(i);
    scanned_at = now;
    return cached;
}

bool app_input_locked()
{
    return autoplayer.is_active() || pending_autoplay || game_result != -1
        || game.is_won() || game.is_gameover() || menu_open || solve_state == SOLVE_RUNNING
        || effects.intro_active(glfwGetTime());
}

void app_start_level(int lv)
{
    level = lv;
    game.initialize(lv);
    game_result = -1;
    autoplayer.stop();
    pending_autoplay = false;
    replay_label = "";
    effects.start_intro(glfwGetTime());
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    reshapeWindow(window, w, h);                 // phép chiếu song song theo kích thước màn mới
    camera.change_view(camera.get_view_type());  // camera "tower" tính lại theo màn mới
    menu_open = false;
}

void app_restart()
{
    app_start_level(level);
}

void app_next_level()
{
    vector<int> v = available_levels();
    if(v.empty())
        return;
    for(size_t k=0; k<v.size(); k++)
        if(v[k] > level)
        {
            app_start_level(v[k]);
            return;
        }
    app_start_level(v[0]);
}

void app_toggle_menu()
{
    if(!cli_mode)
        menu_open = !menu_open;
}

void app_solve(const string &algo)
{
    if(solve_state == SOLVE_RUNNING)
        return;
    solve_algo = algo;
    solve_state = SOLVE_RUNNING;
    solver_level = level;
    solver_done = false;
    menu_open = false;
    // Solver Python có thể chạy lâu: chạy trong luồng riêng để cửa sổ không bị treo.
    int lv = level;
    std::thread([algo, lv]() {
        solver_status = run_solver(lv, algo, "solution.txt", true);
        solver_done = true;
    }).detach();
}

void app_request_screenshot()
{
    screenshot_requested = true;
}

void app_quit()
{
    glfwSetWindowShouldClose(window, 1);
}

static void poll_solver()
{
    if(solve_state != SOLVE_RUNNING || !solver_done)
        return;
    int status = solver_status;
    if(status == SOLVER_OK && solver_level == level && autoplayer.load("solution.txt"))
    {
        app_start_level(level);
        pending_autoplay = true;
        string a = solve_algo == "astar" ? "A*" : solve_algo;
        for(size_t k=0; k<a.size(); k++) a[k] = toupper(a[k]);
        replay_label = "(" + a + ")";
        solve_state = SOLVE_IDLE;
    }
    else
    {
        solve_message = status == SOLVER_OK ? "level changed while solving" : solver_status_text(status);
        solve_state = SOLVE_FAILED;
    }
}

/**************************
 * Cập nhật và vẽ         *
 **************************/

static void on_game_end(int result, double now)
{
    if(result == 1)
    {
        superblock.start_sink(now);
        glm::vec3 goal(stage.tx*tile_side_length + 0.5f, stage.ty*tile_side_length + 0.5f, 0.2f);
        effects.burst(goal, col(GOAL_COLOR), 140, 2.6f);
    }
    else
    {
        int ix = superblock.get_ix(), iy = superblock.get_iy();
        if(superblock.get_along() == Z && stage.has_tile(ix, iy) && stage.get_tile_type(ix, iy) == fragile)
            stage.break_tile(ix, iy);   // ô cam vỡ, rơi theo khối
        superblock.start_fall(now);
    }
}

// Logic mỗi frame (thứ tự giữ như bản gốc: camera -> autoplay -> lăn khối -> kiểm tra thắng/thua).
static void update_game(double now, float dt)
{
    camera.update(dt);
    effects.update(dt);
    if(game_result == -1)
    {
        if(pending_autoplay && !effects.intro_active(now))
        {
            autoplayer.start(now);
            pending_autoplay = false;
        }
        autoplayer.update(now);
        superblock.update(dt);
        int a = game.check();
        if(a==0)
            game.gameover_fun();
        else if(a==1)
            game.won_fun();
        if(a!=-1)
        {
            game_result = a;
            game_end_time = now;
            on_game_end(a, now);
        }
    }
    poll_solver();
}

// Vẽ cảnh 3D vào vùng (x, y, w, h) của framebuffer. minimap: nhìn thẳng từ trên, phép chiếu song song.
static void render_scene(int x, int y, int w, int h, bool minimap, double now)
{
    glViewport(x, y, w, h);
    float cx = (leftt + rightt) / 2, cy = (bottomm + topp) / 2;
    glm::mat4 P, V;
    glm::vec3 eye(cx, cy, 40.0f);
    float fog_near = 1e4f, fog_far = 2e4f;
    if(minimap)
    {
        P = ortho_fit(float(w) / float(h));
        V = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 4), glm::vec3(0, 1, 0));
    }
    else
    {
        P = proj_type ? Matrices.projectionP : Matrices.projectionO;
        V = camera.get_view();
        if(proj_type)
        {
            eye = camera.get_eye();
            fog_near = 14.0f;
            fog_far = 45.0f;
        }
    }

    renderer.begin(P * V, eye, float(now), fog_near, fog_far);
    stage.draw(now);
    superblock.draw(world_model);
    renderer.flush();

    // Lượt trong suốt: không ghi depth để các lớp mờ chồng lên nhau đúng.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    superblock.draw_shadow();
    stage.draw_transparent(now);
    effects.draw();
    renderer.flush();   // thứ tự instance = thứ tự vẽ, nên các lớp mờ vẫn chồng đúng
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

static void render_frame(double now)
{
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    if(fbw <= 0 || fbh <= 0)
        return;   // cửa sổ đang thu nhỏ

    renderer.draw_calls = 0;
    renderer.boxes = 0;
    glViewport(0, 0, fbw, fbh);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.draw_background(fbw, fbh, float(now));
    render_scene(0, 0, fbw, fbh, false, now);

    if(show_minimap)
    {
        // Bản đồ nhỏ góc dưới trái (bản gốc: luôn bật, chiếm 30% cửa sổ). Viền + nền riêng bằng scissor.
        int s = int(0.26f * min(fbw, fbh));
        int x0 = 14, y0 = 14, b = 2;
        glEnable(GL_SCISSOR_TEST);
        glScissor(x0 - b, y0 - b, s + 2*b, s + 2*b);
        glClearColor(0.12f, 0.35f, 0.50f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glScissor(x0, y0, s, s);
        glClearColor(0.015f, 0.02f, 0.045f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        render_scene(x0, y0, s, s, true, now);
    }

    glViewport(0, 0, fbw, fbh);
    last_draw_calls = renderer.draw_calls;
    last_boxes = renderer.boxes;
    hud_frame(now);
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
GLFWwindow* initGLFW (int width, int height, bool vsync){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);   // khử răng cưa MSAA 4x

    window = glfwCreateWindow(width, height, "Bloxorz", NULL, NULL);

    if (!window) {
        glfwTerminate();
	    exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    gpu_name = (const char*) glGetString(GL_RENDERER);
    gpu_name = gpu_name.substr(0, gpu_name.find('/'));   // "NVIDIA ... GPU/PCIe/SSE2" -> "NVIDIA ... GPU"
    glfwSwapInterval(vsync ? 1 : 0);
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, scroll);

    return window;
}

/* Initialize the OpenGL rendering properties */
void initGL (GLFWwindow* window, int width, int height)
{
    game.initialize(level);
    renderer.init();

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    reshapeWindow (window, fbw, fbh);

    glClearDepth (1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    glEnable (GL_MULTISAMPLE);
}

// Game đọc data/ theo đường dẫn tương đối. Nếu thư mục hiện tại không có data/ (chạy từ shortcut,
// từ thư mục khác...) thì chuyển về thư mục chứa file chạy.
static void chdir_to_exe_if_needed()
{
    if(ifstream("data/1.data") || ifstream("data/" + to_string(level) + ".data"))
        return;
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(NULL, buf, MAX_PATH);
    if(n == 0 || n >= MAX_PATH)
        return;
    std::wstring p(buf, n);
    size_t k = p.find_last_of(L"\\/");
    if(k != std::wstring::npos)
        SetCurrentDirectoryW(p.substr(0, k).c_str());
#elif defined(__linux__)
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if(n <= 0)
        return;
    buf[n] = 0;
    string p(buf);
    size_t k = p.rfind('/');
    if(k != string::npos && chdir(p.substr(0, k).c_str()) != 0)
        cerr << "Cannot change directory to " << p.substr(0, k) << endl;
#endif
}

static void usage(const char *prog)
{
    cout << "Usage: " << prog << " [level] [--solve <bfs|dfs|ucs|astar> | --play <solution_file>]" << endl
         << "  level            level number, loads data/<level>.data (default " << level << ")" << endl
         << "  --solve <algo>   run the Python solver with the given algorithm, then replay its solution" << endl
         << "  --play <file>    replay a solution file containing moves U/D/L/R" << endl
         << "  (no --solve/--play: interactive mode with menu, restart and solver buttons)" << endl
         << "Developer options:" << endl
         << "  --no-vsync              render as fast as possible (FPS benchmark)" << endl
         << "  --no-menu               interactive mode without opening the menu at start" << endl
         << "  --camera <c|v|b|x|n|m>  start with this camera view" << endl
         << "  --shot <sec> <file.png> save a screenshot at <sec> seconds (repeatable)" << endl
         << "  --quit-after <sec>      close the window after <sec> seconds" << endl
         << "Python interpreter: $BLOXORZ_PYTHON (default python3, or python / py -3 on Windows)." << endl;
}

int main (int argc, char** argv)
{
#ifdef _WIN32
    // Bản phát hành (-mwindows) không có console: nếu stdout chưa được chuyển hướng thì gắn vào console
    // của terminal đã gọi nó, để các lệnh --play/--solve vẫn in WON/GAME OVER.
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    bool redirected = out != NULL && out != INVALID_HANDLE_VALUE && GetFileType(out) != FILE_TYPE_UNKNOWN;
    if(argc > 1 && !redirected && AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    // argv của MinGW theo bảng mã ANSI -> đường dẫn có dấu bị hỏng. Lấy lại dòng lệnh dạng UTF-16 rồi đổi sang UTF-8.
    vector<string> utf8_args;
    vector<char*> utf8_argv;
    int wargc = 0;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if(wargv)
    {
        for(int i=0; i<wargc; i++)
        {
            int n = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
            string a(n > 0 ? n - 1 : 0, '\0');
            if(n > 1)
                WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &a[0], n, NULL, NULL);
            utf8_args.push_back(a);
        }
        LocalFree(wargv);
        for(size_t i=0; i<utf8_args.size(); i++)
            utf8_argv.push_back(&utf8_args[i][0]);
        argc = wargc;
        argv = &utf8_argv[0];
    }
#endif

    string solve_algo_arg, play_file, camera_arg;
    bool level_given = false, vsync = true, no_menu = false;
    double quit_after = -1.0;
    vector<pair<double, string> > shots;
    for(int i=1; i<argc; i++)
    {
        string arg = argv[i];
        if(arg=="--solve" && i+1<argc)
            solve_algo_arg = argv[++i];
        else if(arg=="--play" && i+1<argc)
            play_file = argv[++i];
        else if(arg=="--no-vsync")
            vsync = false;
        else if(arg=="--no-menu")
            no_menu = true;
        else if(arg=="--camera" && i+1<argc)
            camera_arg = argv[++i];
        else if(arg=="--shot" && i+2<argc)
        {
            double t = atof(argv[i+1]);
            shots.push_back(make_pair(t, string(argv[i+2])));
            i += 2;
        }
        else if(arg=="--quit-after" && i+1<argc)
            quit_after = atof(argv[++i]);
        else if(arg=="-h" || arg=="--help")
        {
            usage(argv[0]);
            return 0;
        }
        else if(!arg.empty() && arg.find_first_not_of("0123456789")==string::npos)
        {
            level = stoi(arg);
            level_given = true;
        }
        else
        {
            usage(argv[0]);
            return 2;
        }
    }
    if(!solve_algo_arg.empty() && !play_file.empty())
    {
        cerr << "Use either --solve or --play, not both." << endl;
        return 2;
    }
    cli_mode = !solve_algo_arg.empty() || !play_file.empty();
    if(!cli_mode && !level_given)
        level = 1;   // mở như một game: bắt đầu từ màn 1

    // Đọc file lời giải trước khi (có thể) đổi thư mục, để đường dẫn tương đối vẫn đúng.
    if(!play_file.empty() && !autoplayer.load(play_file))
        return 1;
    chdir_to_exe_if_needed();

    string level_file = "data/" + to_string(level) + ".data";
    if(!ifstream(level_file))
    {
        cerr << "Cannot open " << level_file << " (run the game from the code/ directory)." << endl;
        return 2;
    }

    if(!solve_algo_arg.empty())
    {
        if(run_solver(level, solve_algo_arg, "solution.txt") != SOLVER_OK)
            return 1;
        if(!autoplayer.load("solution.txt"))
            return 1;
        solve_algo = solve_algo_arg;
    }

    world_model = glm::mat4(1.0f);
    initialize_colors();
    camera.initialize_camera();

    int width = 1280;
    int height = 800;
    // Phối cảnh (camera "follow"). Bản gốc đặt 0 nhưng bản đồ nhỏ vô tình đặt lại 1 mỗi frame.
    proj_type = 1;

    window = initGLFW(width, height, vsync);
    initGL (window, width, height);
    hud_init(window);   // sau khi đặt callback: ImGui gọi tiếp callback của game

    if(!camera_arg.empty())
        camera.change_view(GLFW_KEY_A + (toupper(camera_arg[0]) - 'A'));
    effects.start_intro(glfwGetTime());
    if(cli_mode)
    {
        pending_autoplay = true;   // bắt đầu phát lại sau hiệu ứng mở màn
        if(!solve_algo.empty())
            replay_label = "(" + string(solve_algo == "astar" ? "A*" : solve_algo) + ")";
    }
    else
        menu_open = !no_menu;

    double start_time = glfwGetTime();
    double prev_time = start_time, fps_time = start_time;
    int fps_frames = 0;
    long long total_frames = 0;

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = float(min(0.05, now - prev_time));   // kẹp dt khi kéo cửa sổ / máy khựng
        prev_time = now;

        update_game(now, dt);
        render_frame(now);

        for(size_t k=0; k<shots.size(); k++)
            if(shots[k].first >= 0 && now - start_time >= shots[k].first)
            {
                int fbw, fbh;
                glfwGetFramebufferSize(window, &fbw, &fbh);
                if(save_framebuffer_png(shots[k].second, fbw, fbh))
                    cout << "Saved screenshot " << shots[k].second << endl;
                shots[k].first = -1;
            }
        if(screenshot_requested)
        {
            int fbw, fbh;
            glfwGetFramebufferSize(window, &fbw, &fbh);
            char name[64];
            time_t t = time(NULL);
            strftime(name, sizeof(name), "screenshot_%Y%m%d_%H%M%S.png", localtime(&t));
            if(save_framebuffer_png(name, fbw, fbh))
                cout << "Saved screenshot " << name << endl;
            screenshot_requested = false;
        }

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Đo FPS, cập nhật mỗi 0.5 s lên tiêu đề cửa sổ và HUD.
        fps_frames++;
        total_frames++;
        if(now - fps_time >= 0.5)
        {
            current_fps = float(fps_frames / (now - fps_time));
            current_frame_ms = 1000.0f / max(current_fps, 0.001f);
            fps_frames = 0;
            fps_time = now;
            char title[96];
            snprintf(title, sizeof(title), "Bloxorz - Level %d - %.0f FPS", level, current_fps);
            glfwSetWindowTitle(window, title);
        }

        // Thắng/thua ở chế độ CLI: giữ màn hình một lúc (đủ xem hiệu ứng) rồi mới thoát.
        if(cli_mode && game_result != -1 && now - game_end_time >= END_HOLD_SECONDS)
            break;
        if(quit_after >= 0 && now - start_time >= quit_after)
            break;
    }

    double elapsed = glfwGetTime() - start_time;
    if(cli_mode || !vsync)
        cout << "GPU : " << gpu_name << endl << "Frames : " << total_frames << ", average FPS : " << int(total_frames / max(elapsed, 1e-6)) << endl;
    hud_shutdown();
    glfwTerminate();
    return game_result == 0 ? 1 : 0;   // mã thoát 1 nếu thua
}
