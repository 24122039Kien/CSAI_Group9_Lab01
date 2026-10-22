#ifndef COLORSTRUCT
#define COLORSTRUCT

struct Color
{
    float r;
    float g;
    float b;
};
typedef struct Color Color;

#endif

void initialize_colors();

extern Color * colors;
extern int BLUE;
extern int RED;
extern int GREEN;
extern int SKYBLUE;
extern int BLACK;
extern int WHITE;
extern int GREY;
extern int ORANGE;
extern int DARKGREEN;
extern int FLOOR_COLOR, BRIDGE_COLOR, FRAGILE_COLOR, SWITCH_ACCENT, HEAVY_ACCENT, BLOCK_COLOR, GOAL_COLOR, DUST_COLOR;

#ifndef COLORVEC
#define COLORVEC
inline glm::vec3 col(int id) { return glm::vec3(colors[id].r, colors[id].g, colors[id].b); }
#endif
