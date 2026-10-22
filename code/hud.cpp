#include "headers.h"
#include "hud.h"
#include "app.h"
#include "game.h"
#include "autoplay.h"
#include "effects.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static ImFont *font_ui = NULL;
static ImFont *font_big = NULL;
static ImFont *font_title = NULL;
static double end_seen_at = -1.0;      // lúc HUD thấy ván kết thúc (để hiện bảng sau hiệu ứng)
static double toast_since = 0.0;

static const ImVec4 ACCENT(0.25f, 0.82f, 1.00f, 1.0f);
static const ImVec4 DIM(0.62f, 0.66f, 0.74f, 1.0f);
static const ImVec4 GOOD(0.35f, 1.00f, 0.55f, 1.0f);
static const ImVec4 BAD(1.00f, 0.38f, 0.32f, 1.0f);

static const char *first_existing(const char *const *paths)
{
    for(int k=0; paths[k]; k++)
        if(ifstream(paths[k]))
            return paths[k];
    return NULL;
}

static void load_fonts()
{
    ImGuiIO &io = ImGui::GetIO();
    static const char *const regular[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        NULL };
    static const char *const bold[] = {
        "C:/Windows/Fonts/segoeuib.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
        NULL };
    const char *r = first_existing(regular);
    const char *b = first_existing(bold);
    if(!b) b = r;
    if(r)
    {
        font_ui = io.Fonts->AddFontFromFileTTF(r, 18.0f);
        font_big = io.Fonts->AddFontFromFileTTF(b, 30.0f);
        font_title = io.Fonts->AddFontFromFileTTF(b, 54.0f);
    }
    if(!font_ui || !font_big || !font_title)
    {
        // Không có font hệ thống: dùng font mặc định của ImGui, phóng to bằng FontGlobalScale từng cửa sổ.
        io.Fonts->Clear();
        font_ui = font_big = font_title = io.Fonts->AddFontDefault();
    }
}

static void apply_style()
{
    ImGuiStyle &st = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    st.WindowRounding = 10.0f;
    st.FrameRounding = 6.0f;
    st.GrabRounding = 6.0f;
    st.WindowBorderSize = 1.0f;
    st.WindowPadding = ImVec2(16, 14);
    st.FramePadding = ImVec2(12, 7);
    st.ItemSpacing = ImVec2(10, 9);
    ImVec4 *c = st.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.09f, 0.88f);
    c[ImGuiCol_Border] = ImVec4(0.25f, 0.82f, 1.00f, 0.25f);
    c[ImGuiCol_Button] = ImVec4(0.12f, 0.20f, 0.32f, 1.0f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.42f, 0.62f, 1.0f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.62f, 0.85f, 1.0f);
    c[ImGuiCol_CheckMark] = ACCENT;
    c[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.13f, 0.20f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.24f, 0.36f, 1.0f);
    c[ImGuiCol_Header] = ImVec4(0.12f, 0.20f, 0.32f, 1.0f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.42f, 0.62f, 1.0f);
    c[ImGuiCol_Separator] = ImVec4(0.25f, 0.82f, 1.00f, 0.20f);
}

void hud_init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;   // không ghi imgui.ini cạnh file chạy
    load_fonts();
    apply_style();
    // install_callbacks = true: ImGui gắn callback của nó và gọi tiếp các callback đã đặt trước (phím, chuột).
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

bool hud_wants_keyboard()
{
    return ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
}

bool hud_wants_mouse()
{
    return ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
}

static const char *algo_label(const string &a)
{
    if(a=="bfs") return "BFS";
    if(a=="dfs") return "DFS";
    if(a=="ucs") return "UCS";
    if(a=="astar") return "A*";
    return a.c_str();
}

static bool wide_button(const char *label, float w, bool enabled = true)
{
    if(!enabled) ImGui::BeginDisabled();
    bool r = ImGui::Button(label, ImVec2(w, 0));
    if(!enabled) ImGui::EndDisabled();
    return r;
}

