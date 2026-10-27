#include "headers.h"
#include "tile.h"
#include "superblock.h"
#include "controls.h"
#include "effects.h"
#include "audio.h"

float tile_side_length = 1.0f;
float tile_height = 0.2f;
int normal=1, fragile=2, normal_bridge=3, heavy_bridge=4, split_switch=5;

static const float TILE_GAP = 0.015f;     // khe nhỏ giữa các ô để thấy rõ từng viên
static const float BRIDGE_ANIM = 0.35f;   // thời gian ô cầu trồi lên
static const float GHOST_LIFE = 1.2f;     // thời gian ô bị xóa rơi và mờ dần

Tile::Tile(){}

Tile::Tile(int tile_type)
{
    this->tile_type = tile_type;
}

int Tile::get_tile_type()
{
    return tile_type;
}


Floor::Floor()
{
    tiles = NULL;
    n = m = 0;
    collapse_start = -1.0;
}

Floor::Floor(string filename)
{
    tiles = NULL;
    n = m = 0;
    collapse_start = -1.0;
    initialize(filename);
}

void Floor::clear()
{
    if(tiles)
    {
        for(int i=0; i<n; i++)
        {
            for(int j=0; j<m; j++)
                delete tiles[i][j];
            delete[] tiles[i];
        }
        delete[] tiles;
    }
    tiles = NULL;
    n = m = 0;
    normal_bridge.clear();
    heavy_bridge.clear();
    normal_bridges.clear();
    heavy_bridges.clear();
    switch_modes.clear();
    split_switches.clear();
    bridge_cells.clear();
    appear_time.clear();
    hidden.clear();
    ghosts.clear();
    collapse_start = -1.0;
}

void Floor::initialize(string filename)
{
    clear();   // cho phép chơi lại / đổi màn mà không rò rỉ bộ nhớ

    ifstream fin(filename);
    if(!fin)
    {
        cerr << "Error : Can't open file " << filename << "." << endl;
        exit(EXIT_FAILURE);
    }

    fin >> n >> m;
    tiles = new Tile**[n];

    for(int i=0; i<n; i++)
        tiles[i] = new Tile*[m];

    int k;
    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
        {
            fin >> k;
            if(k)
                tiles[i][j] = new Tile(k);
            else
                tiles[i][j] = NULL;
        }
    fin >> sx >> sy;

    char q; fin >> q;
    if(q=='X')along = X;
    else if(q=='Y')along = Y;
    else if(q=='Z')along = Z;
    else exit(EXIT_FAILURE);

    fin >> tx >> ty;

    leftt = -1.0f;
    rightt = 1.0f + n*tile_side_length;
    topp = 1.0f + m*tile_side_length;
    bottomm = -1.0f;
    fin >> num_normal_bridge; normal_bridge.clear();
    for(int i=0; i<num_normal_bridge; i++)
    {
        int x, y, a, b;
        fin >> x >> y >> a >> b;
        normal_bridge[x*m+y] = a*m+b;
        normal_bridges[x*m+y] = {a*m+b};
        switch_modes[x*m+y] = "toggle";
        bridge_cells.insert(a*m+b);
    }

    fin >> num_heavy_bridge; heavy_bridge.clear();
    for(int i=0; i<num_heavy_bridge; i++)
    {
        int x, y, a, b;
        fin >> x >> y >> a >> b;
        heavy_bridge[x*m+y] = a*m+b;
        heavy_bridges[x*m+y] = {a*m+b};
        switch_modes[x*m+y] = "toggle";
        bridge_cells.insert(a*m+b);
    }

    string token;
    while(fin >> token)
    {
        if(token == "ext")
        {
            int ver;
            if(fin >> ver)
            {
                string cmd;
                while(fin >> cmd)
                {
                    if(cmd == "switch")
                    {
                        string kind, mode;
                        int x, y, count;
                        if(fin >> kind >> x >> y >> mode >> count)
                        {
                            vector<int> targets;
                            for(int c=0; c<count; c++)
                            {
                                int a, b;
                                fin >> a >> b;
                                targets.push_back(a*m+b);
                                bridge_cells.insert(a*m+b);
                            }
                            int key = x*m+y;
                            if(mode == "on") mode = "permanent_on";
                            if(mode == "off") mode = "permanent_off";
                            switch_modes[key] = mode;
                            if(kind == "normal")
                                normal_bridges[key] = targets;
                            else if(kind == "heavy")
                                heavy_bridges[key] = targets;
                        }
                    }
                    else if(cmd == "split")
                    {
                        int x, y, x1, y1, x2, y2;
                        if(fin >> x >> y >> x1 >> y1 >> x2 >> y2)
                        {
                            split_switches[x*m+y] = {{x1, y1}, {x2, y2}};
                        }
                    }
                }
            }
        }
    }

    fin.close();
}

