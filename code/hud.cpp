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

#include <cfloat>

static const float HUD_PI = 3.14159265f;

static ImFont *font_ui = NULL;
static ImFont *font_small = NULL;
static ImFont *font_big = NULL;
static ImFont *font_title = NULL;
static ImFont *font_menu = NULL;      // mục menu chính
static ImFont *font_logo = NULL;      // chữ BLOXORZ lớn
static ImFont *font_display = NULL;   // chữ khối kiểu game gốc (HUD theme Original)
static double end_seen_at = -1.0;      // lúc HUD thấy ván kết thúc (để hiện bảng sau hiệu ứng)
static double toast_since = 0.0;

// Màu nhấn đổi theo theme (Classic: xanh lơ, Original: cam, Minecraft: vàng như chữ trong game).
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
    static const char *const menu[] = {      // chữ hẹp, hiện đại cho menu
        "C:/Windows/Fonts/bahnschrift.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSansCondensed-Bold.ttf",
        NULL };
    static const char *const heavy[] = {     // chữ rất đậm cho logo và HUD kiểu game gốc
        "C:/Windows/Fonts/ariblk.ttf",
        "C:/Windows/Fonts/impact.ttf",
        "/System/Library/Fonts/Supplemental/Arial Black.ttf",
        NULL };
    const char *r = first_existing(regular);
    const char *b = first_existing(bold);
    if(!b) b = r;
    const char *mn = first_existing(menu);
    if(!mn) mn = b;
    const char *hv = first_existing(heavy);
    if(!hv) hv = b;
    if(r)
    {
        font_ui = io.Fonts->AddFontFromFileTTF(r, 18.0f);
        font_small = io.Fonts->AddFontFromFileTTF(r, 15.0f);
        font_big = io.Fonts->AddFontFromFileTTF(b, 30.0f);
        font_title = io.Fonts->AddFontFromFileTTF(b, 54.0f);
        font_menu = io.Fonts->AddFontFromFileTTF(mn, 28.0f);
        font_logo = io.Fonts->AddFontFromFileTTF(hv, 96.0f);
        font_display = io.Fonts->AddFontFromFileTTF(hv, 26.0f);
    }
    if(!font_ui || !font_small || !font_big || !font_title || !font_menu || !font_logo || !font_display)
    {
        // Không có font hệ thống: dùng font mặc định của ImGui.
        io.Fonts->Clear();
        font_ui = font_small = font_big = font_title = font_menu = font_logo = font_display = io.Fonts->AddFontDefault();
    }
}