static void stats_panel()
{
    const ImGuiWindowFlags overlay = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoInputs;
    ImGui::SetNextWindowPos(ImVec2(16, 16));
    ImGui::SetNextWindowBgAlpha(0.72f);
    ImGui::Begin("##stats", NULL, overlay);
    ImGui::PushFont(font_big);
    ImGui::TextColored(ACCENT, "BLOXORZ");
    ImGui::PopFont();
    ImGui::TextColored(DIM, "LEVEL");
    ImGui::SameLine(80);
    ImGui::Text("%d", level);
    ImGui::TextColored(DIM, "MOVES");
    ImGui::SameLine(80);
    ImGui::Text("%d", game.get_steps());
    ImGui::TextColored(DIM, "MODE");
    ImGui::SameLine(80);
    if(solve_state == SOLVE_RUNNING)
        ImGui::TextColored(ACCENT, "Solving (%s)", algo_label(solve_algo));
    else if(autoplayer.total() > 0 && (autoplayer.is_active() || autoplayer.played() > 0))
        ImGui::Text("Replay %d/%d %s", (int)autoplayer.played(), (int)autoplayer.total(), replay_label.c_str());
    else
        ImGui::Text("Manual");
    ImGui::Separator();
    if(current_fps > 0.0f)
        ImGui::TextColored(DIM, "%.0f FPS  %.1f ms", current_fps, current_frame_ms);
    else
        ImGui::TextColored(DIM, "-- FPS");
    ImGui::TextColored(DIM, "%d boxes in %d draw calls", last_boxes, last_draw_calls);
    if(show_help)
        ImGui::TextColored(DIM, "%s", gpu_name.c_str());
    ImGui::End();
}

static void hint_bar(ImVec2 disp)
{
    if(!show_help)
        return;
    const ImGuiWindowFlags overlay = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoInputs;
    ImGui::SetNextWindowPos(ImVec2(disp.x - 16, disp.y - 16), 0, ImVec2(1, 1));
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("##hints", NULL, overlay);
    const char *rows[][2] = {
        {"Arrows", "roll the block"},
        {"R / Enter", "restart / next level"},
        {"C V B X N M", "camera views"},
        {"Tab", "mini-map"},
        {"Esc", cli_mode ? "quit" : "menu"},
        {"F1 / F12", "hide help / screenshot"},
    };
    for(size_t k=0; k<sizeof(rows)/sizeof(rows[0]); k++)
    {
        if(cli_mode && k==1) continue;
        ImGui::TextColored(ACCENT, "%s", rows[k][0]);
        ImGui::SameLine(120);
        ImGui::TextColored(DIM, "%s", rows[k][1]);
    }
    ImGui::End();
}

static void solver_toast(ImVec2 disp, double now)
{
    if(solve_state == SOLVE_IDLE)
        return;
    ImGui::SetNextWindowPos(ImVec2(disp.x/2, 16), 0, ImVec2(0.5f, 0));
    ImGuiWindowFlags f = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::Begin("##toast", NULL, f);
    if(solve_state == SOLVE_RUNNING)
    {
        int dots = int(now * 3.0) % 4;
        ImGui::TextColored(ACCENT, "Solving level %d with %s%s", level, algo_label(solve_algo), string(dots, '.').c_str());
        ImGui::TextColored(DIM, "The window stays responsive while Python searches.");
        toast_since = now;
    }
    else
    {
        ImGui::TextColored(BAD, "%s: %s", algo_label(solve_algo), solve_message.c_str());
        if(ImGui::Button("OK") || now - toast_since > 8.0)
            solve_state = SOLVE_IDLE;
    }
    ImGui::End();
}