BoxStyle Floor::tile_style(int i, int j)
{
    int k = tiles[i][j]->get_tile_type();
    BoxStyle s(col(FLOOR_COLOR));
    s.material = MAT_FLOOR;
    if(k==fragile)
    {
        s.color = col(FRAGILE_COLOR);
        s.pattern = PAT_FRAGILE;
        s.material = MAT_FRAGILE;
    }
    else if(k==::normal_bridge)   // :: vì Floor có thành viên cùng tên (map công tắc)
    {
        s.pattern = PAT_SWITCH;
        s.accent = col(SWITCH_ACCENT);
        s.material = MAT_SWITCH;
    }
    else if(k==::heavy_bridge)
    {
        s.pattern = PAT_HEAVY;
        s.accent = col(HEAVY_ACCENT);
        s.material = MAT_HEAVY;
    }
    else if(k==split_switch || split_switches.count(i*m+j))
    {
        s.pattern = PAT_SWITCH;
        s.color = glm::vec3(0.20f, 0.70f, 0.95f);
        s.accent = glm::vec3(1.0f, 1.0f, 1.0f);
        s.material = MAT_SPLIT;
    }
    else if(bridge_cells.count(i*m+j))
    {
        s.color = col(BRIDGE_COLOR);
        s.material = MAT_BRIDGE;
    }
    else if((i+j)&1)
        s.color *= 0.92f;   // bàn cờ nhẹ để dễ đếm ô
    return s;
}

void Floor::draw(double now)
{
    float l = tile_side_length;
    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
        {
            if(!tiles[i][j] || hidden.count(i*m+j))
                continue;
            float dz = effects.tile_drop(i, j, n, m, now);
            float grow = 1.0f;
            map<int, double>::iterator it = appear_time.find(i*m+j);
            if(it != appear_time.end())
            {
                float t = float(now - it->second) / BRIDGE_ANIM;
                if(t >= 1.0f)
                    appear_time.erase(it);
                else
                {
                    grow = 0.3f + 0.7f*ease_out_bounce(t);
                    dz -= (1.0f - ease_out_cubic(t)) * 0.8f;
                }
            }
            float side = (l - 2*TILE_GAP) * grow;
            float spin = 0.0f;
            dz += collapse_offset(i, j, now, spin);
            glm::vec3 pos(i*l + (l-side)/2, j*l + (l-side)/2, -tile_height + dz);
            glm::vec3 size(side, side, tile_height);
            if(spin == 0.0f)
                renderer.draw_aabb(pos, size, tile_style(i, j));
            else
            {
                // Ô đang sụp: lật nhẹ quanh tâm khi rơi.
                glm::vec3 c = pos + size * 0.5f;
                glm::vec3 axis = glm::normalize(glm::vec3(((i*7+j*3)&1) ? 1.0f : -0.6f, ((i+j*5)&1) ? 0.8f : -1.0f, 0.0f));
                renderer.draw_box(glm::translate(c) * glm::rotate(spin, axis) * glm::translate(-size * 0.5f) * glm::scale(size),
                                  size, tile_style(i, j));
            }
        }

    // Ô đích: một lỗ vuông có thành tối và khung phát sáng nhấp nháy.
    float pulse = 0.6f + 0.4f*sin(float(now)*3.0f);
    float goal_spin = 0.0f;
    float gx = tx*l, gy = ty*l, dz = effects.tile_drop(tx, ty, n, m, now) + collapse_offset(tx, ty, now, goal_spin);
    float w = 0.07f, h = 0.03f;
    BoxStyle frame(col(GOAL_COLOR), PAT_PLAIN, 0.6f + 0.6f*pulse);
    frame.material = MAT_GOAL;
    renderer.draw_aabb(glm::vec3(gx - w, gy - w, dz), glm::vec3(l + 2*w, w, h), frame);
    renderer.draw_aabb(glm::vec3(gx - w, gy + l, dz), glm::vec3(l + 2*w, w, h), frame);
    renderer.draw_aabb(glm::vec3(gx - w, gy, dz), glm::vec3(w, l, h), frame);
    renderer.draw_aabb(glm::vec3(gx + l, gy, dz), glm::vec3(w, l, h), frame);
    BoxStyle pit(glm::vec3(0.03f, 0.05f, 0.08f));
    float depth = 1.2f, t = 0.02f;
    renderer.draw_aabb(glm::vec3(gx - t, gy - t, -depth + dz), glm::vec3(l + 2*t, t, depth - 0.01f), pit);
    renderer.draw_aabb(glm::vec3(gx - t, gy + l, -depth + dz), glm::vec3(l + 2*t, t, depth - 0.01f), pit);
    renderer.draw_aabb(glm::vec3(gx - t, gy, -depth + dz), glm::vec3(t, l, depth - 0.01f), pit);
    renderer.draw_aabb(glm::vec3(gx + l, gy, -depth + dz), glm::vec3(t, l, depth - 0.01f), pit);
    BoxStyle bottom(col(GOAL_COLOR) * 0.25f, PAT_PLAIN, 0.3f*pulse);
    renderer.draw_aabb(glm::vec3(gx, gy, -depth + dz), glm::vec3(l, l, 0.05f), bottom);
}