static void apply_style(int theme)
{
    ImGuiStyle &st = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    st.WindowPadding = ImVec2(16, 14);
    st.FramePadding = ImVec2(12, 7);
    st.ItemSpacing = ImVec2(10, 9);
    st.ScrollbarSize = 12.0f;
    ImVec4 *c = st.Colors;
    if(theme == THEME_MINECRAFT)
    {
        // Kiểu giao diện Minecraft: góc vuông, viền đen dày, nút xám đá, chữ vàng.
        ACCENT = ImVec4(1.00f, 0.86f, 0.25f, 1.0f);
        st.WindowRounding = 0.0f;
        st.FrameRounding = 0.0f;
        st.GrabRounding = 0.0f;
        st.ChildRounding = 0.0f;
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
    if(theme == THEME_ORIGINAL)
    {
        // Kiểu game gốc: bảng nâu đỏ trong suốt, nút cam đất, chữ trắng.
        ACCENT = ImVec4(1.00f, 0.72f, 0.28f, 1.0f);
        st.WindowRounding = 6.0f;
        st.FrameRounding = 4.0f;
        st.GrabRounding = 4.0f;
        st.ChildRounding = 6.0f;
        st.WindowBorderSize = 1.0f;
        st.FrameBorderSize = 1.0f;
        c[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.04f, 0.02f, 0.82f);
        c[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        c[ImGuiCol_Border] = ImVec4(1.00f, 0.62f, 0.25f, 0.35f);
        c[ImGuiCol_Button] = ImVec4(0.46f, 0.17f, 0.05f, 1.0f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.72f, 0.30f, 0.08f, 1.0f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.92f, 0.45f, 0.12f, 1.0f);
        c[ImGuiCol_CheckMark] = ACCENT;
        c[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.08f, 0.04f, 1.0f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.34f, 0.13f, 0.05f, 1.0f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.92f, 0.48f, 0.15f, 1.0f);
        c[ImGuiCol_SliderGrabActive] = ACCENT;
        c[ImGuiCol_Header] = ImVec4(0.46f, 0.17f, 0.05f, 1.0f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.72f, 0.30f, 0.08f, 1.0f);
        c[ImGuiCol_Separator] = ImVec4(1.00f, 0.62f, 0.25f, 0.25f);
        c[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.05f, 0.02f, 0.96f);
        return;
    }
    ACCENT = ImVec4(0.25f, 0.82f, 1.00f, 1.0f);
    st.WindowRounding = 10.0f;
    st.FrameRounding = 6.0f;
    st.GrabRounding = 6.0f;
    st.ChildRounding = 8.0f;
    st.WindowBorderSize = 1.0f;
    st.FrameBorderSize = 0.0f;
    c[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.09f, 0.88f);
    c[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
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

static ImU32 rgba(const ImVec4 &c, float a = 1.0f)
{
    return ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * a));
}

static ImVec4 mix4(const ImVec4 &a, const ImVec4 &b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

// Chữ có viền (kiểu chữ HUD của game gốc: trắng viền đen).
static void text_outlined(ImDrawList *dl, ImFont *f, float size, ImVec2 p, ImU32 col, ImU32 outline, float t, const char *s)
{
    for(int dx=-1; dx<=1; dx++)
        for(int dy=-1; dy<=1; dy++)
            if(dx || dy)
                dl->AddText(f, size, ImVec2(p.x + dx*t, p.y + dy*t), outline, s);
    dl->AddText(f, size, p, col, s);
}

static ImVec2 text_size(ImFont *f, float size, const char *s)
{
    return f->CalcTextSizeA(size, FLT_MAX, 0.0f, s);
}

static const ImGuiWindowFlags OVERLAY = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
    | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

static const char *mode_text(char *buf, size_t n)
{
    if(paused) snprintf(buf, n, "Paused");
    else if(viz_is_open()) snprintf(buf, n, "Search visualizer");
    else if(solve_state == SOLVE_RUNNING) snprintf(buf, n, "Solving (%s)", algo_label(solve_algo));
    else if(autoplayer.total() > 0 && (autoplayer.is_active() || autoplayer.played() > 0))
        snprintf(buf, n, "Replay %d/%d %s", (int)autoplayer.played(), (int)autoplayer.total(), replay_label.c_str());
    else snprintf(buf, n, "Manual");
    return buf;
}

// Theme Original: chỉ số nằm thẳng trên cảnh bằng chữ khối trắng viền đen, giống "Moves / Passcode" của game gốc.
static void stats_original(double now)
{
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImU32 white = IM_COL32(255, 255, 255, 255), black = IM_COL32(0, 0, 0, 230);
    float x = 22, y = 16, fs = font_display->FontSize;
    char line[160];
    string title = level_title(level);
    snprintf(line, sizeof(line), "Stage %02d", level);
    text_outlined(dl, font_display, fs * 1.25f, ImVec2(x, y), white, black, 2.0f, line);
    if(!title.empty())
        text_outlined(dl, font_ui, font_ui->FontSize, ImVec2(x + text_size(font_display, fs * 1.25f, line).x + 14, y + 12),
                      IM_COL32(255, 205, 140, 255), black, 1.0f, title.c_str());
    y += fs * 1.25f + 6;
    snprintf(line, sizeof(line), "Moves:  %d", game.get_steps());
    text_outlined(dl, font_display, fs, ImVec2(x, y), white, black, 2.0f, line);
    y += fs + 4;
    if(!cli_mode)
    {
        snprintf(line, sizeof(line), "Falls:  %d", fall_count);
        text_outlined(dl, font_display, fs, ImVec2(x, y), white, black, 2.0f, line);
        y += fs + 4;
    }
    char mode[96];
    mode_text(mode, sizeof(mode));
    text_outlined(dl, font_ui, font_ui->FontSize, ImVec2(x, y + 2), IM_COL32(255, 205, 140, 255), black, 1.0f, mode);
    y += font_ui->FontSize + 6;
    if(current_fps > 0.0f)
    {
        snprintf(line, sizeof(line), "%.0f FPS  %.1f ms  |  %d boxes, %d draw calls", current_fps, current_frame_ms,
                 last_boxes, last_draw_calls);
        text_outlined(dl, font_small, font_small->FontSize, ImVec2(x, y), IM_COL32(255, 255, 255, 170), IM_COL32(0, 0, 0, 140), 1.0f, line);
    }
    (void)now;
}

static void stats_panel(double now)
{
    if(menu_open && !cli_mode) return;
    if(renderer.theme == THEME_ORIGINAL)
    {
        stats_original(now);
        return;
    }
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
        if(best_moves.count(level))
        {
            ImGui::TextColored(DIM, "BEST");
            ImGui::SameLine(80);
            ImGui::Text("%d moves", best_moves[level]);
        }
    }
    ImGui::TextColored(DIM, "MODE");
    ImGui::SameLine(80);
    char mode[96];
    mode_text(mode, sizeof(mode));
    if(string(mode) == "Manual") ImGui::Text("%s", mode);
    else ImGui::TextColored(ACCENT, "%s", mode);
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

// Tên màn hiện lên giữa màn hình khi bắt đầu (như đoạn "Stage N" của game gốc).
static void stage_banner(ImVec2 disp, double now)
{
    float t = float(now - level_started_at);
    if(menu_open || paused || t < 0.0f || t > 2.4f) return;
    float a = clamp01(t / 0.25f) * clamp01((2.4f - t) / 0.6f);
    float slide = (1.0f - ease_out_cubic(t / 0.45f)) * 30.0f;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();   // sau các bảng HUD, trước cảnh 3D
    char head[32];
    snprintf(head, sizeof(head), "STAGE %02d", level);
    float fs = font_logo->FontSize * 0.72f;
    ImVec2 hs = text_size(font_logo, fs, head);
    ImVec2 p(disp.x * 0.5f - hs.x * 0.5f, disp.y * 0.24f - slide);
    ImU32 face = renderer.theme == THEME_ORIGINAL ? IM_COL32(255, 255, 255, (int)(255 * a)) : rgba(ACCENT, a);
    text_outlined(dl, font_logo, fs, p, face, IM_COL32(0, 0, 0, (int)(220 * a)), 3.0f, head);
    string title = level_title(level);
    if(!title.empty())
    {
        float ts = font_big->FontSize;
        ImVec2 tsz = text_size(font_big, ts, title.c_str());
        text_outlined(dl, font_big, ts, ImVec2(disp.x * 0.5f - tsz.x * 0.5f, p.y + hs.y + 2), IM_COL32(255, 255, 255, (int)(235 * a)),
                      IM_COL32(0, 0, 0, (int)(200 * a)), 2.0f, title.c_str());
    }
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
    if(wide_button("Menu", 90)) app_open_menu();
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
    if(!show_help || viz_is_open() || (menu_open && !cli_mode))
        return;
    ImGui::SetNextWindowPos(ImVec2(disp.x - 16, disp.y - 16), 0, ImVec2(1, 1));
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("##hints", NULL, OVERLAY | ImGuiWindowFlags_NoInputs);
    const char *rows[][2] = {
        {"Arrows", "roll the block"},
        {"Space", "switch cube (after split)"},
        {"R / P", "restart / pause"},
        {"C B N M X V", "camera views"},
        {"Drag / Wheel", "rotate / zoom camera"},
        {"Tab", "mini-map"},
        {"Esc", cli_mode ? "quit" : "pause"},
        {"F1 / F12", "hide help / screenshot"},
    };
    for(size_t k=0; k<sizeof(rows)/sizeof(rows[0]); k++)
    {
        if(cli_mode && k==2) continue;
        ImGui::TextColored(ACCENT, "%s", rows[k][0]);
        ImGui::SameLine(130);
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
    char line[96];
    snprintf(line, sizeof(line), "%d move%s", game.get_steps(), game.get_steps() == 1 ? "" : "s");
    ImGui::SetCursorPosX((tw - ImGui::CalcTextSize(line).x) / 2 + ImGui::GetStyle().WindowPadding.x);
    ImGui::TextColored(DIM, "%s", line);
    ImGui::PopFont();
    if(won && !cli_mode && best_moves.count(level))
    {
        snprintf(line, sizeof(line), "Best: %d moves%s", best_moves[level],
                 best_moves[level] == game.get_steps() && autoplayer.played() == 0 ? "  - new record!" : "");
        ImGui::SetCursorPosX((tw - ImGui::CalcTextSize(line).x) / 2 + ImGui::GetStyle().WindowPadding.x);
        ImGui::TextColored(ACCENT, "%s", line);
    }
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

/****************************************
 *  Ô gạch thu nhỏ (chú giải + bản đồ)  *
 ****************************************/

enum MiniTile { MT_NORMAL = 1, MT_FRAGILE, MT_SOFT, MT_HEAVY, MT_SPLIT, MT_BRIDGE, MT_GOAL, MT_START, MT_CLOSED };

struct MiniPalette { ImU32 tile, edge, fragile, mark, split, bridge, hole, block, bg; };

static MiniPalette mini_palette()
{
    MiniPalette p;
    if(renderer.theme == THEME_ORIGINAL)
        p = {IM_COL32(196, 200, 210, 255), IM_COL32(90, 92, 100, 255), IM_COL32(240, 130, 36, 255), IM_COL32(120, 122, 130, 255),
             IM_COL32(150, 152, 160, 255), IM_COL32(176, 170, 160, 255), IM_COL32(40, 30, 20, 255), IM_COL32(150, 72, 30, 255),
             IM_COL32(40, 8, 4, 150)};
    else if(renderer.theme == THEME_MINECRAFT)
        p = {IM_COL32(95, 160, 64, 255), IM_COL32(60, 40, 24, 255), IM_COL32(219, 125, 54, 255), IM_COL32(128, 128, 128, 255),
             IM_COL32(20, 30, 40, 255), IM_COL32(173, 132, 79, 255), IM_COL32(10, 10, 10, 255), IM_COL32(200, 40, 30, 255),
             IM_COL32(20, 30, 50, 150)};
    else
        p = {IM_COL32(128, 140, 163, 255), IM_COL32(40, 46, 60, 255), IM_COL32(242, 122, 36, 255), IM_COL32(64, 255, 115, 255),
             IM_COL32(51, 178, 242, 255), IM_COL32(184, 158, 97, 255), IM_COL32(8, 16, 26, 255), IM_COL32(200, 31, 26, 255),
             IM_COL32(4, 6, 14, 160)};
    return p;
}

// Vẽ một ô (vuông cạnh s, góc trên trái p) theo loại ô.
static void draw_mini_tile(ImDrawList *dl, ImVec2 p, float s, int type, const MiniPalette &c)
{
    ImVec2 q(p.x + s, p.y + s), m(p.x + s * 0.5f, p.y + s * 0.5f);
    float g = max(1.0f, s * 0.06f), r = s * 0.12f;
    if(type == MT_CLOSED)
    {
        dl->AddRect(ImVec2(p.x + g, p.y + g), ImVec2(q.x - g, q.y - g), c.bridge, r, 0, max(1.0f, s * 0.07f));
        return;
    }
    if(type == MT_GOAL)
    {
        dl->AddRectFilled(ImVec2(p.x + g, p.y + g), ImVec2(q.x - g, q.y - g), c.hole, r);
        dl->AddRect(ImVec2(p.x + g, p.y + g), ImVec2(q.x - g, q.y - g), rgba(ACCENT, 0.9f), r, 0, max(1.0f, s * 0.08f));
        return;
    }
    ImU32 face = type == MT_FRAGILE ? c.fragile : (type == MT_BRIDGE ? c.bridge : c.tile);
    dl->AddRectFilled(ImVec2(p.x + g, p.y + g), ImVec2(q.x - g, q.y - g), face, r);
    if(type == MT_SOFT)
        dl->AddCircleFilled(m, s * 0.24f, c.mark);
    else if(type == MT_HEAVY)
    {
        float k = s * 0.26f, w = max(1.5f, s * 0.11f);
        dl->AddLine(ImVec2(m.x - k, m.y - k), ImVec2(m.x + k, m.y + k), c.mark, w);
        dl->AddLine(ImVec2(m.x - k, m.y + k), ImVec2(m.x + k, m.y - k), c.mark, w);
    }
    else if(type == MT_SPLIT)
    {
        float w = max(1.5f, s * 0.09f);
        dl->PathArcTo(m, s * 0.27f, HUD_PI * 0.65f, HUD_PI * 1.35f, 10);
        dl->PathStroke(c.split, 0, w);
        dl->PathArcTo(m, s * 0.27f, -HUD_PI * 0.35f, HUD_PI * 0.35f, 10);
        dl->PathStroke(c.split, 0, w);
    }
    else if(type == MT_START)
        dl->AddRectFilled(ImVec2(p.x + s * 0.2f, p.y + s * 0.2f), ImVec2(q.x - s * 0.2f, q.y - s * 0.2f), c.block, s * 0.08f);
}

// Đọc nhanh file màn (chỉ để vẽ bản đồ xem trước trong menu, không đụng tới màn đang chơi).
struct LevelPreview
{
    bool ok;
    int n, m, sx, sy, tx, ty;
    vector<int> grid;
    set<int> bridges, splits;
};

static const LevelPreview &level_preview(int lv)
{
    static map<int, LevelPreview> cache;
    map<int, LevelPreview>::iterator it = cache.find(lv);
    if(it != cache.end())
        return it->second;
    LevelPreview p;
    p.ok = false;
    p.n = p.m = p.sx = p.sy = p.tx = p.ty = 0;
    ifstream f("data/" + to_string(lv) + ".data");
    char along;
    if(f >> p.n >> p.m && p.n > 0 && p.m > 0 && p.n < 200 && p.m < 200)
    {
        p.grid.assign(p.n * p.m, 0);
        for(int k=0; k<p.n*p.m; k++) f >> p.grid[k];
        if(f >> p.sx >> p.sy >> along >> p.tx >> p.ty)
        {
            p.ok = true;
            for(int part=0; part<2; part++)
            {
                int cnt = 0;
                if(!(f >> cnt)) break;
                for(int c=0; c<cnt; c++)
                {
                    int x, y, a, b;
                    if(f >> x >> y >> a >> b) p.bridges.insert(a * p.m + b);
                }
            }
            string tok;
            while(f >> tok)
            {
                if(tok == "switch")
                {
                    string kind, mode;
                    int x, y, cnt;
                    if(!(f >> kind >> x >> y >> mode >> cnt)) break;
                    for(int c=0; c<cnt; c++)
                    {
                        int a, b;
                        if(f >> a >> b) p.bridges.insert(a * p.m + b);
                    }
                }
                else if(tok == "split")
                {
                    int x, y, x1, y1, x2, y2;
                    if(f >> x >> y >> x1 >> y1 >> x2 >> y2) p.splits.insert(x * p.m + y);
                }
            }
        }
    }
    return cache[lv] = p;
}

// Bản đồ xem trước của màn lv, căn giữa trong khung (p0, size). Trục x sang phải, y lên trên (như bản đồ nhỏ).
static void draw_level_map(ImDrawList *dl, int lv, ImVec2 p0, ImVec2 size)
{
    const LevelPreview &L = level_preview(lv);
    MiniPalette c = mini_palette();
    dl->AddRectFilled(p0, ImVec2(p0.x + size.x, p0.y + size.y), c.bg, 8.0f);
    if(!L.ok) return;
    int i0 = L.n, i1 = -1, j0 = L.m, j1 = -1;
    for(int i=0; i<L.n; i++)
        for(int j=0; j<L.m; j++)
            if(L.grid[i * L.m + j] || L.bridges.count(i * L.m + j) || (i == L.tx && j == L.ty))
            {
                i0 = min(i0, i); i1 = max(i1, i);
                j0 = min(j0, j); j1 = max(j1, j);
            }
    if(i1 < 0) return;
    int w = i1 - i0 + 1, h = j1 - j0 + 1;
    float s = min((size.x - 20) / w, (size.y - 20) / h);
    s = min(s, 34.0f);
    float ox = p0.x + (size.x - s * w) * 0.5f, oy = p0.y + (size.y - s * h) * 0.5f;
    for(int i=i0; i<=i1; i++)
        for(int j=j0; j<=j1; j++)
        {
            int k = i * L.m + j, v = L.grid[k], type = 0;
            if(i == L.tx && j == L.ty) type = MT_GOAL;
            else if(L.splits.count(k) || v == 5) type = MT_SPLIT;
            else if(L.bridges.count(k)) type = v ? MT_BRIDGE : MT_CLOSED;
            else if(v == 1) type = MT_NORMAL;
            else if(v == 2) type = MT_FRAGILE;
            else if(v == 3) type = MT_SOFT;
            else if(v == 4) type = MT_HEAVY;
            if(!type) continue;
            ImVec2 p(ox + (i - i0) * s, oy + (j1 - j) * s);
            draw_mini_tile(dl, p, s, type, c);
            if(i == L.sx && j == L.sy)
                draw_mini_tile(dl, p, s, MT_START, c);
        }
}

/****************
 *  Options     *
 ****************/

static const char *THEME_NAMES[THEME_COUNT] = {"Classic", "Minecraft", "Original"};

static void graphics_and_audio(float full)
{
    bool changed = false;
    ImGui::TextColored(DIM, "Graphics");
    ImGui::SameLine(110);
    changed |= ImGui::Checkbox("Shadows", &renderer.shadows);
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("Real-time shadow mapping (soft edges).");
    ImGui::SameLine();
    changed |= ImGui::Checkbox("Bloom", &renderer.bloom);
    if(ImGui::IsItemHovered()) ImGui::SetTooltip("HDR glow on bright parts + vignette.");
    ImGui::SameLine();
    changed |= ImGui::Checkbox("Effects", &effects.enabled);
    ImGui::TextColored(DIM, "Interface");
    ImGui::SameLine(110);
    changed |= ImGui::Checkbox("Mini-map", &show_minimap);
    ImGui::SameLine();
    changed |= ImGui::Checkbox("Hints", &show_help);
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
    if(changed) app_save_settings();
}

// Bảng Options gọn trong menu Pause.
static void options_panel(float full)
{
    ImGui::SeparatorText("OPTIONS");
    ImGui::TextColored(DIM, "Theme");
    static const int order[THEME_COUNT] = {THEME_CLASSIC, THEME_ORIGINAL, THEME_MINECRAFT};
    for(int k=0; k<THEME_COUNT; k++)
    {
        ImGui::SameLine(k == 0 ? 110.0f : 0.0f);
        int t = order[k];
        if(ImGui::RadioButton(THEME_NAMES[t], renderer.theme == t)) { app_set_theme(t); app_save_settings(); }
    }
    graphics_and_audio(full);
}

static void pause_menu(ImVec2 disp)
{
    if(!paused || cli_mode || menu_open) return;
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), disp, IM_COL32(0, 0, 0, 120));
    ImGui::SetNextWindowPos(ImVec2(disp.x/2, disp.y/2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460, 0));
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
    if(wide_button("Main menu", full)) app_open_menu();
    if(wide_button(options_open ? "Hide options" : "Options", full)) options_open = !options_open;
    if(options_open) options_panel(full);
    ImGui::Spacing();
    if(wide_button("Quit", full)) app_quit();
    ImGui::End();
}

/*********************************************
 *  Màn hình chính                           *
 *  Cột trái: logo + menu (chuột hoặc phím   *
 *  lên/xuống + Enter). Bên phải: nội dung   *
 *  của trang đang chọn. Phía sau: màn chơi  *
 *  với camera quay chậm.                    *
 *********************************************/

enum MenuPage { PAGE_HOME = 0, PAGE_LEVELS, PAGE_OPTIONS, PAGE_HOWTO, PAGE_CREDITS };
enum MenuAction { ACT_CONTINUE = 0, ACT_PLAY, ACT_LEVELS, ACT_OPTIONS, ACT_HOWTO, ACT_CREDITS, ACT_QUIT };

static int menu_page = PAGE_HOME;
static int menu_sel = 1;
static float item_anim[8];
static bool menu_was_open = false;
static double menu_opened_at = 0.0;
static int preview_level = -1;
static int pending_page = -1;   // trang mở sẵn lần tới (cờ dev --ui)

void hud_open_page(const string &name)
{
    if(name == "levels") pending_page = PAGE_LEVELS;
    else if(name == "settings") pending_page = PAGE_OPTIONS;
    else if(name == "howto") pending_page = PAGE_HOWTO;
    else if(name == "credits") pending_page = PAGE_CREDITS;
}

bool hud_menu_back()
{
    if(!menu_open || menu_page == PAGE_HOME)
        return false;
    menu_page = PAGE_HOME;
    audio_play(SFX_CLICK);
    return true;
}

static ImVec4 group_color(const string &g)
{
    if(g == "G1") return ImVec4(0.35f, 0.85f, 0.45f, 1.0f);
    if(g == "G2") return ImVec4(1.00f, 0.60f, 0.20f, 1.0f);
    if(g == "G3") return ImVec4(0.30f, 0.70f, 1.00f, 1.0f);
    if(g == "G4") return ImVec4(0.75f, 0.50f, 1.00f, 1.0f);
    if(g == "G5") return ImVec4(1.00f, 0.38f, 0.38f, 1.0f);
    return ImVec4(0.70f, 0.70f, 0.75f, 1.0f);
}

static const char *group_name(const string &g)
{
    if(g == "G1") return "Basics";
    if(g == "G2") return "Fragile";
    if(g == "G3") return "Switches";
    if(g == "G4") return "Split";
    if(g == "G5") return "Combined";
    if(g == "G6") return "Special";
    return "Stage";
}

static int next_stage()
{
    vector<int> v = available_levels();
    for(size_t k=0; k<v.size(); k++)
        if(!best_moves.count(v[k]) && v[k] != 11)   // màn 11 cố ý vô nghiệm: không bao giờ "qua"
            return v[k];
    return level;
}

static int cleared_count(int &total)
{
    vector<int> v = available_levels();
    total = 0;
    int c = 0;
    for(size_t k=0; k<v.size(); k++)
    {
        if(v[k] == 11) continue;
        total++;
        if(best_moves.count(v[k])) c++;
    }
    return c;
}

static void draw_logo(ImDrawList *dl, ImVec2 pos, double now, float t_open)
{
    const char *word = "BLOXORZ";
    ImFont *f = font_logo;
    float sz = f->FontSize;
    ImVec4 face, hi, deep;
    if(renderer.theme == THEME_ORIGINAL)
    { face = ImVec4(1.00f, 0.58f, 0.14f, 1); hi = ImVec4(1.00f, 0.86f, 0.52f, 1); deep = ImVec4(0.28f, 0.06f, 0.02f, 1); }
    else if(renderer.theme == THEME_MINECRAFT)
    { face = ImVec4(0.66f, 0.66f, 0.66f, 1); hi = ImVec4(0.92f, 0.92f, 0.92f, 1); deep = ImVec4(0.16f, 0.16f, 0.16f, 1); }
    else
    { face = ImVec4(0.25f, 0.78f, 1.00f, 1); hi = ImVec4(0.78f, 0.96f, 1.00f, 1); deep = ImVec4(0.03f, 0.14f, 0.26f, 1); }
    float x = pos.x;
    for(int i=0; word[i]; i++)
    {
        char ch[2] = {word[i], 0};
        ImVec2 cs = text_size(f, sz, ch);
        float appear = ease_out_cubic((t_open - 0.06f * i) / 0.55f);
        float y = pos.y - (1.0f - appear) * 70.0f + sinf(float(now) * 1.5f + i * 0.7f) * 2.5f;
        float a = appear;
        const int depth = 9;
        for(int k=depth; k>=1; k--)   // đùn khối: chồng các lớp tối lệch xuống phải
        {
            float shade = 0.55f + 0.45f * (1.0f - float(k) / depth);
            dl->AddText(f, sz, ImVec2(x + k * 1.1f, y + k * 1.1f), rgba(ImVec4(deep.x * shade, deep.y * shade, deep.z * shade, 1), a), ch);
        }
        text_outlined(dl, f, sz, ImVec2(x, y), rgba(face, a), IM_COL32(0, 0, 0, (int)(160 * a)), 1.5f, ch);
        dl->PushClipRect(ImVec2(x - 4, y), ImVec2(x + cs.x + 4, y + sz * 0.50f), true);   // nửa trên sáng hơn: mặt vát
        dl->AddText(f, sz, ImVec2(x, y), rgba(hi, a), ch);
        dl->PopClipRect();
        x += cs.x + 1.0f;
    }
}

static bool menu_item(ImDrawList *dl, int idx, const char *label, const char *hint, ImVec2 pos, float w, float dt)
{
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(idx);
    bool clicked = ImGui::InvisibleButton("##item", ImVec2(w, 50));
    ImGui::PopID();
    if(ImGui::IsItemHovered() && (ImGui::GetIO().MouseDelta.x != 0.0f || ImGui::GetIO().MouseDelta.y != 0.0f))
        menu_sel = idx;
    bool sel = menu_sel == idx;
    item_anim[idx] += ((sel ? 1.0f : 0.0f) - item_anim[idx]) * min(1.0f, dt * 14.0f);
    float a = item_anim[idx];
    if(a > 0.01f)
    {
        dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + w, pos.y + 50), rgba(ACCENT, 0.28f * a), rgba(ACCENT, 0.0f),
                                    rgba(ACCENT, 0.0f), rgba(ACCENT, 0.28f * a));
        dl->AddRectFilled(ImVec2(pos.x, pos.y + 8), ImVec2(pos.x + 5, pos.y + 42), rgba(ACCENT, a));
        // khối nhỏ "lăn" vào khi chọn
        float bx = pos.x + 16 + a * 6, by = pos.y + 19;
        dl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + 12, by + 12), rgba(ACCENT, a), 2.0f);
    }
    float tx = pos.x + 22 + a * 18;
    ImU32 col = rgba(mix4(ImVec4(0.80f, 0.82f, 0.86f, 0.85f), ImVec4(1, 1, 1, 1), a));
    text_outlined(dl, font_menu, font_menu->FontSize, ImVec2(tx, pos.y + 9), col, IM_COL32(0, 0, 0, 120), 1.0f, label);
    if(hint && hint[0])
    {
        float lw = text_size(font_menu, font_menu->FontSize, label).x;
        dl->AddText(font_small, font_small->FontSize, ImVec2(tx + lw + 12, pos.y + 19), rgba(ImVec4(1, 1, 1, 0.45f + 0.3f * a)), hint);
    }
    if(clicked) audio_play(SFX_CLICK);
    return clicked;
}

