#include "headers.h"
#include "hud.h"
#include "app.h"
#include "game.h"
#include "autoplay.h"
#include "effects.h"
#include "render.h"
#include "audio.h"
#include "viz.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static ImFont *font_ui = NULL;
static ImFont *font_big = NULL;
static ImFont *font_title = NULL;
static double end_seen_at = -1.0;      // lúc HUD thấy ván kết thúc (để hiện bảng sau hiệu ứng)
static double toast_since = 0.0;

// Màu nhấn đổi theo theme (Classic: xanh lơ, Minecraft: vàng như chữ trong game).
static ImVec4 ACCENT(0.25f, 0.82f, 1.00f, 1.0f);
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

static void apply_style(int theme)
{
    ImGuiStyle &st = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    st.WindowPadding = ImVec2(16, 14);
    st.FramePadding = ImVec2(12, 7);
    st.ItemSpacing = ImVec2(10, 9);
    ImVec4 *c = st.Colors;
    if(theme == THEME_MINECRAFT)
    {
        // Kiểu giao diện Minecraft: góc vuông, viền đen dày, nút xám đá, chữ vàng.
        ACCENT = ImVec4(1.00f, 0.86f, 0.25f, 1.0f);
        st.WindowRounding = 0.0f;
        st.FrameRounding = 0.0f;
        st.GrabRounding = 0.0f;
        st.WindowBorderSize = 2.0f;
        st.FrameBorderSize = 2.0f;
        c[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.90f);
        c[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        c[ImGuiCol_Button] = ImVec4(0.43f, 0.43f, 0.43f, 1.0f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.48f, 0.53f, 0.75f, 1.0f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.36f, 0.60f, 1.0f);
        c[ImGuiCol_CheckMark] = ImVec4(0.42f, 0.85f, 0.25f, 1.0f);
        c[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.62f, 0.62f, 0.62f, 1.0f);
        c[ImGuiCol_SliderGrabActive] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
        c[ImGuiCol_Header] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.42f, 0.42f, 0.42f, 1.0f);
        c[ImGuiCol_Separator] = ImVec4(0.45f, 0.45f, 0.45f, 0.6f);
        c[ImGuiCol_TitleBg] = c[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
        return;
    }
    ACCENT = ImVec4(0.25f, 0.82f, 1.00f, 1.0f);
    st.WindowRounding = 10.0f;
    st.FrameRounding = 6.0f;
    st.GrabRounding = 6.0f;
    st.WindowBorderSize = 1.0f;
    st.FrameBorderSize = 0.0f;
    c[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.09f, 0.88f);
    c[ImGuiCol_Border] = ImVec4(0.25f, 0.82f, 1.00f, 0.25f);
    c[ImGuiCol_Button] = ImVec4(0.12f, 0.20f, 0.32f, 1.0f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.42f, 0.62f, 1.0f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.62f, 0.85f, 1.0f);
    c[ImGuiCol_CheckMark] = ACCENT;
    c[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.13f, 0.20f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.24f, 0.36f, 1.0f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.62f, 0.85f, 1.0f);
    c[ImGuiCol_SliderGrabActive] = ACCENT;
    c[ImGuiCol_Header] = ImVec4(0.12f, 0.20f, 0.32f, 1.0f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.42f, 0.62f, 1.0f);
    c[ImGuiCol_Separator] = ImVec4(0.25f, 0.82f, 1.00f, 0.20f);
}

void hud_apply_theme(int theme)
{
    if(ImGui::GetCurrentContext())
        apply_style(theme);
}

void hud_init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;   // không ghi imgui.ini cạnh file chạy
    load_fonts();
    apply_style(renderer.theme);
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
    if(r) audio_play(SFX_CLICK);
    return r;
}

static bool in_level_ui()
{
    return !cli_mode && !menu_open;
}

static const ImGuiWindowFlags OVERLAY = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
    | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

