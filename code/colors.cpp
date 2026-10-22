//#include "colors.h"

struct Color
{
    float r;
    float g;
    float b;
};
typedef struct Color Color;


Color * colors = new Color[50];

int BLUE = 0;
int RED = 1;
int GREEN = 2;
int SKYBLUE = 3;
int BLACK = 4;
int WHITE = 5;
int GREY = 6;
int ORANGE = 7;
int DARKGREEN = 8;
// Bảng màu giao diện mới (nền tối)
int FLOOR_COLOR = 9;
int BRIDGE_COLOR = 10;
int FRAGILE_COLOR = 11;
int SWITCH_ACCENT = 12;
int HEAVY_ACCENT = 13;
int BLOCK_COLOR = 14;
int GOAL_COLOR = 15;
int DUST_COLOR = 16;

static void set_color(int id, float r, float g, float b)
{
    colors[id].r = r;
    colors[id].g = g;
    colors[id].b = b;
}

void initialize_colors()
{
    set_color(BLUE, 0.0f, 0.0f, 1.0f);
    set_color(RED, 1.0f, 0.0f, 0.0f);
    set_color(GREEN, 0.0f, 1.0f, 0.0f);
    set_color(SKYBLUE, 0.6f, 0.94f, 0.94f);
    set_color(BLACK, 0.0f, 0.0f, 0.0f);
    set_color(WHITE, 1.0f, 1.0f, 1.0f);
    set_color(GREY, 0.8f, 0.8f, 0.8f);
    set_color(ORANGE, 255.0/255.0, 127.0/255.0, 80.0/255.0);
    set_color(DARKGREEN, 0/255.0, 100.0/255.0, 0.0/255.0);

    set_color(FLOOR_COLOR, 0.50f, 0.55f, 0.64f);     // xám thép ánh xanh
    set_color(BRIDGE_COLOR, 0.72f, 0.62f, 0.38f);    // ô cầu: xám ánh vàng
    set_color(FRAGILE_COLOR, 0.95f, 0.48f, 0.14f);   // ô cam dễ vỡ
    set_color(SWITCH_ACCENT, 0.25f, 1.00f, 0.45f);   // vòng công tắc thường
    set_color(HEAVY_ACCENT, 0.10f, 0.80f, 0.55f);    // dấu X công tắc nặng
    set_color(BLOCK_COLOR, 0.78f, 0.12f, 0.10f);     // khối người chơi
    set_color(GOAL_COLOR, 0.20f, 0.85f, 1.00f);      // khung đích phát sáng
    set_color(DUST_COLOR, 0.75f, 0.80f, 0.90f);
}