// Khung nội dung bên phải (một cửa sổ ImGui có nền bo góc).
static bool begin_page(const char *id, ImVec2 p, ImVec2 s, const char *title, const char *subtitle)
{
    ImGui::SetNextWindowPos(p);
    ImGui::SetNextWindowSize(s);
    ImGui::SetNextWindowBgAlpha(renderer.theme == THEME_MINECRAFT ? 0.86f : 0.80f);
    ImGui::Begin(id, NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
    ImGui::PushFont(font_big);
    ImGui::TextColored(ACCENT, "%s", title);
    ImGui::PopFont();
    if(subtitle && subtitle[0])
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        ImGui::TextColored(DIM, "%s", subtitle);
    }
    if(menu_page != PAGE_HOME)
    {
        ImGui::SameLine(ImGui::GetWindowWidth() - 110);
        if(wide_button("< Back", 90)) menu_page = PAGE_HOME;
    }
    ImGui::Separator();
    return true;
}

static void stage_details(int lv, float w)
{
    string t = level_title(lv), g = level_group(lv);
    ImGui::PushFont(font_big);
    ImGui::Text("Stage %02d", lv);
    ImGui::PopFont();
    if(!t.empty())
    {
        ImGui::PushFont(font_menu);
        ImGui::TextWrapped("%s", t.c_str());
        ImGui::PopFont();
    }
    ImGui::TextColored(group_color(g), "%s  %s", g.empty() ? "" : g.c_str(), group_name(g));
    if(lv == 11)
        ImGui::TextColored(DIM, "Unsolvable by design - try the AI on it.");
    else if(best_moves.count(lv))
        ImGui::TextColored(GOOD, "Cleared - best %d moves", best_moves[lv]);
    else
        ImGui::TextColored(DIM, "Not cleared yet");
    (void)w;
}

