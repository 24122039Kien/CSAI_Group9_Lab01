#include "headers.h"
#include "viz.h"
#include "render.h"
#include "app.h"
#include "audio.h"

#include "imgui.h"

#include <sstream>
#include <climits>

namespace {

struct VNode
{
    int parent, depth;
    std::string action;
    double g, h;
    int x, y;
    char o;                  // 'X' 'Y' 'Z' hoặc 'S' (đã tách thành 2 khối nhỏ)
    long long mask;
    int c[4];                // vị trí 2 khối nhỏ khi o == 'S'
    int disc;                // bước mà node được sinh ra (0 = gốc)
    int expd;                // bước mà node được mở rộng (INT_MAX = chưa)
    float tx, ty;            // vị trí trong cây (đơn vị: ô lưới bố cục)
    vector<int> kids;
};

struct VStep
{
    int node, frontier;
    vector<pair<int, char> > items;   // (id con, 'n' mới / 'u' đổi cha / 'o' đã có)
};

vector<VNode> nodes;
vector<VStep> steps;
vector<int> path;
vector<char> on_path;
bool is_open = false, playing = false, found = false, truncated = false;
std::string algo, heuristic;
int cur = 0;                 // số bước mở rộng đã áp dụng (0..steps.size())
double speed = 8.0, acc = 0.0, last_time = -1.0;
int view_mode = 0;           // 0 cây, 1 đồ thị theo vị trí trên bàn chơi
bool show_board = true, follow = true;
ImVec2 pan(0, 0);
float zoom = 1.0f;
bool need_fit = true;

const ImU32 C_EDGE = IM_COL32(110, 125, 150, 140);
const ImU32 C_FRONTIER = IM_COL32(255, 190, 60, 255);
const ImU32 C_EXPANDED = IM_COL32(70, 150, 255, 255);
const ImU32 C_GOAL = IM_COL32(80, 255, 140, 255);
const ImU32 C_PATH = IM_COL32(80, 255, 140, 255);

const char *algo_name(const std::string &a)
{
    if(a == "bfs") return "BFS";
    if(a == "dfs") return "DFS";
    if(a == "ucs") return "UCS";
    if(a == "astar") return "A*";
    return a.c_str();
}

void layout_tree()
{
    for(size_t i = 0; i < nodes.size(); i++) nodes[i].kids.clear();
    for(size_t i = 0; i < nodes.size(); i++)
        if(nodes[i].parent >= 0 && nodes[i].parent < (int)nodes.size())
            nodes[nodes[i].parent].kids.push_back((int)i);
    // Bố cục "lá liên tiếp": lá nhận x = 0, 1, 2...; node trong nằm giữa con đầu và con cuối.
    float leaf = 0.0f;
    vector<pair<int, size_t> > st;
    if(!nodes.empty()) st.push_back(make_pair(0, (size_t)0));
    while(!st.empty())
    {
        int v = st.back().first;
        size_t &k = st.back().second;
        if(k < nodes[v].kids.size())
        {
            int child = nodes[v].kids[k++];
            st.push_back(make_pair(child, (size_t)0));
            continue;
        }
        VNode &n = nodes[v];
        if(n.kids.empty()) n.tx = leaf++;
        else n.tx = 0.5f * (nodes[n.kids.front()].tx + nodes[n.kids.back()].tx);
        n.ty = float(n.depth);
        st.pop_back();
    }
}

// Vị trí node (đơn vị pixel trước khi zoom) theo chế độ xem.
ImVec2 node_pos(const VNode &n)
{
    if(view_mode == 0)
        return ImVec2(n.tx * 18.0f, n.ty * 54.0f);
    // Đồ thị: như bản đồ chữ (cột y sang phải, dòng x xuống dưới); tư thế/cầu lệch nhẹ để không chồng nhau.
    float cell = 46.0f;
    float px = (n.y + 0.5f) * cell, py = (n.x + 0.5f) * cell;
    if(n.o == 'X') py += 0.5f * cell;
    else if(n.o == 'Y') px += 0.5f * cell;
    else if(n.o == 'S') { px = ((n.c[1] + n.c[3]) * 0.5f + 0.5f) * cell; py = ((n.c[0] + n.c[2]) * 0.5f + 0.5f) * cell; }
    float a = float((n.mask * 2654435761LL) % 628) / 100.0f;
    float r = n.mask ? 0.18f * cell : 0.0f;
    return ImVec2(px + r * cos(a), py + r * sin(a));
}

int current_node() { return cur > 0 ? steps[cur - 1].node : (nodes.empty() ? -1 : 0); }

void fit_view(ImVec2 canvas)
{
    if(nodes.empty()) return;
    ImVec2 lo(1e9f, 1e9f), hi(-1e9f, -1e9f);
    for(size_t i = 0; i < nodes.size(); i++)
    {
        ImVec2 p = node_pos(nodes[i]);
        lo.x = min(lo.x, p.x); lo.y = min(lo.y, p.y);
        hi.x = max(hi.x, p.x); hi.y = max(hi.y, p.y);
    }
    float w = max(1.0f, hi.x - lo.x), h = max(1.0f, hi.y - lo.y);
    zoom = min((canvas.x - 40) / w, (canvas.y - 40) / h);
    zoom = max(0.05f, min(zoom, 2.5f));
    pan = ImVec2(20 - lo.x * zoom + max(0.0f, (canvas.x - 40 - w * zoom) * 0.5f), 20 - lo.y * zoom);
}

void center_on(int id, ImVec2 canvas)
{
    if(id < 0) return;
    ImVec2 p = node_pos(nodes[id]);
    ImVec2 target(canvas.x * 0.5f - p.x * zoom, canvas.y * 0.35f - p.y * zoom);
    pan.x += (target.x - pan.x) * 0.15f;
    pan.y += (target.y - pan.y) * 0.15f;
}

std::string describe(const VNode &n)
{
    char buf[160];
    if(n.o == 'S')
        snprintf(buf, sizeof(buf), "cubes (%d,%d) + (%d,%d)  mask=%lld", n.c[0], n.c[1], n.c[2], n.c[3], n.mask);
    else
        snprintf(buf, sizeof(buf), "(%d,%d) %c  mask=%lld", n.x, n.y, n.o, n.mask);
    return buf;
}

void cells_of(const VNode &n, vector<pair<int, int> > &out)
{
    out.clear();
    if(n.o == 'S') { out.push_back(make_pair(n.c[0], n.c[1])); out.push_back(make_pair(n.c[2], n.c[3])); }
    else
    {
        out.push_back(make_pair(n.x, n.y));
        if(n.o == 'X') out.push_back(make_pair(n.x + 1, n.y));
        if(n.o == 'Y') out.push_back(make_pair(n.x, n.y + 1));
    }
}

void ghost_block(const VNode &n, glm::vec3 color, float alpha, float glow)
{
    BoxStyle s(color, PAT_PLAIN, glow, alpha);
    const float m = 0.1f;
    if(n.o == 'S')
    {
        for(int k = 0; k < 2; k++)
            renderer.draw_aabb(glm::vec3(n.c[2*k] + m, n.c[2*k+1] + m, 0.0f), glm::vec3(1 - 2*m, 1 - 2*m, 1 - 2*m), s);
        return;
    }
    glm::vec3 size(1 - 2*m, 1 - 2*m, 2 - 2*m);
    if(n.o == 'X') size = glm::vec3(2 - 2*m, 1 - 2*m, 1 - 2*m);
    if(n.o == 'Y') size = glm::vec3(1 - 2*m, 2 - 2*m, 1 - 2*m);
    renderer.draw_aabb(glm::vec3(n.x + m, n.y + m, 0.0f), size, s);
}

bool button(const char *label, bool enabled = true)
{
    if(!enabled) ImGui::BeginDisabled();
    bool r = ImGui::Button(label);
    if(!enabled) ImGui::EndDisabled();
    if(r) audio_play(SFX_CLICK);
    return r;
}

} // namespace