static void end_panel(ImVec2 disp, double now)
{
    if(game_result == -1)
    {
        end_seen_at = -1.0;
        return;
    }
    if(end_seen_at < 0)
        end_seen_at = now;
    float t = float(now - end_seen_at);
    if(t < 0.5f)
        return;   // chờ hiệu ứng chìm/rơi chạy trước

    bool won = game_result == 1;
    ImGui::SetNextWindowPos(ImVec2(disp.x/2, disp.y*0.38f), 0, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.9f * clamp01((t - 0.5f) * 4.0f));
    ImGuiWindowFlags f = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
    if(cli_mode) f |= ImGuiWindowFlags_NoInputs;
    ImGui::Begin("##end", NULL, f);
    ImGui::PushFont(font_title);
    const char *title = won ? "LEVEL COMPLETE" : "GAME OVER";
    float tw = max(ImGui::CalcTextSize(title).x, 360.0f);   // đủ rộng cho 3 nút bên dưới
    ImGui::SetCursorPosX((tw - ImGui::CalcTextSize(title).x) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(won ? GOOD : BAD, "%s", title);
    ImGui::PopFont();
    ImGui::PushFont(font_big);
    char line[64];
    snprintf(line, sizeof(line), "%d move%s", game.get_steps(), game.get_steps() == 1 ? "" : "s");
    ImGui::SetCursorPosX((tw - ImGui::CalcTextSize(line).x) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(DIM, "%s", line);
    ImGui::PopFont();
    if(!cli_mode)
    {
        ImGui::Spacing();
        float w = (tw - 2*ImGui::GetStyle().ItemSpacing.x) / 3;
        if(wide_button("Retry  [R]", w)) app_restart();
        ImGui::SameLine();
        if(wide_button("Next  [Enter]", w)) app_next_level();
        ImGui::SameLine();
        if(wide_button("Menu  [Esc]", w)) app_toggle_menu();
    }
    ImGui::End();
}

static void main_menu(ImVec2 disp)
{
    if(!menu_open)
        return;
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), disp, IM_COL32(0, 0, 0, 150));

    ImGui::SetNextWindowPos(ImVec2(disp.x/2, disp.y/2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440, 0));
    ImGuiWindowFlags f = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("##menu", NULL, f);
    float full = ImGui::GetContentRegionAvail().x;

    ImGui::PushFont(font_title);
    float tw = ImGui::CalcTextSize("BLOXORZ").x;
    ImGui::SetCursorPosX((full - tw) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(ACCENT, "BLOXORZ");
    ImGui::PopFont();
    const char *sub = "Roll the block and stand it in the square hole";
    ImGui::SetCursorPosX((full - ImGui::CalcTextSize(sub).x) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(DIM, "%s", sub);
    ImGui::Spacing();

    if(game_result == -1 && game.get_steps() > 0)
        if(wide_button("Resume  [Esc]", full)) app_toggle_menu();

    ImGui::SeparatorText("SELECT LEVEL");
    vector<int> levels = available_levels();
    float bw = 62.0f;
    int per_row = max(1, int((full + ImGui::GetStyle().ItemSpacing.x) / (bw + ImGui::GetStyle().ItemSpacing.x)));
    for(size_t k=0; k<levels.size(); k++)
    {
        if(k % per_row) ImGui::SameLine();
        bool cur = levels[k] == level;
        if(cur) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.80f, 1.0f));
        char label[16];
        snprintf(label, sizeof(label), "%d", levels[k]);
        if(ImGui::Button(label, ImVec2(bw, 44)))
            app_start_level(levels[k]);
        if(cur) ImGui::PopStyleColor();
    }

    char head[64];
    snprintf(head, sizeof(head), "AI SOLVER  (level %d)", level);
    ImGui::SeparatorText(head);
    const char *algos[4] = {"bfs", "dfs", "ucs", "astar"};
    float aw = (full - 3*ImGui::GetStyle().ItemSpacing.x) / 4;
    for(int k=0; k<4; k++)
    {
        if(k) ImGui::SameLine();
        if(wide_button(algo_label(algos[k]), aw, solve_state != SOLVE_RUNNING))
            app_solve(algos[k]);
    }
    ImGui::TextColored(DIM, "Runs the Python solver, then replays the solution.");

    ImGui::SeparatorText("OPTIONS");
    ImGui::Checkbox("Mini-map", &show_minimap);
    ImGui::SameLine(150);
    ImGui::Checkbox("Effects", &effects.enabled);
    ImGui::SameLine(280);
    ImGui::Checkbox("Hints", &show_help);

    ImGui::Spacing();
    if(wide_button("Quit", full)) app_quit();
    ImGui::Spacing();
    ImGui::TextColored(DIM, "CSC14003 - Group 9  |  based on chaitanya100100/Bloxorz");
    ImGui::End();
}

void hud_frame(double now)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(font_ui);

    ImVec2 disp = ImGui::GetIO().DisplaySize;
    stats_panel();
    if(!menu_open)
        hint_bar(disp);
    solver_toast(disp, now);
    if(!menu_open)
        end_panel(disp, now);
    main_menu(disp);

    ImGui::PopFont();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void hud_shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
