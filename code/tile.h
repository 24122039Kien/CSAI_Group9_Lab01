#include "colors.h"
#include "render.h"

#ifndef TILES
#define TILES
class Game;
class Tile
{
    private:
        int tile_type;
    public:
        friend class Game;
        Tile();
        Tile(int tile_type);
        int get_tile_type();
};

#endif


#ifndef FLOOR
#define FLOOR
class Game;

// Ô vừa bị xóa (cầu đóng, ô cam vỡ): chỉ để vẽ hiệu ứng rơi, không ảnh hưởng luật.
struct GhostTile
{
    int i, j;
    BoxStyle style;
    double start;
    float spin;
};

class Floor
{
    public:
        Tile *** tiles;
        int n;
        int m;
        int tx;
        int ty;
        int sx;
        int sy;
        int along;
        int num_normal_bridge;
        int num_heavy_bridge;
        map<int, int> normal_bridge;
        map<int, int> heavy_bridge;
        map<int, vector<int>> normal_bridges;
        map<int, vector<int>> heavy_bridges;
        map<int, string> switch_modes;
    private:
        set<int> bridge_cells;          // các ô do công tắc điều khiển (vẽ màu cầu)
        map<int, double> appear_time;   // thời điểm ô cầu vừa hiện ra
        set<int> hidden;                // ô đã vỡ (chỉ ẩn khi vẽ)
        vector<GhostTile> ghosts;
        void clear();
        void make_ghost(int i, int j, float spin);
        void toggle(int a, int b);
    public:
        friend class Game;
        Floor();
        Floor(string filename);
        void initialize(string filename);
        void draw(double now);              // ô sàn + khung đích (không trong suốt)
        void draw_transparent(double now);  // ô đang rơi, cột sáng ở đích
        BoxStyle tile_style(int i, int j);
        void break_tile(int i, int j);      // hiệu ứng ô cam vỡ khi thua
        int get_along();
        glm::vec3 get_initial_pos();
        pair<int, int> get_initial_index();
        int get_tile_type(int i, int j);
        bool has_tile(int i, int j);    // false nếu ngoài lưới hoặc ô trống
        void heavy_bridge_toggle(int i, int j);
        void normal_bridge_toggle(int i, int j);
        ~Floor();
};

#endif

extern float tile_side_length;
extern float tile_height;
extern Floor stage;
extern int normal, fragile, normal_bridge, heavy_bridge;