static void page_home(ImVec2 p, ImVec2 s)
{
    int nxt = next_stage();
    begin_page("##page_home", p, s, "NEXT UP", "");
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float full = ImGui::GetContentRegionAvail().x;
    stage_details(nxt, full);
    ImVec2 mp = ImGui::GetCursorScreenPos();
    float mh = max(120.0f, ImGui::GetContentRegionAvail().y - 150.0f);
    draw_level_map(dl, nxt, mp, ImVec2(full, mh));
    ImGui::Dummy(ImVec2(full, mh + 4));
    char label[64];
    snprintf(label, sizeof(label), "PLAY STAGE %02d", nxt);
    ImGui::PushFont(font_menu);
    if(wide_button(label, full)) app_start_level(nxt);
    ImGui::PopFont();
    int total = 0, done = cleared_count(total);
    ImGui::TextColored(DIM, "Progress");
    ImGui::SameLine(100);
    char pr[32];
    snprintf(pr, sizeof(pr), "%d / %d stages cleared", done, total);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ACCENT);
    ImGui::ProgressBar(total ? float(done) / total : 0.0f, ImVec2(-1, 0), pr);
    ImGui::PopStyleColor();
    ImGui::End();
}

static void page_levels(ImVec2 p, ImVec2 s)
{
    int total = 0, done = cleared_count(total);
    char sub[48];
    snprintf(sub, sizeof(sub), "%d / %d cleared", done, total);
    begin_page("##page_levels", p, s, "SELECT STAGE", sub);
    float full = ImGui::GetContentRegionAvail().x;
    float grid_w = full * 0.60f;
    vector<int> levels = available_levels();
    if(preview_level < 0) preview_level = level;

    ImGui::BeginChild("##grid", ImVec2(grid_w, 0), ImGuiChildFlags_None);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float gap = 10.0f;
    int cols = max(2, int((ImGui::GetContentRegionAvail().x + gap) / (150.0f + gap)));
    float cw = (ImGui::GetContentRegionAvail().x - gap * (cols - 1)) / cols, ch = 92.0f;
    for(size_t k=0; k<levels.size(); k++)
    {
        int lv = levels[k];
        if(k % cols) ImGui::SameLine(0, gap);
        ImVec2 c0 = ImGui::GetCursorScreenPos();
        ImGui::PushID(lv);
        bool clicked = ImGui::InvisibleButton("##card", ImVec2(cw, ch));
        ImGui::PopID();
        bool hov = ImGui::IsItemHovered();
        if(hov) preview_level = lv;
        bool cur = lv == level, done_lv = best_moves.count(lv) > 0;
        string g = level_group(lv), t = level_title(lv);
        ImVec2 c1(c0.x + cw, c0.y + ch);
        float rr = renderer.theme == THEME_MINECRAFT ? 0.0f : 8.0f;
        dl->AddRectFilled(c0, c1, rgba(hov ? ImVec4(1, 1, 1, 0.14f) : ImVec4(1, 1, 1, 0.06f)), rr);
        dl->AddRectFilled(c0, ImVec2(c0.x + 5, c1.y), rgba(group_color(g), 0.9f), rr, ImDrawFlags_RoundCornersLeft);
        if(cur || hov)
            dl->AddRect(c0, c1, rgba(ACCENT, hov ? 1.0f : 0.6f), rr, 0, 2.0f);
        char num[8];
        snprintf(num, sizeof(num), "%02d", lv);
        dl->AddText(font_big, font_big->FontSize, ImVec2(c0.x + 14, c0.y + 6), IM_COL32(255, 255, 255, 240), num);
        dl->AddText(font_small, font_small->FontSize, ImVec2(c0.x + 62, c0.y + 10), rgba(group_color(g)), group_name(g));
        if(done_lv)
        {
            char b[24];
            snprintf(b, sizeof(b), "%d mv", best_moves[lv]);
            float bw = text_size(font_small, font_small->FontSize, b).x;
            dl->AddText(font_small, font_small->FontSize, ImVec2(c1.x - bw - 10, c0.y + 10), rgba(GOOD), b);
        }
        // tên màn, cắt bớt cho vừa thẻ
        string shown = t;
        while(!shown.empty() && text_size(font_ui, font_ui->FontSize, shown.c_str()).x > cw - 24)
            shown = shown.substr(0, shown.size() - 1);
        if(shown.size() < t.size() && shown.size() > 2) shown = shown.substr(0, shown.size() - 2) + "..";
        dl->AddText(font_ui, font_ui->FontSize, ImVec2(c0.x + 14, c0.y + 46), rgba(ImVec4(1, 1, 1, 0.82f)), shown.c_str());
        dl->AddText(font_small, font_small->FontSize, ImVec2(c0.x + 14, c0.y + 68),
                    rgba(done_lv ? GOOD : DIM, 0.9f), lv == 11 ? "no solution" : (done_lv ? "cleared" : "new"));
        if(clicked)
        {
            audio_play(SFX_CLICK);
            app_start_level(lv);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine(0, 16);
    ImGui::BeginChild("##preview", ImVec2(0, 0), ImGuiChildFlags_None);
    float pw = ImGui::GetContentRegionAvail().x;
    stage_details(preview_level, pw);
    ImVec2 mp = ImGui::GetCursorScreenPos();
    float mh = max(100.0f, ImGui::GetContentRegionAvail().y - 60.0f);
    draw_level_map(ImGui::GetWindowDrawList(), preview_level, mp, ImVec2(pw, mh));
    ImGui::Dummy(ImVec2(pw, mh + 4));
    char label[48];
    snprintf(label, sizeof(label), "Play stage %02d", preview_level);
    ImGui::PushFont(font_menu);
    if(wide_button(label, pw)) app_start_level(preview_level);
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::End();
}

static void theme_card(ImDrawList *dl, int t, ImVec2 p, ImVec2 s)
{
    ImGui::SetCursorScreenPos(p);
    ImGui::PushID(t);
    bool clicked = ImGui::InvisibleButton("##theme", s);
    ImGui::PopID();
    bool hov = ImGui::IsItemHovered(), sel = renderer.theme == t;
    ImU32 bg, tile, side, block;
    const char *desc;
    if(t == THEME_ORIGINAL)
    { bg = IM_COL32(170, 40, 12, 255); tile = IM_COL32(200, 204, 214, 255); side = IM_COL32(90, 92, 102, 255); block = IM_COL32(120, 60, 28, 255); desc = "Red sky, stone tiles, rusty block"; }
    else if(t == THEME_MINECRAFT)
    { bg = IM_COL32(120, 165, 245, 255); tile = IM_COL32(95, 160, 64, 255); side = IM_COL32(134, 96, 67, 255); block = IM_COL32(200, 40, 30, 255); desc = "Pixel blocks: grass, TNT, planks"; }
    else
    { bg = IM_COL32(6, 10, 24, 255); tile = IM_COL32(128, 140, 163, 255); side = IM_COL32(60, 66, 82, 255); block = IM_COL32(200, 31, 26, 255); desc = "Dark space, steel tiles, red block"; }
    float r = renderer.theme == THEME_MINECRAFT ? 0.0f : 8.0f;
    ImVec2 q(p.x + s.x, p.y + s.y);
    dl->AddRectFilled(p, q, bg, r);
    // ba ô + khối nhỏ minh họa
    float ts = s.y * 0.20f, by = p.y + s.y * 0.40f;
    for(int k=0; k<3; k++)
    {
        float x = p.x + s.x * 0.5f + (k - 1.5f) * (ts + 3);
        dl->AddRectFilled(ImVec2(x, by + ts), ImVec2(x + ts, by + ts * 1.35f), side);
        dl->AddRectFilled(ImVec2(x, by), ImVec2(x + ts, by + ts), tile);
    }
    float bx = p.x + s.x * 0.5f - ts * 0.5f;
    dl->AddRectFilled(ImVec2(bx, by - ts * 1.6f), ImVec2(bx + ts, by + ts * 0.4f), block, 2.0f);
    dl->AddRectFilledMultiColor(ImVec2(p.x, q.y - 46), q, IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 200), IM_COL32(0, 0, 0, 200));
    dl->AddText(font_menu, font_menu->FontSize * 0.8f, ImVec2(p.x + 10, q.y - 46), IM_COL32(255, 255, 255, 255), THEME_NAMES[t]);
    dl->AddText(font_small, font_small->FontSize, ImVec2(p.x + 10, q.y - 22), IM_COL32(255, 255, 255, 190), desc);
    if(sel || hov)
        dl->AddRect(p, q, rgba(ACCENT, sel ? 1.0f : 0.6f), r, 0, sel ? 3.0f : 2.0f);
    if(clicked && !sel)
    {
        audio_play(SFX_CLICK);
        app_set_theme(t);
        app_save_settings();
    }
}

static void page_options(ImVec2 p, ImVec2 s)
{
    begin_page("##page_options", p, s, "OPTIONS", "");
    float full = ImGui::GetContentRegionAvail().x;
    ImGui::SeparatorText("THEME");
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float gap = 12.0f, cw = (full - 2 * gap) / 3, chh = min(170.0f, cw * 0.62f);
    ImVec2 c0 = ImGui::GetCursorScreenPos();
    static const int order[THEME_COUNT] = {THEME_CLASSIC, THEME_ORIGINAL, THEME_MINECRAFT};
    for(int k=0; k<THEME_COUNT; k++)
        theme_card(dl, order[k], ImVec2(c0.x + k * (cw + gap), c0.y), ImVec2(cw, chh));
    ImGui::SetCursorScreenPos(ImVec2(c0.x, c0.y + chh + 10));
    ImGui::SeparatorText("GRAPHICS & AUDIO");
    graphics_and_audio(full);
    ImGui::SeparatorText("CAMERA");
    ImGui::TextColored(DIM, "In a level: C follow, B overview, N free orbit, M mouse orbit, X fly, V top.");
    ImGui::TextColored(DIM, "Drag with the mouse to rotate, wheel to zoom (N: right-drag or WASD to pan).");
    ImGui::End();
}

static void legend_row(ImDrawList *dl, int type, const char *name, const char *desc)
{
    MiniPalette c = mini_palette();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float s = 34.0f;
    dl->AddRectFilled(ImVec2(p.x - 2, p.y - 2), ImVec2(p.x + s + 2, p.y + s + 2), c.bg, 6.0f);
    draw_mini_tile(dl, p, s, type == MT_START ? MT_NORMAL : type, c);
    if(type == MT_START) draw_mini_tile(dl, p, s, MT_START, c);
    ImGui::Dummy(ImVec2(s, s));
    ImGui::SameLine(0, 14);
    ImGui::BeginGroup();
    ImGui::Text("%s", name);
    ImGui::TextColored(DIM, "%s", desc);
    ImGui::EndGroup();
}

static void page_howto(ImVec2 p, ImVec2 s)
{
    begin_page("##page_howto", p, s, "HOW TO PLAY", "");
    float full = ImGui::GetContentRegionAvail().x;
    ImGui::TextWrapped("Roll the 1x1x2 block across the floating tiles and drop it standing upright into the square hole. "
                       "Falling off the edge collapses the floor and restarts the stage.");
    ImGui::Spacing();
    ImGui::BeginChild("##keys", ImVec2(full * 0.42f, 0), ImGuiChildFlags_None);
    ImGui::SeparatorText("CONTROLS");
    const char *rows[][2] = {
        {"Arrow keys", "roll the block"},
        {"Space", "switch cube after a split"},
        {"R", "restart the stage"},
        {"P / Esc", "pause"},
        {"Enter", "next stage after a win"},
        {"Mouse drag", "rotate the camera"},
        {"Mouse wheel", "zoom in / out"},
        {"C / B", "follow / overview camera"},
        {"N / M", "free orbit / mouse orbit"},
        {"X / V", "fly / top-down view"},
        {"Tab", "mini-map"},
        {"F12", "screenshot"},
    };
    for(size_t k=0; k<sizeof(rows)/sizeof(rows[0]); k++)
    {
        ImGui::TextColored(ACCENT, "%s", rows[k][0]);
        ImGui::SameLine(130);
        ImGui::TextColored(DIM, "%s", rows[k][1]);
    }
    ImGui::SeparatorText("AI");
    ImGui::TextWrapped("Inside a stage, the AI panel solves it with BFS, DFS, UCS or A* and replays the moves. "
                       "Visualize search shows the search tree step by step.");
    ImGui::EndChild();
    ImGui::SameLine(0, 20);
    ImGui::BeginChild("##tiles", ImVec2(0, 0), ImGuiChildFlags_None);
    ImGui::SeparatorText("TILES");
    ImDrawList *dl = ImGui::GetWindowDrawList();
    legend_row(dl, MT_NORMAL, "Floor", "Safe in any position.");
    legend_row(dl, MT_FRAGILE, "Fragile tile", "Breaks if the block stands upright on it.");
    legend_row(dl, MT_SOFT, "Soft switch (O)", "Any contact toggles its bridges.");
    legend_row(dl, MT_HEAVY, "Heavy switch (X)", "Only the upright block presses it.");
    legend_row(dl, MT_SPLIT, "Split switch ( )", "Splits the block into two cubes.");
    legend_row(dl, MT_CLOSED, "Bridge", "Appears / disappears when a switch is pressed.");
    legend_row(dl, MT_GOAL, "Goal hole", "Stand upright here to clear the stage.");
    ImGui::EndChild();
    ImGui::End();
}

static void page_credits(ImVec2 p, ImVec2 s)
{
    begin_page("##page_credits", p, s, "CREDITS", "");
    ImGui::PushFont(font_menu);
    ImGui::Text("CSC14003 - Introduction to AI");
    ImGui::PopFont();
    ImGui::TextColored(DIM, "Project 1 - Group 9: search algorithms on Bloxorz");
    ImGui::Spacing();
    ImGui::SeparatorText("GAME");
    ImGui::TextWrapped("3D engine based on chaitanya100100/Bloxorz (C++/OpenGL), rebuilt with instanced rendering, "
                       "real-time shadows, HDR bloom, three visual themes, sound and a new interface.");
    ImGui::TextWrapped("Gameplay concept: Bloxorz by Damien Clarke (2007). The Original theme is a tribute drawn "
                       "procedurally in shaders - no assets from the original game are used.");
    ImGui::SeparatorText("AI");
    ImGui::TextWrapped("Python solvers (BFS, DFS, UCS, A* with a split-aware Manhattan heuristic), search traces "
                       "for the visualizer.");
    ImGui::SeparatorText("LIBRARIES");
    ImGui::TextWrapped("GLFW, GLAD, GLM, Dear ImGui, miniaudio - see THIRD_PARTY_NOTICES.md.");
    ImGui::End();
}

static void main_menu(ImVec2 disp, double now)
{
    if(!menu_open || cli_mode)
    {
        menu_was_open = false;
        return;
    }
    if(!menu_was_open)
    {
        menu_was_open = true;
        menu_opened_at = now;
        menu_page = pending_page >= 0 ? pending_page : PAGE_HOME;
        pending_page = -1;
        menu_sel = (game_result == -1 && game.get_steps() > 0) ? ACT_CONTINUE : ACT_PLAY;
        if(menu_page != PAGE_HOME) menu_sel = ACT_LEVELS + (menu_page - PAGE_LEVELS);
        preview_level = -1;
        for(int k=0; k<8; k++) item_anim[k] = 0.0f;
    }
    float dt = ImGui::GetIO().DeltaTime;
    float t_open = float(now - menu_opened_at);

    // Nền: tối dần sang trái (chỗ đặt menu) và dưới, cảnh 3D phía sau vẫn thấy rõ ở giữa.
    ImDrawList *bg = ImGui::GetBackgroundDrawList();
    float lw = min(disp.x * 0.55f, 720.0f);
    bg->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(lw, disp.y), IM_COL32(0, 0, 0, 200), IM_COL32(0, 0, 0, 0),
                                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 200));
    bg->AddRectFilledMultiColor(ImVec2(0, disp.y - 120), disp, IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
                                IM_COL32(0, 0, 0, 170), IM_COL32(0, 0, 0, 170));

    // Cửa sổ trong suốt phủ màn hình chứa logo + menu.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(disp);
    ImGui::Begin("##menuroot", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings
                 | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float lx = max(36.0f, disp.x * 0.05f);
    float logo_y = max(30.0f, disp.y * 0.08f);
    draw_logo(dl, ImVec2(lx, logo_y), now, t_open);
    const char *tag = "The block-rolling puzzle  -  AI edition";
    dl->AddText(font_menu, font_menu->FontSize * 0.75f, ImVec2(lx + 6, logo_y + font_logo->FontSize + 8),
                rgba(ImVec4(1, 1, 1, 0.75f * clamp01(t_open * 2.0f))), tag);

    bool can_continue = game_result == -1 && game.get_steps() > 0;
    char play_hint[32] = "";
    int nxt = next_stage();
    snprintf(play_hint, sizeof(play_hint), "stage %02d", nxt);
    char cont_hint[32] = "";
    snprintf(cont_hint, sizeof(cont_hint), "stage %02d, %d moves", level, game.get_steps());
    struct Item { int act; const char *label; const char *hint; };
    Item items[] = {
        {ACT_CONTINUE, "CONTINUE", cont_hint},
        {ACT_PLAY, best_moves.empty() ? "NEW GAME" : "PLAY", play_hint},
        {ACT_LEVELS, "SELECT STAGE", ""},
        {ACT_OPTIONS, "OPTIONS", ""},
        {ACT_HOWTO, "HOW TO PLAY", ""},
        {ACT_CREDITS, "CREDITS", ""},
        {ACT_QUIT, "QUIT", ""},
    };
    const int N = sizeof(items) / sizeof(items[0]);
    if(!can_continue && menu_sel == ACT_CONTINUE) menu_sel = ACT_PLAY;

    // Phím: lên/xuống chọn, Enter kích hoạt (chỉ khi không gõ vào ô nhập nào).
    int activate = -1;
    if(!ImGui::GetIO().WantTextInput)
    {
        int dir = ImGui::IsKeyPressed(ImGuiKey_DownArrow) ? 1 : (ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? -1 : 0);
        if(dir)
        {
            do { menu_sel = (menu_sel + dir + N) % N; } while(menu_sel == ACT_CONTINUE && !can_continue);
            audio_play(SFX_CLICK, 0.5f);
        }
        if(ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
            activate = menu_sel;
    }

    float y = max(logo_y + font_logo->FontSize + 60.0f, disp.y * 0.36f);
    float iw = min(lw - lx - 20, 380.0f);
    for(int k=0; k<N; k++)
    {
        if(items[k].act == ACT_CONTINUE && !can_continue) continue;
        float appear = ease_out_cubic((t_open - 0.25f - 0.05f * k) / 0.4f);
        ImVec2 pos(lx - (1.0f - appear) * 60.0f, y);
        if(menu_item(dl, items[k].act, items[k].label, items[k].hint, pos, iw, dt))
            activate = items[k].act;
        y += 54;
    }
    // Chân trang
    const char *keys = "Up / Down  select     Enter  confirm     Esc  back";
    dl->AddText(font_small, font_small->FontSize, ImVec2(lx, disp.y - 34), IM_COL32(255, 255, 255, 140), keys);
    const char *sig = "CSC14003  -  Group 9";
    float sw = text_size(font_small, font_small->FontSize, sig).x;
    dl->AddText(font_small, font_small->FontSize, ImVec2(disp.x - sw - 24, disp.y - 34), IM_COL32(255, 255, 255, 140), sig);
    ImGui::End();

    switch(activate)
    {
        case ACT_CONTINUE: audio_play(SFX_CLICK); app_toggle_menu(); return;
        case ACT_PLAY:     audio_play(SFX_CLICK); app_start_level(nxt); return;
        case ACT_LEVELS:   menu_page = PAGE_LEVELS; break;
        case ACT_OPTIONS:  menu_page = PAGE_OPTIONS; break;
        case ACT_HOWTO:    menu_page = PAGE_HOWTO; break;
        case ACT_CREDITS:  menu_page = PAGE_CREDITS; break;
        case ACT_QUIT:     app_quit(); return;
        default: break;
    }

    // Trang nội dung bên phải (trượt vào khi mở menu).
    float px = max(lx + iw + 40.0f, disp.x * 0.40f);
    float slide = (1.0f - ease_out_cubic((t_open - 0.2f) / 0.5f)) * 80.0f;
    ImVec2 pp(px + slide, max(28.0f, disp.y * 0.07f));
    ImVec2 ps(disp.x - px - max(28.0f, disp.x * 0.035f), disp.y - pp.y - 64.0f);
    if(ps.x < 300) return;   // cửa sổ quá hẹp: chỉ hiện menu
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, clamp01((t_open - 0.15f) * 3.0f));
    if(menu_page == PAGE_LEVELS) page_levels(pp, ps);
    else if(menu_page == PAGE_OPTIONS) page_options(pp, ps);
    else if(menu_page == PAGE_HOWTO) page_howto(pp, ps);
    else if(menu_page == PAGE_CREDITS) page_credits(pp, ps);
    else page_home(pp, ps);
    ImGui::PopStyleVar();
}

void hud_frame(double now)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(font_ui);

    ImVec2 disp = ImGui::GetIO().DisplaySize;
    stats_panel(now);
    if(!menu_open)
    {
        toolbar(disp);
        ai_panel(disp);
        hint_bar(disp);
        fall_toast(disp);
        end_panel(disp, now);
        stage_banner(disp, now);
    }
    solver_toast(disp, now);
    if(viz_is_open() && !menu_open)
        viz_draw_window(disp.x * 0.52f + 8, 12, disp.x * 0.48f - 20, disp.y - 24);
    pause_menu(disp);
    main_menu(disp, now);

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
