#ifndef RENDER
#define RENDER

// Bộ vẽ chung: một mesh hộp đơn vị [0,1]^3 dùng cho mọi ô, khối, hạt (không cấp phát GPU khi chơi).
// draw_box() chỉ ghi hộp vào lô; flush() gửi cả lô lên GPU và vẽ bằng MỘT lệnh instanced.
enum BoxPattern { PAT_PLAIN = 0, PAT_FRAGILE = 1, PAT_SWITCH = 2, PAT_HEAVY = 3, PAT_SHADOW = 4 };

// Vật liệu cho theme Minecraft (texture pixel) và Original (đá, kim loại gỉ, công tắc nổi) sinh trong shader.
enum BoxMaterial
{
    MAT_NONE = 0, MAT_FLOOR, MAT_FRAGILE, MAT_SWITCH, MAT_HEAVY, MAT_SPLIT, MAT_BRIDGE,
    MAT_BLOCK, MAT_GOLD, MAT_GOAL
};
enum Theme { THEME_CLASSIC = 0, THEME_MINECRAFT = 1, THEME_ORIGINAL = 2, THEME_COUNT = 3 };

struct BoxStyle
{
    glm::vec3 color;
    glm::vec3 accent;
    int pattern;
    float emissive;
    float alpha;
    bool unlit;
    int material;
    BoxStyle(glm::vec3 c = glm::vec3(1.0f), int p = PAT_PLAIN, float e = 0.0f, float a = 1.0f)
        : color(c), accent(c), pattern(p), emissive(e), alpha(a), unlit(false), material(MAT_NONE) {}
};

// Dữ liệu một hộp gửi lên GPU (khớp với các attribute location 2..9 trong BOX_VERT).
struct BoxInstance
{
    glm::mat4 model;
    glm::vec4 color;    // rgb + emissive
    glm::vec4 accent;   // rgb + alpha
    glm::vec4 scale;    // kích thước + pattern
    glm::vec4 extra;    // x: unlit, y: vật liệu (BoxMaterial)
};

class Renderer
{
    private:
        GLuint box_prog, bg_prog;
        GLuint box_vao, box_vbo, inst_vbo, bg_vao;
        GLint u_viewproj, u_time, u_eye, u_light, u_fogcolor, u_fogrange, u_theme;
        GLint u_lightvp, u_shadowon;
        GLint ubg_time, ubg_res, ubg_theme;
        vector<BoxInstance> batch;
        // Bóng đổ: shadow map độ sâu nhìn từ nguồn sáng (chiếu song song phủ cả màn).
        GLuint shadow_prog, shadow_fbo, shadow_tex;
        GLint us_lightvp;
        int shadow_size;
        bool shadow_now;          // lượt vẽ hiện tại dùng bóng đổ (chỉ cảnh chính)
        glm::mat4 light_vp;
        // Hậu kỳ: vẽ cảnh vào framebuffer HDR có MSAA, rồi bloom + ghép ra màn hình.
        GLuint post_bright, post_blur, post_comp;
        GLint up_blur_dir, up_comp_strength, up_comp_theme;
        GLuint scene_fbo, scene_color_rb, scene_depth_rb, resolve_fbo, resolve_tex;
        GLuint bloom_fbo[4], bloom_tex[4];
        int post_w, post_h;
        bool post_failed, post_active;
        void upload_batch();
        bool ensure_post(int w, int h);
        void release_post();
        void fullscreen_pass(GLuint prog, GLuint target_fbo, int w, int h);
    public:
        int draw_calls;   // số lệnh vẽ trong frame (hiện trên HUD)
        int boxes;        // số hộp đã vẽ trong frame
        int theme;        // THEME_CLASSIC / THEME_MINECRAFT / THEME_ORIGINAL (đổi trong menu Options)
        bool shadows;     // bóng đổ thật (Options > Graphics)
        bool bloom;       // hậu kỳ bloom + vignette
        void init();
        void draw_background(int fbwidth, int fbheight, float time);
        // Đầu/cuối frame: chuyển sang framebuffer hậu kỳ (nếu bật bloom) rồi ghép kết quả ra màn hình.
        void begin_frame(int fbwidth, int fbheight);
        void end_frame(int fbwidth, int fbheight);
        // Bắt đầu vẽ cảnh 3D với camera cho trước. main_view: cảnh chính (có bóng đổ), false: bản đồ nhỏ.
        void begin(const glm::mat4 &view_proj, const glm::vec3 &eye, float time, float fog_near, float fog_far,
                   bool main_view = false);
        bool shadows_active() { return shadow_now; }
        glm::vec3 light_dir();
        // Vẽ lô đục: trước tiên ghi shadow map từ chính lô này, sau đó vẽ cảnh có bóng.
        void flush_opaque();
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
