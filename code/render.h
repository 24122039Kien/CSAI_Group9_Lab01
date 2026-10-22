#ifndef RENDER
#define RENDER

// Bộ vẽ chung: một mesh hộp đơn vị [0,1]^3 dùng cho mọi ô, khối, hạt (không cấp phát GPU khi chơi).
// draw_box() chỉ ghi hộp vào lô; flush() gửi cả lô lên GPU và vẽ bằng MỘT lệnh instanced.
enum BoxPattern { PAT_PLAIN = 0, PAT_FRAGILE = 1, PAT_SWITCH = 2, PAT_HEAVY = 3, PAT_SHADOW = 4 };

struct BoxStyle
{
    glm::vec3 color;
    glm::vec3 accent;
    int pattern;
    float emissive;
    float alpha;
    bool unlit;
    BoxStyle(glm::vec3 c = glm::vec3(1.0f), int p = PAT_PLAIN, float e = 0.0f, float a = 1.0f)
        : color(c), accent(c), pattern(p), emissive(e), alpha(a), unlit(false) {}
};

// Dữ liệu một hộp gửi lên GPU (khớp với các attribute location 2..9 trong BOX_VERT).
struct BoxInstance
{
    glm::mat4 model;
    glm::vec4 color;    // rgb + emissive
    glm::vec4 accent;   // rgb + alpha
    glm::vec4 scale;    // kích thước + pattern
    glm::vec4 extra;    // x: unlit
};

class Renderer
{
    private:
        GLuint box_prog, bg_prog;
        GLuint box_vao, box_vbo, inst_vbo, bg_vao;
        GLint u_viewproj, u_time, u_eye, u_light, u_fogcolor, u_fogrange;
        GLint ubg_time, ubg_res;
        vector<BoxInstance> batch;
    public:
        int draw_calls;   // số lệnh vẽ trong frame (hiện trên HUD)
        int boxes;        // số hộp đã vẽ trong frame
        void init();
        void draw_background(int fbwidth, int fbheight, float time);
        // Bắt đầu vẽ cảnh 3D với camera cho trước.
        void begin(const glm::mat4 &view_proj, const glm::vec3 &eye, float time, float fog_near, float fog_far);
        // Thêm một hộp vào lô: model biến hộp đơn vị thành hộp thật; size = kích thước thật (để viền dày đều).
        void draw_box(const glm::mat4 &model, const glm::vec3 &size, const BoxStyle &style);
        // Tiện ích: hộp thẳng trục từ góc `pos` với kích thước `size`.
        void draw_aabb(const glm::vec3 &pos, const glm::vec3 &size, const BoxStyle &style);
        // Vẽ toàn bộ lô hiện tại (gọi cuối lượt đục và cuối lượt trong suốt).
        void flush();
};

GLuint compile_program(const char *vert_src, const char *frag_src);

extern Renderer renderer;

#endif