bool viz_load(const std::string &file, std::string &error)
{
    std::ifstream fin(file.c_str());
    if(!fin) { error = "cannot open " + file; return false; }
    std::string head; int version = 0;
    fin >> head >> version;
    if(head != "BLOXORZ-TRACE" || version != 1) { error = "not a trace file"; return false; }
    std::string key; int f = 0, t = 0;
    fin >> key >> algo >> key >> heuristic >> key >> f >> key >> t;
    found = f != 0; truncated = t != 0;
    nodes.clear(); steps.clear(); path.clear();
    std::string line;
    std::getline(fin, line);
    while(std::getline(fin, line))
    {
        std::istringstream ss(line);
        std::string tag; ss >> tag;
        if(tag == "N")
        {
            VNode n; int id;
            ss >> id >> n.parent >> n.action >> n.depth >> n.g >> n.h >> n.x >> n.y >> n.o >> n.mask
               >> n.c[0] >> n.c[1] >> n.c[2] >> n.c[3];
            n.disc = id == 0 ? 0 : INT_MAX;
            n.expd = INT_MAX;
            n.tx = n.ty = 0.0f;
            nodes.push_back(n);
        }
        else if(tag == "S")
        {
            VStep s; int k;
            ss >> s.node >> s.frontier >> k;
            for(int i = 0; i < k; i++) { int id; char kind; ss >> id >> kind; s.items.push_back(make_pair(id, kind)); }
            steps.push_back(s);
        }
        else if(tag == "P")
        {
            int id; while(ss >> id) path.push_back(id);
        }
    }
    if(nodes.empty()) { error = "empty trace"; return false; }
    for(size_t k = 0; k < steps.size(); k++)
    {
        VStep &s = steps[k];
        if(s.node >= 0 && s.node < (int)nodes.size()) nodes[s.node].expd = min(nodes[s.node].expd, (int)k + 1);
        for(size_t i = 0; i < s.items.size(); i++)
        {
            int id = s.items[i].first;
            if(s.items[i].second == 'n' && id >= 0 && id < (int)nodes.size())
                nodes[id].disc = (int)k + 1;
        }
    }
    on_path.assign(nodes.size(), 0);
    for(size_t i = 0; i < path.size(); i++)
        if(path[i] >= 0 && path[i] < (int)nodes.size()) on_path[path[i]] = 1;
    layout_tree();
    cur = 0; playing = false; acc = 0.0; last_time = -1.0; need_fit = true;
    is_open = true;
    return true;
}