static void stats_panel()
{
    ImGui::SetNextWindowPos(ImVec2(16, 16));
    ImGui::SetNextWindowBgAlpha(0.72f);
    ImGui::Begin("##stats", NULL, OVERLAY | ImGuiWindowFlags_NoInputs);
    ImGui::PushFont(font_big);
    ImGui::TextColored(ACCENT, "BLOXORZ");
    ImGui::PopFont();
    ImGui::TextColored(DIM, "LEVEL");
    ImGui::SameLine(80);
    string title = level_title(level);
    if(title.empty()) ImGui::Text("%d", level);
    else ImGui::Text("%d  %s", level, title.c_str());
    ImGui::TextColored(DIM, "MOVES");
    ImGui::SameLine(80);
    ImGui::Text("%d", game.get_steps());
    if(!cli_mode)
    {
        ImGui::TextColored(DIM, "FALLS");
        ImGui::SameLine(80);
        ImGui::Text("%d", fall_count);
    }
    ImGui::TextColored(DIM, "MODE");
    ImGui::SameLine(80);
    if(paused)
        ImGui::TextColored(ACCENT, "Paused");
    else if(viz_is_open())
        ImGui::TextColored(ACCENT, "Search visualizer");
    else if(solve_state == SOLVE_RUNNING)
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

// Thanh công cụ giữa phía trên: New Game / Restart / Pause luôn trong tầm tay khi đang chơi.
static void toolbar(ImVec2 disp)
{
    if(!in_level_ui()) return;
    // Khi Visualizer chiếm nửa phải màn hình, đặt thanh công cụ giữa vùng 3D bên trái.
    float cx = viz_is_open() ? disp.x * 0.26f + 150 : disp.x * 0.5f;
    ImGui::SetNextWindowPos(ImVec2(cx, 14), 0, ImVec2(0.5f, 0));
    ImGui::SetNextWindowBgAlpha(0.72f);
    ImGui::Begin("##toolbar", NULL, OVERLAY);
    if(wide_button("New Game", 120)) app_open_menu();
    ImGui::SameLine();
    if(wide_button("Restart  [R]", 130)) app_restart();
    ImGui::SameLine();
    if(wide_button(paused ? "Resume  [P]" : "Pause  [P]", 130, game_result == -1)) app_toggle_pause();
    ImGui::End();
}

static void stat_row(const char *name, const char *fmt, double v)
{
    ImGui::TextColored(DIM, "%s", name);
    ImGui::SameLine(120);
    if(v < 0) ImGui::TextDisabled("-");
    else ImGui::Text(fmt, v);
}

// Bảng AI bên phải: chạy 4 thuật toán ngay trong màn, chọn heuristic cho A*, mở Visualizer, xem 4 chỉ số.
static void ai_panel(ImVec2 disp)
{
    if(!in_level_ui() || viz_is_open() || paused) return;
    ImGui::SetNextWindowPos(ImVec2(disp.x - 16, 16), 0, ImVec2(1, 0));
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    ImGui::SetNextWindowBgAlpha(0.78f);
    ImGui::Begin("##ai", NULL, OVERLAY & ~ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextColored(ACCENT, "AI SOLVER");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40);
    if(ImGui::SmallButton(show_ai_panel ? "hide" : "show")) { show_ai_panel = !show_ai_panel; app_save_settings(); }
    if(!show_ai_panel) { ImGui::End(); return; }

    bool idle = solve_state != SOLVE_RUNNING;
    float full = ImGui::GetContentRegionAvail().x;
    float bw = (full - 3 * ImGui::GetStyle().ItemSpacing.x) / 4;
    static const char *algos[4] = {"bfs", "dfs", "ucs", "astar"};
    for(int k = 0; k < 4; k++)
    {
        if(k) ImGui::SameLine();
        if(wide_button(algo_label(algos[k]), bw, idle)) app_solve(algos[k]);
    }
    ImGui::TextColored(DIM, "Solve this level, then replay the moves.");
    ImGui::TextColored(DIM, "A* heuristic");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    int h = solve_heuristic == "zero" ? 1 : 0;
    if(ImGui::Combo("##heur", &h, "Manhattan (split-aware)\0Zero (= UCS)\0"))
    {
        solve_heuristic = h == 1 ? "zero" : "manhattan";
        app_save_settings();
    }
    if(wide_button("Visualize search  >", full, idle)) app_visualize(solve_algo.empty() ? "bfs" : solve_algo);
    if(ImGui::IsItemHovered())
        ImGui::SetTooltip("Shows the search tree step by step and a heat map of the explored cells.");

    if(last_stats.valid)
    {
        ImGui::Separator();
        ImGui::TextColored(ACCENT, "Last run: %s", algo_label(last_stats.algorithm));
        ImGui::SameLine();
        ImGui::TextColored(last_stats.status == "ok" ? GOOD : BAD, "%s", last_stats.status.c_str());
        stat_row("Solution", "%.0f moves", last_stats.length);
        stat_row("Cost", "%g", last_stats.length < 0 ? -1.0 : last_stats.cost);
        stat_row("Expanded", "%.0f nodes", (double)last_stats.expanded);
        stat_row("Max frontier", "%.0f", (double)last_stats.frontier);
        stat_row("Search time", "%.2f ms", last_stats.time_ms);
        stat_row("Peak memory", "%.1f KB", last_stats.memory_kb);
    }
    ImGui::End();
}

static void hint_bar(ImVec2 disp)
{
    if(!show_help || viz_is_open())
        return;
    ImGui::SetNextWindowPos(ImVec2(disp.x - 16, disp.y - 16), 0, ImVec2(1, 1));
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("##hints", NULL, OVERLAY | ImGuiWindowFlags_NoInputs);
    const char *rows[][2] = {
        {"Arrows", "roll the block"},
        {"Space", "switch cube (after split)"},
        {"R / P", "restart / pause"},
        {"C V B X N M", "camera views"},
        {"Tab", "mini-map"},
        {"Esc", cli_mode ? "quit" : "pause"},
        {"F1 / F12", "hide help / screenshot"},
    };
    for(size_t k=0; k<sizeof(rows)/sizeof(rows[0]); k++)
    {
        if(cli_mode && k==2) continue;
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
    ImGui::SetNextWindowPos(ImVec2(disp.x/2, 70), 0, ImVec2(0.5f, 0));
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

// Rơi khi đang chơi tay: sàn sụp rồi tự chơi lại (như Bloxorz gốc), chỉ hiện một dòng thông báo.
static void fall_toast(ImVec2 disp)
{
    if(cli_mode || game_result != 0) return;
    ImGui::SetNextWindowPos(ImVec2(disp.x/2, disp.y*0.30f), 0, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin("##fall", NULL, OVERLAY | ImGuiWindowFlags_NoInputs);
    ImGui::PushFont(font_big);
    ImGui::TextColored(BAD, "Oops! The block fell off.");
    ImGui::PopFont();
    ImGui::TextColored(DIM, "Restarting level %d...  (falls: %d)", level, fall_count);
    ImGui::End();
}

static void end_panel(ImVec2 disp, double now)
{
    if(game_result == -1 || (game_result == 0 && !cli_mode))
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
        if(wide_button("Menu", w)) app_open_menu();
    }
    ImGui::End();
}

static void options_panel(float full)
{
    ImGui::SeparatorText("OPTIONS");
    ImGui::TextColored(DIM, "Theme");
    ImGui::SameLine(110);
    if(ImGui::RadioButton("Classic", renderer.theme == THEME_CLASSIC)) { app_set_theme(THEME_CLASSIC); app_save_settings(); }
    ImGui::SameLine();
    if(ImGui::RadioButton("Minecraft", renderer.theme == THEME_MINECRAFT)) { app_set_theme(THEME_MINECRAFT); app_save_settings(); }
    ImGui::TextColored(DIM, "Music");
    ImGui::SameLine(110);
    ImGui::SetNextItemWidth(full - 110);
    if(ImGui::SliderFloat("##music", &music_volume, 0.0f, 1.0f, "%.2f")) app_apply_volumes();
    if(ImGui::IsItemDeactivatedAfterEdit()) app_save_settings();
    ImGui::TextColored(DIM, "Sounds");
    ImGui::SameLine(110);
    ImGui::SetNextItemWidth(full - 110);
    if(ImGui::SliderFloat("##sfx", &sfx_volume, 0.0f, 1.0f, "%.2f")) app_apply_volumes();
    if(ImGui::IsItemDeactivatedAfterEdit()) { app_save_settings(); audio_play(SFX_SWITCH); }
    ImGui::TextColored(DIM, "Audio: %s", audio_status());
    bool changed = false;
    changed |= ImGui::Checkbox("Mini-map", &show_minimap);
    ImGui::SameLine(150);
    changed |= ImGui::Checkbox("Effects", &effects.enabled);
    ImGui::SameLine(280);
    changed |= ImGui::Checkbox("Hints", &show_help);
    if(changed) app_save_settings();
}

static void pause_menu(ImVec2 disp)
{
    if(!paused || cli_mode || menu_open) return;
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), disp, IM_COL32(0, 0, 0, 120));
    ImGui::SetNextWindowPos(ImVec2(disp.x/2, disp.y/2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 0));
    ImGui::Begin("##pause", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings
                 | ImGuiWindowFlags_AlwaysAutoResize);
    float full = ImGui::GetContentRegionAvail().x;
    ImGui::PushFont(font_title);
    float tw = ImGui::CalcTextSize("PAUSED").x;
    ImGui::SetCursorPosX((full - tw) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(ACCENT, "PAUSED");
    ImGui::PopFont();
    char sub[96];
    string title = level_title(level);
    snprintf(sub, sizeof(sub), "Level %d%s%s  -  %d moves", level, title.empty() ? "" : ": ", title.c_str(), game.get_steps());
    ImGui::SetCursorPosX((full - ImGui::CalcTextSize(sub).x) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(DIM, "%s", sub);
    ImGui::Spacing();
    if(wide_button("Resume  [P / Esc]", full)) app_toggle_pause();
    if(wide_button("Restart level  [R]", full)) app_restart();
    if(wide_button("New Game - select level", full)) app_open_menu();
    if(wide_button(options_open ? "Hide options" : "Options", full)) options_open = !options_open;
    if(options_open) options_panel(full);
    ImGui::Spacing();
    if(wide_button("Quit", full)) app_quit();
    ImGui::End();
}

static void main_menu(ImVec2 disp)
{
    if(!menu_open)
        return;
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), disp, IM_COL32(0, 0, 0, 150));

    ImGui::SetNextWindowPos(ImVec2(disp.x/2, disp.y/2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 0));
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
        if(wide_button("Continue  [Esc]", full)) app_toggle_menu();

    ImGui::SeparatorText("NEW GAME - SELECT LEVEL");
    vector<int> levels = available_levels();
    float bw = 64.0f;
    int per_row = max(1, int((full + ImGui::GetStyle().ItemSpacing.x) / (bw + ImGui::GetStyle().ItemSpacing.x)));
    for(size_t k=0; k<levels.size(); k++)
    {
        if(k % per_row) ImGui::SameLine();
        bool cur = levels[k] == level;
        if(cur) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.80f, 1.0f));
        char label[16];
        snprintf(label, sizeof(label), "%d", levels[k]);
        if(ImGui::Button(label, ImVec2(bw, 46)))
        {
            audio_play(SFX_CLICK);
            app_start_level(levels[k]);
        }
        if(ImGui::IsItemHovered())
        {
            string t = level_title(levels[k]), g = level_group(levels[k]);
            if(!t.empty()) ImGui::SetTooltip("%s  (%s)", t.c_str(), g.c_str());
        }
        if(cur) ImGui::PopStyleColor();
    }
    ImGui::TextColored(DIM, "In a level: AI solver panel on the right, toolbar on top.");

    options_panel(full);

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
    {
        toolbar(disp);
        ai_panel(disp);
        hint_bar(disp);
        fall_toast(disp);
        end_panel(disp, now);
    }
    solver_toast(disp, now);
    if(viz_is_open() && !menu_open)
        viz_draw_window(disp.x * 0.52f + 8, 12, disp.x * 0.48f - 20, disp.y - 24);
    pause_menu(disp);
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