void Floor::draw_transparent(double now)
{
    float l = tile_side_length;
    for(size_t k=0; k<ghosts.size(); k++)
    {
        const GhostTile &g = ghosts[k];
        float t = float(now - g.start);
        float a = 1.0f - t / GHOST_LIFE;
        if(a <= 0.0f)
            continue;
        glm::vec3 c(g.i*l + l/2, g.j*l + l/2, -tile_height/2 - 4.0f*t*t);
        glm::mat4 model = glm::translate(c)
                        * glm::rotate(g.spin * t, glm::normalize(glm::vec3(1.0f, 0.6f, 0.0f)))
                        * glm::translate(glm::vec3(-l/2, -l/2, -tile_height/2))
                        * glm::scale(glm::vec3(l - 2*TILE_GAP, l - 2*TILE_GAP, tile_height));
        BoxStyle s = g.style;
        s.alpha = a;
        renderer.draw_box(model, glm::vec3(l, l, tile_height), s);
    }

    // Cột sáng mờ phía trên lỗ đích để nhìn thấy mục tiêu từ xa.
    float pulse = 0.5f + 0.5f*sin(float(now)*2.0f);
    BoxStyle beam(col(GOAL_COLOR), PAT_PLAIN, 0.8f, 0.05f + 0.04f*pulse);
    beam.unlit = true;
    renderer.draw_aabb(glm::vec3(tx*l + 0.1f, ty*l + 0.1f, 0.0f), glm::vec3(l - 0.2f, l - 0.2f, 3.0f), beam);

    // Bỏ các ô đã rơi xong.
    for(size_t k=0; k<ghosts.size(); )
        if(now - ghosts[k].start > GHOST_LIFE)
        {
            ghosts[k] = ghosts.back();
            ghosts.pop_back();
        }
        else
            k++;
}

void Floor::make_ghost(int i, int j, float spin)
{
    GhostTile g;
    g.i = i;
    g.j = j;
    g.style = tile_style(i, j);
    g.start = glfwGetTime();
    g.spin = spin;
    ghosts.push_back(g);
}

void Floor::break_tile(int i, int j)
{
    if(!has_tile(i, j) || hidden.count(i*m+j))
        return;
    make_ghost(i, j, 2.5f);
    hidden.insert(i*m+j);
    audio_play(SFX_CRACK);
}