bool viz_is_open() { return is_open; }
void viz_close() { is_open = false; playing = false; }
void viz_toggle_play() { if(cur >= (int)steps.size()) cur = 0; playing = !playing; }

void viz_step(int delta)
{
    cur = max(0, min((int)steps.size(), cur + delta));
    if(cur >= (int)steps.size()) playing = false;
}

void viz_update(double now)
{
    if(!is_open) return;
    double dt = last_time < 0 ? 0.0 : now - last_time;
    last_time = now;
    if(!playing) { acc = 0.0; return; }
    acc += dt * speed;
    int n = int(acc);
    if(n > 0) { acc -= n; viz_step(n); }
}

void viz_draw_window(float x, float y, float w, float h)
{
    if(!is_open) return;
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    char title[96];
    snprintf(title, sizeof(title), "SEARCH VISUALIZER - %s%s%s###viz", algo_name(algo),
             algo == "astar" ? " h=" : "", algo == "astar" ? heuristic.c_str() : "");
    ImGui::Begin(title, NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
                 | ImGuiWindowFlags_NoSavedSettings);

    // Chọn thuật toán và chạy lại
    static const char *algos[4] = {"bfs", "dfs", "ucs", "astar"};
    for(int k = 0; k < 4; k++)
    {
        if(k) ImGui::SameLine();
        bool sel = algo == algos[k];
        if(sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.80f, 1.0f));
        if(button(algo_name(algos[k]), solve_state != SOLVE_RUNNING)) app_visualize(algos[k]);
        if(sel) ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    if(button("Play solution", found && solve_state != SOLVE_RUNNING)) app_play_solution();
    ImGui::SameLine();
    if(button("Close")) app_close_visualizer();

    // Điều khiển từng bước
    int total = (int)steps.size();
    if(button("|<")) { cur = 0; playing = false; }
    ImGui::SameLine();
    if(button("< Step")) { playing = false; viz_step(-1); }
    ImGui::SameLine();
    if(button(playing ? "Pause" : "Play")) viz_toggle_play();
    ImGui::SameLine();
    if(button("Step >")) { playing = false; viz_step(1); }
    ImGui::SameLine();
    if(button(">|")) { cur = total; playing = false; }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    float sp = (float)speed;
    if(ImGui::SliderFloat("steps/s", &sp, 1.0f, 300.0f, "%.0f", ImGuiSliderFlags_Logarithmic)) speed = sp;
    ImGui::SetNextItemWidth(-1);
    int c = cur;
    if(ImGui::SliderInt("##timeline", &c, 0, total, "step %d")) { cur = c; playing = false; }

    // Số liệu tại bước hiện tại
    int discovered = 0, expanded = 0;
    for(size_t i = 0; i < nodes.size(); i++)
    {
        if(nodes[i].disc <= cur) discovered++;
        if(nodes[i].expd <= cur) expanded++;
    }
    int frontier = cur > 0 ? steps[cur - 1].frontier : 1;
    ImGui::Text("Expanded %d / %d   Generated nodes %d   Frontier %d", expanded, total, discovered, frontier);
    int cn = current_node();
    if(cn >= 0)
    {
        const VNode &n = nodes[cn];
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "Current: %s  depth %d  g=%g  h=%g  f=%g",
                           describe(n).c_str(), n.depth, n.g, n.h, n.g + n.h);
    }
    if(cur >= total)
    {
        if(found) ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.55f, 1.0f), "Goal reached: solution has %d moves (green path).", (int)path.size() - 1);
        else if(!truncated) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.35f, 1.0f), "Frontier empty: no solution.");
    }
    if(truncated)
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "Large search: only the first %d expansions were recorded.", total);

    // Chế độ xem
    if(ImGui::RadioButton("Search tree", view_mode == 0)) { view_mode = 0; need_fit = true; }
    ImGui::SameLine();
    if(ImGui::RadioButton("State graph", view_mode == 1)) { view_mode = 1; need_fit = true; }
    ImGui::SameLine();
    ImGui::Checkbox("Follow", &follow);
    ImGui::SameLine();
    ImGui::Checkbox("Board overlay", &show_board);
    ImGui::SameLine();
    if(button("Fit")) { need_fit = true; follow = false; }

    // Chú thích
    ImDrawList *dl = ImGui::GetWindowDrawList();
    struct { ImU32 col; const char *txt; } legend[4] = {
        {C_FRONTIER, "frontier"}, {C_EXPANDED, "expanded"}, {IM_COL32(255, 255, 255, 255), "current"}, {C_PATH, "solution"}};
    for(int k = 0; k < 4; k++)
    {
        if(k) ImGui::SameLine();
        ImVec2 p = ImGui::GetCursorScreenPos();
        dl->AddCircleFilled(ImVec2(p.x + 7, p.y + 9), 5, legend[k].col);
        ImGui::Dummy(ImVec2(16, 18));
        ImGui::SameLine();
        ImGui::TextDisabled("%s", legend[k].txt);
    }

    // Khung vẽ: kéo chuột để di chuyển, cuộn để phóng to
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 csz = ImGui::GetContentRegionAvail();
    csz.x = max(csz.x, 50.0f); csz.y = max(csz.y, 50.0f);
    ImGui::InvisibleButton("canvas", csz, ImGuiButtonFlags_MouseButtonLeft);
    ImGuiIO &io = ImGui::GetIO();
    bool hovered = ImGui::IsItemHovered();
    if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        pan.x += io.MouseDelta.x; pan.y += io.MouseDelta.y;
        follow = false;
    }
    if(hovered && io.MouseWheel != 0.0f)
    {
        float old = zoom;
        zoom = max(0.03f, min(6.0f, zoom * (io.MouseWheel > 0 ? 1.15f : 1.0f / 1.15f)));
        ImVec2 m(io.MousePos.x - p0.x, io.MousePos.y - p0.y);
        pan.x = m.x - (m.x - pan.x) * zoom / old;
        pan.y = m.y - (m.y - pan.y) * zoom / old;
    }
    if(need_fit) { fit_view(csz); need_fit = false; }
    else if(follow) center_on(cn, csz);

    dl->AddRectFilled(p0, ImVec2(p0.x + csz.x, p0.y + csz.y), IM_COL32(8, 10, 18, 230), 6.0f);
    dl->PushClipRect(p0, ImVec2(p0.x + csz.x, p0.y + csz.y), true);
    bool finished = cur >= total && found;
    float rad = max(1.5f, 5.0f * min(zoom, 1.6f));
    int hover_id = -1;
    float hover_d = 1e9f;
    for(int pass = 0; pass < 2; pass++)
        for(size_t i = 0; i < nodes.size(); i++)
        {
            const VNode &n = nodes[i];
            if(n.disc > cur) continue;
            ImVec2 q = node_pos(n);
            ImVec2 s(p0.x + pan.x + q.x * zoom, p0.y + pan.y + q.y * zoom);
            if(pass == 0)
            {
                if(n.parent < 0 || nodes[n.parent].disc > cur) continue;
                ImVec2 pq = node_pos(nodes[n.parent]);
                ImVec2 ps(p0.x + pan.x + pq.x * zoom, p0.y + pan.y + pq.y * zoom);
                bool hot = finished && on_path[i] && on_path[n.parent];
                dl->AddLine(ps, s, hot ? C_PATH : C_EDGE, hot ? 3.0f : 1.0f);
                if(view_mode == 0 && zoom > 0.9f && !n.action.empty() && n.action != "-")
                    dl->AddText(ImVec2((ps.x + s.x) * 0.5f + 3, (ps.y + s.y) * 0.5f - 7), IM_COL32(150, 165, 190, 200), n.action.c_str());
                continue;
            }
            if(s.x < p0.x - 10 || s.y < p0.y - 10 || s.x > p0.x + csz.x + 10 || s.y > p0.y + csz.y + 10) continue;
            ImU32 col = n.expd <= cur ? C_EXPANDED : C_FRONTIER;
            if(finished && on_path[i]) col = C_PATH;
            if(found && !path.empty() && (int)i == path.back() && n.disc <= cur) col = C_GOAL;
            dl->AddCircleFilled(s, rad, col);
            if((int)i == cn) dl->AddCircle(s, rad + 4.0f, IM_COL32(255, 255, 255, 255), 0, 2.5f);
            if(hovered)
            {
                float dx = io.MousePos.x - s.x, dy = io.MousePos.y - s.y, d = dx*dx + dy*dy;
                if(d < hover_d && d < (rad + 5) * (rad + 5)) { hover_d = d; hover_id = (int)i; }
            }
        }
    dl->PopClipRect();
    if(hover_id >= 0)
    {
        const VNode &n = nodes[hover_id];
        ImGui::BeginTooltip();
        ImGui::Text("node #%d  %s", hover_id, describe(n).c_str());
        ImGui::Text("move %s  depth %d  g=%g  h=%g  f=%g", n.action.c_str(), n.depth, n.g, n.h, n.g + n.h);
        if(n.expd <= cur) ImGui::Text("expanded at step %d", n.expd);
        else ImGui::Text("in frontier (discovered at step %d)", n.disc);
        ImGui::EndTooltip();
    }
    ImGui::End();
}