// Sàn sụp khi khối rơi (giống Bloxorz gốc): mỗi ô rơi sau một độ trễ ngẫu nhiên nhỏ, vừa rơi vừa lật.
void Floor::collapse(double now)
{
    if(collapse_start < 0.0)
        collapse_start = now;
}

bool Floor::collapsing()
{
    return collapse_start >= 0.0;
}

float Floor::collapse_offset(int i, int j, double now, float &spin)
{
    spin = 0.0f;
    if(collapse_start < 0.0)
        return 0.0f;
    unsigned h = unsigned(i) * 73856093u ^ unsigned(j) * 19349663u;
    float delay = 0.05f + 0.45f * float(h % 1000u) / 1000.0f;
    float t = float(now - collapse_start) - delay;
    if(t <= 0.0f)
        return 0.0f;
    spin = min(t * 2.2f, 2.5f);
    return -9.0f * t * t;
}

int Floor::get_along()
{
    return along;
}

pair<int, int> Floor::get_initial_index()
{
    return make_pair(sx, sy);
}

glm::vec3 Floor::get_initial_pos()
{
    float l = tile_side_length;

    glm::vec3 r(
        sx*l,
        sy*l,
        0.0f
    );
    int a = get_along();

    if(a==X)
        r = r + glm::vec3(l, l/2, l/2);
    else if(a==Y)
        r = r + glm::vec3(l/2, l, l/2);
    else if(a==Z)
        r = r + glm::vec3(l/2, l/2, l);

    return r;
}

int Floor::get_tile_type(int i, int j)
{
    return tiles[i][j]->get_tile_type();
}

bool Floor::has_tile(int i, int j)
{
    return i >= 0 && i < n && j >= 0 && j < m && tiles[i][j] != NULL;
}

// Bật/tắt một ô cầu: luật giữ nguyên bản gốc (ô trống -> tạo sàn type 1, có sàn -> xóa);
// phần thêm vào chỉ ghi lại thời điểm để vẽ hiệu ứng.
void Floor::toggle(int a, int b)
{
    if(!tiles[a][b])
    {
        tiles[a][b] = new Tile(1);
        appear_time[a*m+b] = glfwGetTime();
        audio_play(SFX_BRIDGE_ON);
    }
    else
    {
        audio_play(SFX_BRIDGE_OFF);
        make_ghost(a, b, 1.5f);
        delete tiles[a][b];
        tiles[a][b] = NULL;
        appear_time.erase(a*m+b);
    }
}

void Floor::heavy_bridge_toggle(int i, int j)
{
    int key = i*m+j;
    string mode = switch_modes.count(key) ? switch_modes[key] : "toggle";
    vector<int> targets;
    if(heavy_bridges.count(key) && !heavy_bridges[key].empty())
        targets = heavy_bridges[key];
    else if(heavy_bridge.count(key))
        targets.push_back(heavy_bridge[key]);

    for(size_t idx=0; idx<targets.size(); idx++)
    {
        int q = targets[idx];
        int a = q/m, b = q%m;
        if(mode == "permanent_on")
        {
            if(!tiles[a][b]) toggle(a, b);
        }
        else if(mode == "permanent_off")
        {
            if(tiles[a][b]) toggle(a, b);
        }
        else // toggle
        {
            toggle(a, b);
        }
    }
}

void Floor::normal_bridge_toggle(int i, int j)
{
    int key = i*m+j;
    string mode = switch_modes.count(key) ? switch_modes[key] : "toggle";
    vector<int> targets;
    if(normal_bridges.count(key) && !normal_bridges[key].empty())
        targets = normal_bridges[key];
    else if(normal_bridge.count(key))
        targets.push_back(normal_bridge[key]);

    for(size_t idx=0; idx<targets.size(); idx++)
    {
        int q = targets[idx];
        int a = q/m, b = q%m;
        if(mode == "permanent_on")
        {
            if(!tiles[a][b]) toggle(a, b);
        }
        else if(mode == "permanent_off")
        {
            if(tiles[a][b]) toggle(a, b);
        }
        else // toggle
        {
            toggle(a, b);
        }
    }
}

Floor::~Floor()
{
    clear();
}