void viz_draw_board()
{
    if(!is_open || !show_board || nodes.empty()) return;
    // Bản đồ nhiệt: mỗi ô được tô theo số trạng thái đã mở rộng có khối nằm trên ô đó.
    map<pair<int, int>, int> heat;
    vector<pair<int, int> > cells;
    int hottest = 1;
    for(size_t i = 0; i < nodes.size(); i++)
    {
        if(nodes[i].expd > cur) continue;
        cells_of(nodes[i], cells);
        for(size_t k = 0; k < cells.size(); k++) hottest = max(hottest, ++heat[cells[k]]);
    }
    for(map<pair<int, int>, int>::iterator it = heat.begin(); it != heat.end(); ++it)
    {
        float t = float(it->second) / float(hottest);
        glm::vec3 col = glm::mix(glm::vec3(0.15f, 0.45f, 1.0f), glm::vec3(1.0f, 0.25f, 0.15f), t);
        BoxStyle s(col, PAT_PLAIN, 0.7f, 0.25f + 0.35f * t);
        s.unlit = true;
        renderer.draw_aabb(glm::vec3(it->first.first + 0.08f, it->first.second + 0.08f, 0.012f),
                           glm::vec3(0.84f, 0.84f, 0.02f), s);
    }
    bool finished = cur >= (int)steps.size() && found;
    if(finished)
        for(size_t k = 0; k < path.size(); k++)
            if(path[k] >= 0 && path[k] < (int)nodes.size())
                ghost_block(nodes[path[k]], glm::vec3(0.3f, 1.0f, 0.5f), 0.22f, 0.4f);
    int cn = current_node();
    if(cn >= 0)
        ghost_block(nodes[cn], glm::vec3(1.0f, 1.0f, 1.0f), 0.55f, 0.5f);
}
